#include <application.h>
#include <marble.h>


#pragma region Marble_Asset
static void inline Marble_Asset_Internal_ReadCommonAssetHead(Marble_Util_FileStream *sFileStream, Marble_AssetHead *sHead) {
	Marble_Util_FileStream_ReadSize(sFileStream, sizeof(*sHead), sHead);
}

static _Bool inline Marble_Asset_Internal_CheckForDuplicate(CHAR const *astrAssetId, _Bool (*fnFind)(CHAR const *, void *), void **ptrpDupePtr) {
	void *ptrDupe = Marble_Util_HashTable_Find(gl_sApplication.sAssets.sHashTable, astrAssetId, fnFind);

	if (ptrpDupePtr)
		*ptrpDupePtr = ptrDupe;

	return (_Bool)ptrDupe;
}

static void inline Marble_Asset_Internal_CopyAssetId(Marble_AssetHead *sAssetHead, CHAR *astrId) {
	memcpy_s(astrId, MARBLE_ASSETIDLEN, sAssetHead->astrAssetId, MARBLE_ASSETIDLEN);
}

static void Marble_Asset_Internal_ReleaseChildAssets(Marble_Asset *sParent) {
	if (!sParent || !sParent->sDependencies)
		return;

	for (size_t stIndex = 0; stIndex < sParent->sDependencies->stSize; stIndex++)
		Marble_Asset_Release(Marble_Util_Vector_Get(sParent->sDependencies, stIndex));
}

static int Marble_Asset_Internal_ProcessAssetDependency_Asset(Marble_Asset *sParent, Marble_Util_FileStream *sFileStream) { MARBLE_ERRNO
	DWORD  dwAssetPathLen        = 0;
	CHAR   astrAssetPath[1024]   = { 0 };
	WCHAR  wstrAssetPath[1024]   = { 0 };
	_Bool  blIsDuplicate         = FALSE;
	size_t stNumOfCharsConverted = 0;

	Marble_Util_FileStream_ReadDWORD(sFileStream, &dwAssetPathLen);
	Marble_Util_FileStream_ReadSize(sFileStream, dwAssetPathLen, &astrAssetPath[0]);

	Marble_Asset *sAsset = NULL;
	mbstowcs_s(&stNumOfCharsConverted, wstrAssetPath, 1024, astrAssetPath, 1024);
	if ((iErrorCode = Marble_Asset_CreateAndLoadFromFile(wstrAssetPath, &sAsset, &sAsset)) && iErrorCode ^ Marble_ErrorCode_DuplicatesNotAllowed)
		return iErrorCode;
	blIsDuplicate = iErrorCode == Marble_ErrorCode_DuplicatesNotAllowed;

	Marble_Asset_Obtain(sAsset);
	if (iErrorCode = Marble_Util_Vector_PushBack(sParent->sDependencies, sAsset))
		return iErrorCode;

	if (!blIsDuplicate && (iErrorCode = Marble_Asset_Register(sAsset))) {
		Marble_Asset_Release(sAsset);

		return iErrorCode;
	}

	return Marble_ErrorCode_Ok;
}

static int Marble_Asset_Internal_EvaluateDependencyTable(Marble_Asset *sAsset, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead) { MARBLE_ERRNO
	if (!sAssetHead->dwNumOfDeps)
		return Marble_ErrorCode_Ok;

	/* 
		* Initialize dependency table; is used as some kind of cache,
		* and will be traversed when the asset gets unloaded, decrementing
		* the ref-count of all the assets this asset loaded throughout its
		* life-time.
		* The dependency table will only be created when the dependency table
		* inside the file actually has entries.
	*/
	if (iErrorCode = Marble_Util_Vector_Create(Marble_Util_VectorType_VecOfPointers, 0, 16, NULL, NULL, &sAsset->sDependencies))
		return iErrorCode;

	for (DWORD dwIndex = 0; dwIndex < sAssetHead->dwNumOfDeps; dwIndex++) {
		BYTE bCommand = 0;
		Marble_Util_FileStream_ReadBYTE(sStream, &bCommand);

		switch (bCommand) {
			case Marble_AssetDependency_Asset: 
				if (iErrorCode = Marble_Asset_Internal_ProcessAssetDependency_Asset(sAsset, sStream))
					goto ON_ERROR;
		}
	}

	return Marble_ErrorCode_Ok;

ON_ERROR:
	Marble_Asset_Internal_ReleaseChildAssets(sAsset);

	return iErrorCode;
}

static _Bool Marble_Asset_Internal_FindCallbackFn(CHAR const *astrKey, Marble_Asset *sAsset) {
	return astrKey && sAsset && !strcmp(astrKey, sAsset->astrAssetId);
}


int Marble_Asset_Create(Marble_Asset **ptrpAsset) {
	return Marble_ErrorCode_UnimplementedFeature;
}

int Marble_Asset_CreateExplicit(int iAssetType, void const *ptrCreateParams, Marble_Asset **ptrpAsset) { MARBLE_ERRNO
	extern int Marble_Asset_CreateImageAsset(Marble_Asset **ptrpImageAsset);
	extern int Marble_Asset_CreateColorTableAsset(Marble_Asset **ptrpColorTable);
	extern int Marble_Asset_CreateMapAsset(void const *ptrCreateParams, Marble_Asset **ptrpMapAsset);

	if (!ptrpAsset)
		return Marble_ErrorCode_Parameter;

	switch (iAssetType) {
		case Marble_AssetType_Image:      iErrorCode = Marble_Asset_CreateImageAsset(ptrpAsset);                break;
		case Marble_AssetType_ColorTable: iErrorCode = Marble_Asset_CreateColorTableAsset(ptrpAsset);           break;
		case Marble_AssetType_Map:        iErrorCode = Marble_Asset_CreateMapAsset(ptrCreateParams, ptrpAsset); break;
		default:
			*ptrpAsset = NULL;

			return Marble_ErrorCode_AssetType;
	}

	if (ptrCreateParams)
		Marble_Asset_SetId(*ptrpAsset, ((Marble_Asset_CreateParams *)ptrCreateParams)->astrId);

	/* Free generic asset structure */
	if (iErrorCode)
		Marble_Asset_Destroy(ptrpAsset);

	return iErrorCode;
}

void Marble_Asset_Destroy(Marble_Asset **ptrpAsset) {
	extern void Marble_ImageAsset_Destroy(Marble_Asset *sImage);
	extern void Marble_ColorTableAsset_Destroy(Marble_Asset *sColorTable);
	extern void Marble_MapAsset_Destroy(Marble_Asset *sMap);

	if (!ptrpAsset || !*ptrpAsset)
		return;

	Marble_Asset_Internal_ReleaseChildAssets(*ptrpAsset);
	Marble_Util_Vector_Destroy(&(*ptrpAsset)->sDependencies);

	//Marble_Util_HashTable_Erase((*ptrpAsset)->sRefAssetMan->sHashTable, (*ptrpAsset)->astrAssetId, FALSE);

	switch ((*ptrpAsset)->iAssetType) {
		case Marble_AssetType_Image:      Marble_ImageAsset_Destroy(*ptrpAsset);      break;
		case Marble_AssetType_ColorTable: Marble_ColorTableAsset_Destroy(*ptrpAsset); break;
		case Marble_AssetType_Map:        Marble_MapAsset_Destroy(*ptrpAsset);        break;
	}

	printf("Destroyed asset \"%s\" (type = %i) at address 0x%p.\n",
		(*ptrpAsset)->astrAssetId,
		(*ptrpAsset)->iAssetType,
		*ptrpAsset
	);

	free(*ptrpAsset);
	*ptrpAsset = NULL;
}

int Marble_Asset_LoadFromFile(Marble_Asset *sAsset, TCHAR const *strPath, void **ptrpExistingAssetPtr) { MARBLE_ERRNO
	extern int Marble_ImageAsset_LoadFromFile(Marble_Asset *sImage, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead);
	extern int Marble_ColorTableAsset_LoadFromFile(Marble_Asset *sColorTable, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead);
	extern int Marble_MapAsset_LoadFromFile(Marble_Asset *sMap, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead);

	if (!sAsset || !strPath || !*strPath)
		return Marble_ErrorCode_Parameter;

	Marble_Util_FileStream *sStream = NULL;
	if (iErrorCode = Marble_Util_FileStream_Open(strPath, Marble_Util_StreamPerm_Read, &sStream))
		return iErrorCode;
	
	Marble_AssetHead sAssetHead = { 0 };
	Marble_Asset_Internal_ReadCommonAssetHead(sStream, &sAssetHead);
	if (Marble_Asset_Internal_CheckForDuplicate(sAsset->astrAssetId, (_Bool (*)(CHAR const *, void *))&Marble_Asset_Internal_FindCallbackFn, ptrpExistingAssetPtr))
		return Marble_ErrorCode_DuplicatesNotAllowed;
	Marble_Asset_Internal_CopyAssetId(&sAssetHead, sAsset->astrAssetId);

	/* Read dependency table and load all deps */
	if (iErrorCode = Marble_Asset_Internal_EvaluateDependencyTable(sAsset, sStream, &sAssetHead))
		return iErrorCode;

	/* Load asset from file */
	switch (sAsset->iAssetType) {
		case Marble_AssetType_Image:      iErrorCode = Marble_ImageAsset_LoadFromFile(sAsset, strPath, sStream, &sAssetHead);      break;
		case Marble_AssetType_ColorTable: iErrorCode = Marble_ColorTableAsset_LoadFromFile(sAsset, strPath, sStream, &sAssetHead); break;
		case Marble_AssetType_Map:        iErrorCode = Marble_MapAsset_LoadFromFile(sAsset, strPath, sStream, &sAssetHead);        break;
		default:                          iErrorCode = Marble_ErrorCode_AssetType;
	}

	Marble_Util_FileStream_Destroy(&sStream);
	return iErrorCode;
}

int Marble_Asset_CreateAndLoadFromFile(TCHAR const *strPath, Marble_Asset **ptrpAsset, void **ptrpExistingAssetPtr) { MARBLE_ERRNO
	if (!ptrpAsset || !strPath || !*strPath)
		return Marble_ErrorCode_Parameter;

	Marble_Util_FileStream *sStream = NULL;
	if (iErrorCode = Marble_Util_FileStream_Open(strPath, Marble_Util_StreamPerm_Read, &sStream))
		return iErrorCode;

	Marble_AssetHead sTmpHead = { 0 };
	Marble_Asset_Internal_ReadCommonAssetHead(sStream, &sTmpHead);
	Marble_Util_FileStream_Destroy(&sStream);

	if (iErrorCode = Marble_Asset_CreateAndLoadFromFileExplicit(sTmpHead.dwMagic >> 16, strPath, NULL, ptrpAsset, ptrpExistingAssetPtr))
		return iErrorCode;

	return Marble_ErrorCode_Ok;
}

int Marble_Asset_CreateAndLoadFromFileExplicit(int iAssetType, TCHAR const *strPath, void const *ptrCreateParams, Marble_Asset **ptrpAsset, void **ptrpExistingAssetPtr) { MARBLE_ERRNO
	if (!ptrpAsset || !strPath || !*strPath)
		return Marble_ErrorCode_Parameter;

	if (iErrorCode = Marble_Asset_CreateExplicit(iAssetType, ptrCreateParams, ptrpAsset))
		return iErrorCode;

	if (iErrorCode = Marble_Asset_LoadFromFile(*ptrpAsset, strPath, NULL)) {
		Marble_Asset_Destroy(ptrpAsset);

		return iErrorCode;
	}

	return Marble_ErrorCode_Ok;
}

int Marble_Asset_GetType(Marble_Asset *sAsset) {
	return sAsset ? sAsset->iAssetType : Marble_AssetType_Unknown;
}

CHAR *Marble_Asset_GetId(Marble_Asset *sAsset) {
	return sAsset ? sAsset->astrAssetId : NULL;
}

void Marble_Asset_SetId(Marble_Asset *sAsset, CHAR const *astrNewId) {
	if (!sAsset || !astrNewId)
		return;

	strcpy_s(sAsset->astrAssetId, MARBLE_ASSETIDLEN, astrNewId);
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
	if (!gl_sApplication.sAssets.blIsInit || !sAsset)
		return sAsset ? Marble_ErrorCode_Parameter : Marble_ErrorCode_ComponentInitState;

	if (Marble_Asset_Internal_CheckForDuplicate(sAsset->astrAssetId, (_Bool (*)(CHAR const *, void *))&Marble_Asset_Internal_FindCallbackFn, NULL))
		return Marble_ErrorCode_DuplicatesNotAllowed;

	if (iErrorCode = Marble_Util_HashTable_Insert(gl_sApplication.sAssets.sHashTable, sAsset->astrAssetId, sAsset, FALSE))
		return iErrorCode;

	return Marble_ErrorCode_Ok;
}

int Marble_Asset_Unregister(Marble_Asset *sAsset, _Bool blDoFree) {
	if (!gl_sApplication.sAssets.blIsInit || !sAsset)
		return sAsset ? Marble_ErrorCode_Parameter : Marble_ErrorCode_ComponentInitState;

	Marble_Util_HashTable_Erase(gl_sApplication.sAssets.sHashTable, sAsset->astrAssetId, blDoFree);

	return Marble_ErrorCode_Ok;
}

int Marble_Asset_Obtain(Marble_Asset *sAsset) {
	if (!sAsset)
		return -1;

	if (sAsset->iRefCount ^ -1)
		return ++sAsset->iRefCount;

	return -2;
}

int Marble_Asset_Release(Marble_Asset *sAsset) {
	if (!sAsset)
		return -1;

	if (sAsset->iRefCount ^ -1) {
		if (!--sAsset->iRefCount)
			return Marble_Asset_Unregister(sAsset, TRUE);

		return sAsset->iRefCount;
	}

	return -2;
}
#pragma endregion


#pragma region Marble_AssetManager
int Marble_AssetManager_Create(void) { MARBLE_ERRNO
	extern void Marble_AssetManager_Destroy(void);

	Marble_IfError(
		CoCreateInstance(
			&CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			&IID_IWICImagingFactory2,
			&gl_sApplication.sAssets.sWICFactory
		), S_OK, {
			Marble_AssetManager_Destroy();

			return Marble_ErrorCode_CreateWICImagingFactory;
		}
	);

	Marble_IfError(
		Marble_Util_HashTable_Create(
			&gl_sApplication.sAssets.sHashTable,
			64, 
			(void (*)(void **))&Marble_Asset_Destroy
		), Marble_ErrorCode_Ok, {
			Marble_AssetManager_Destroy();

			return iErrorCode;
		}
	);

	gl_sApplication.sAssets.blIsInit = TRUE;
	return Marble_ErrorCode_Ok;
}

void Marble_AssetManager_Destroy(void) {
	if (!gl_sApplication.sAssets.blIsInit)
		return;

	Marble_Util_HashTable_Destroy(&gl_sApplication.sAssets.sHashTable);

	if (gl_sApplication.sAssets.sWICFactory)
		gl_sApplication.sAssets.sWICFactory->lpVtbl->Release(gl_sApplication.sAssets.sWICFactory);
}
#pragma endregion


