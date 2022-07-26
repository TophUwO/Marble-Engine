#include <application.h>
#include <marble.h>


#pragma region Marble_Asset
static _Bool inline Marble_Asset_Internal_IsValidAssetType(int iAssetType) {
	return iAssetType > Marble_AssetType_Unknown && iAssetType < __MARBLE_NUMASSETTYPES__;
}

static _Bool inline Marble_Asset_Internal_FindFn(CHAR const *strKey, void *ptrCmpAsset) {
	return !strcmp(strKey, ((Marble_Asset *)ptrCmpAsset)->astrAssetID);
}

static _Bool inline Marble_Asset_Internal_AssetAlreadyLoaded(CHAR const *strId, Marble_Asset **ptrpExistingAssetPtr) {
	Marble_Asset *ptrFound = Marble_Util_HashTable_Find(gl_sApplication.sAssets.sHashTable, strId, &Marble_Asset_Internal_FindFn);

	if (ptrFound && ptrpExistingAssetPtr)
		*ptrpExistingAssetPtr = ptrFound;

	return (_Bool)ptrFound;
}

static void inline Marble_Asset_Internal_SetID(Marble_Asset *sAsset, CHAR const *strNewID) {
	strcpy_s(sAsset->astrAssetID, MARBLE_ASSETIDLEN, strNewID);
}

static _Bool inline Marble_Asset_Internal_IsValidAssetID(CHAR const *strID, Marble_Asset **ptrpExistingAssetPtr) {
	if (!strID || !*strID)
		return FALSE;

	return !Marble_Asset_Internal_AssetAlreadyLoaded(strID, ptrpExistingAssetPtr);
}

static int Marble_Asset_Internal_EvaluateDependencyTable(Marble_Asset *sParentAsset, Marble_Util_FileStream *sStream, Marble_CommonAssetHead *sCommonAssetHead) { MARBLE_ERRNO
	if (!sCommonAssetHead->dwNumOfDeps)
		return Marble_ErrorCode_Ok;

	/* Create asset vector in case there are asset dependencies to load */
	MB_IFNOK_RET_CODE(Marble_Util_Vector_Create(
		Marble_Util_VectorType_VecOfPointers, 
		0, 
		8, 
		NULL, 
		NULL, 
		&sParentAsset->sDependencies
	));

	/* Traverse asset dependency table */
	for (DWORD dwIndex = 0; dwIndex < sCommonAssetHead->dwNumOfDeps; dwIndex++) {
		BYTE bCommand = 0;
		MB_IFNOK_RET_CODE(Marble_Util_FileStream_ReadBYTE(sStream, &bCommand));

		switch (bCommand) {
			case 1: {
				CHAR  strPath[1024] = { 0 };
				WCHAR wstrPath[1024] = { 0 };

				DWORD dwSizeInBytes = 0;
				Marble_Util_FileStream_ReadDWORD(sStream, &dwSizeInBytes);
				Marble_Util_FileStream_ReadSize(sStream, dwSizeInBytes, strPath);
				if (!*strPath) {
					iErrorCode = Marble_ErrorCode_AssetID;

					goto lbl_CLEANUP;
				}

				size_t stNumOfCharsConverted = 0;
				mbstowcs_s(
					&stNumOfCharsConverted,
					wstrPath,
					1024,
					strPath,
					1024
				);

				Marble_Asset *sAsset = NULL;
				MB_IFNOK_GOTO_LBL(Marble_Asset_LoadFromFile(
						wstrPath, 
						&sAsset, 
						&sAsset
				), lbl_CLEANUP);
				_Bool const blIsDupe = iErrorCode == Marble_ErrorCode_AssetID;

				Marble_Asset_Obtain(sAsset);
				Marble_Util_Vector_PushBack(sParentAsset->sDependencies, sAsset);
				if (!blIsDupe && (iErrorCode = Marble_Asset_Register(sAsset)))
					goto lbl_CLEANUP;
			}
		}

	lbl_CLEANUP:
		if (iErrorCode) {
			printf("AssetManager: Dependency command %i (index: %u) failed; error: %i (%S).\n",
				bCommand,
				dwIndex,
				iErrorCode,
				Marble_Error_ToString(iErrorCode)
			);

			return iErrorCode;
		}
	}

	return iErrorCode;
}


int Marble_Asset_Create(int iAssetType, CHAR const *strID, void const *ptrCreateParams, Marble_Asset **ptrpAsset, Marble_Asset **ptrpExistingAssetPtr) { MARBLE_ERRNO
	extern int Marble_ColorTableAsset_Create(void const *ptrCreateParams, Marble_Asset **ptrpColorTable);
	extern int Marble_MapAsset_Create(void const *ptrCreateParams, Marble_Asset **ptrpMapAsset);

	if (!ptrpAsset)
		return Marble_ErrorCode_Parameter;
	MB_IFNTRUE_RET_CODE(Marble_Asset_Internal_IsValidAssetType(iAssetType), Marble_ErrorCode_AssetType);
	MB_IFNTRUE_RET_CODE(Marble_Asset_Internal_IsValidAssetID(strID, ptrpExistingAssetPtr), Marble_ErrorCode_AssetID);

	switch (iAssetType) {
		case Marble_AssetType_ColorTable: iErrorCode = Marble_ColorTableAsset_Create(ptrCreateParams, ptrpAsset); break;
		case Marble_AssetType_Map:        iErrorCode = Marble_MapAsset_Create(ptrCreateParams, ptrpAsset);        break;
		default:
			iErrorCode = Marble_ErrorCode_UnimplementedFeature;
	}

	if (!iErrorCode)
		Marble_Asset_Internal_SetID(*ptrpAsset, strID);
	else
		Marble_Asset_Destroy(ptrpAsset, TRUE);

	return iErrorCode;
}

int Marble_Asset_LoadFromFile(TCHAR const *strPath, Marble_Asset **ptrpAsset, Marble_Asset **ptrpExistingAssetPtr) { MARBLE_ERRNO
	extern int Marble_ColorTableAsset_LoadFromFile(Marble_Asset *sColorTable, Marble_Util_FileStream *sStream, Marble_CommonAssetHead *sAssetHead);
	extern int Marble_MapAsset_LoadFromFile(Marble_Asset *sMap, Marble_Util_FileStream *sStream, Marble_CommonAssetHead *sAssetHead);

	if (!ptrpAsset)
		return Marble_ErrorCode_Parameter;
	_Bool blWasCreated = FALSE;

	Marble_Util_FileStream *sStream = NULL;
	MB_IFNOK_RET_CODE(Marble_Util_FileStream_Open(
		strPath, 
		Marble_Util_StreamPerm_Read, 
		&sStream
	));

	Marble_CommonAssetHead sCommonHead = { 0 };
	MB_IFNOK_GOTO_LBL(Marble_Util_FileStream_ReadSize(
		sStream, 
		sizeof(sCommonHead), 
		&sCommonHead
	), lbl_CLEANUP);

	/* Create asset if it has not been created yet */
	if (!*ptrpAsset) {
		blWasCreated = TRUE;

		MB_IFNOK_GOTO_LBL(Marble_Asset_Create(
			sCommonHead.dwMagic >> 16, 
			sCommonHead.astrAssetID, 
			NULL, 
			ptrpAsset, 
			ptrpExistingAssetPtr
		), lbl_CLEANUP);
	}


	MB_IFNOK_GOTO_LBL(Marble_Asset_Internal_EvaluateDependencyTable(
		*ptrpAsset, 
		sStream, 
		&sCommonHead
	), lbl_CLEANUP);

	Marble_Util_FileStream_Goto(sStream, sCommonHead.dwOffAssetHead);

	switch (sCommonHead.uAssetType) {
		case Marble_AssetType_ColorTable: iErrorCode = Marble_ColorTableAsset_LoadFromFile(*ptrpAsset, sStream, &sCommonHead); break;
		case Marble_AssetType_Map:        iErrorCode = Marble_MapAsset_LoadFromFile(*ptrpAsset, sStream, &sCommonHead);        break;
	}

lbl_CLEANUP:
	if (iErrorCode && blWasCreated) {
		Marble_Asset_Destroy(ptrpAsset, TRUE);

		printf("AssetManager: Failed to create asset \"%s\"; error: %i (%S).\n",
			sCommonHead.astrAssetID,
			iErrorCode,
			Marble_Error_ToString(iErrorCode)
		);
	}

	Marble_Util_FileStream_Destroy(&sStream);
	return iErrorCode;
}

void Marble_Asset_Destroy(Marble_Asset **ptrpAsset, _Bool blIsInternal) {
	extern void Marble_ColorTableAsset_Destroy(Marble_Asset *ptrColorTable);
	extern void Marble_MapAsset_Destroy(Marble_Asset *sMapAsset);

	if (!ptrpAsset || !*ptrpAsset)
		return;

	switch ((*ptrpAsset)->iAssetType) {
		case Marble_AssetType_ColorTable: Marble_ColorTableAsset_Destroy(*ptrpAsset); break;
		case Marble_AssetType_Map:        Marble_MapAsset_Destroy(*ptrpAsset);        break;
	}

	if ((*ptrpAsset)->sDependencies) {
		for (size_t stIndex = 0; stIndex < (*ptrpAsset)->sDependencies->stSize; stIndex++)
			Marble_Asset_Release((*ptrpAsset)->sDependencies->ptrpData[stIndex]);

		Marble_Util_Vector_Destroy(&(*ptrpAsset)->sDependencies);
	}

	printf("AssetManager: Destroyed asset \"%s\" (type = %i), at address 0x%p (outstanding references: %i).\n",
		(*ptrpAsset)->astrAssetID,
		(*ptrpAsset)->iAssetType,
		*ptrpAsset,
		(*ptrpAsset)->iRefCount
	);

	free(*ptrpAsset);
	*ptrpAsset = NULL;
}

int Marble_Asset_GetType(Marble_Asset *sAsset) {
	return sAsset ? sAsset->iAssetType : Marble_AssetType_Unknown;
}

CHAR *Marble_Asset_GetId(Marble_Asset *sAsset) {
	return sAsset ? sAsset->astrAssetID : NULL;
}


/// <summary>
/// int Marble_Asset_Register(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset)
/// 
/// Attempt to register asset (i.e. transfer ownership of asset pointer from user to engine);
/// If the asset does already exist or it cannot be registered, the ownership will not be
/// transferred, meaning that the user will still have to take care of destroying the asset.
/// </summary>
/// <param name="Marble_AssetManager *sAssetManager"> > Asset manager to register asset in </param>
/// <param name="Marble_Asset *sAsset"> > Asset to register </param>
/// <returns>Non-zero on error; 0 on success.</returns>
int Marble_Asset_Register(Marble_Asset *sAsset) { MARBLE_ERRNO
	if (!sAsset || !gl_sApplication.sAssets.blIsInit)
		return sAsset ? Marble_ErrorCode_Parameter : Marble_ErrorCode_ComponentInitState;

	MB_IFNOK_RET_CODE(Marble_Util_HashTable_Insert(
		gl_sApplication.sAssets.sHashTable, 
		sAsset->astrAssetID, 
		sAsset, 
		FALSE
	));

	printf("AssetManager: Successfully registered asset \"%s\" (type = %i).\n",
		sAsset->astrAssetID,
		sAsset->iAssetType
	);

	return Marble_ErrorCode_Ok;
}

int Marble_Asset_Unregister(Marble_Asset *sAsset, _Bool blDoFree) {
	if (!sAsset || !gl_sApplication.sAssets.blIsInit)
		return sAsset ? Marble_ErrorCode_Parameter : Marble_ErrorCode_ComponentInitState;

	Marble_Util_HashTable_Erase(gl_sApplication.sAssets.sHashTable, sAsset->astrAssetID, &Marble_Asset_Internal_FindFn, blDoFree);

	return Marble_ErrorCode_Ok;
}

int Marble_Asset_Obtain(Marble_Asset *sAsset) {
	if (!sAsset)
		return Marble_ErrorCode_Parameter;

	if (sAsset->iRefCount ^ -1)
		++sAsset->iRefCount;

	return Marble_ErrorCode_Ok;
}

int Marble_Asset_Release(Marble_Asset *sAsset) {
	if (!sAsset)
		return Marble_ErrorCode_Parameter;

	if (sAsset->iRefCount ^ -1) {
		printf("AssetManager: Releasing asset \"%s\" (type = %i); current ref-count: %i.\n",
			sAsset->astrAssetID,
			sAsset->iAssetType,
			sAsset->iRefCount
		);

		if (!--sAsset->iRefCount)
			return Marble_Asset_Unregister(sAsset, TRUE);
	}

	return Marble_ErrorCode_Ok;
}
#pragma endregion


#pragma region Marble_AssetManager
int Marble_AssetManager_Create(void) { MARBLE_ERRNO
	extern void Marble_AssetManager_Destroy(void);

	/* 
		* If asset manager is already initialized, block further
		* attempts to (re-)initialize. 
	*/
	if (gl_sApplication.sAssets.blIsInit)
		return Marble_ErrorCode_ComponentInitState;

	MB_IFNOK_GOTO_LBL(CoCreateInstance(
		&CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		&IID_IWICImagingFactory2,
		&gl_sApplication.sAssets.sWICFactory
	), lbl_ERROR);

	MB_IFNOK_GOTO_LBL(Marble_Util_HashTable_Create(
		&gl_sApplication.sAssets.sHashTable,
		64, 
		(void (*)(void **))&Marble_Asset_Destroy
	), lbl_ERROR);

	gl_sApplication.sAssets.blIsInit = TRUE;
	return Marble_ErrorCode_Ok;

lbl_ERROR:
	Marble_AssetManager_Destroy();

	return iErrorCode;
}

void Marble_AssetManager_Destroy(void) {
	if (!gl_sApplication.sAssets.blIsInit)
		return;

	gl_sApplication.sAssets.blIsInit = FALSE;

	Marble_Util_HashTable_Destroy(&gl_sApplication.sAssets.sHashTable);
	if (gl_sApplication.sAssets.sWICFactory)
		gl_sApplication.sAssets.sWICFactory->lpVtbl->Release(gl_sApplication.sAssets.sWICFactory);
}
#pragma endregion


