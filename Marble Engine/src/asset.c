#include <application.h>
#include <marble.h>


#pragma region Marble_Asset
static void Marble_Asset_Internal_CopyAssetId(Marble_AssetHead *sAssetHead, CHAR *astrId) {
	memcpy_s(astrId, MARBLE_ASSETIDLEN, sAssetHead->astrAssetId, MARBLE_ASSETIDLEN);
}

static int Marble_Asset_Internal_ReadAssetDependency_Asset(Marble_Util_FileStream *sFileStream) { MARBLE_ERRNO
	DWORD dwAssetPathLen      = 0;
	WCHAR wstrAssetPath[1024] = { 0 };
	_Bool blIsRequired        = FALSE;

	Marble_Util_FileStream_ReadDWORD(sFileStream, &dwAssetPathLen);
	Marble_Util_FileStream_ReadSize(sFileStream, dwAssetPathLen, &wstrAssetPath[0]);
	Marble_Util_FileStream_ReadBYTE(sFileStream, &blIsRequired);

	Marble_Asset *sAsset = NULL;
	if (iErrorCode = Marble_Asset_CreateAndLoadFromFile(wstrAssetPath, &sAsset))
		return iErrorCode;

	if (iErrorCode = Marble_Asset_Register(gl_sApplication.sAssets, sAsset)) {
		Marble_Asset_Destroy(&sAsset);

		return iErrorCode;
	}

	return Marble_ErrorCode_Ok;
}

static int Marble_Asset_Internal_EvaluateDependencyTable(Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead) { MARBLE_ERRNO
	Marble_Util_FileStream_Goto(sStream, sAssetHead->dwOffDepTable);

	for (DWORD dwIndex = 0; dwIndex < sAssetHead->dwNumOfDeps; dwIndex++) {
		BYTE bCommand = 0;
		Marble_Util_FileStream_ReadBYTE(sStream, &bCommand);

		switch (bCommand) {
			case Marble_AssetDependency_Asset: 
				if (iErrorCode = Marble_Asset_Internal_ReadAssetDependency_Asset(sStream))
					goto ON_ERROR;
		}
	}

	return Marble_Util_FileStream_Goto(sStream, sAssetHead->dwOffData);

ON_ERROR:
	/* TODO: unload all assets already loaded */

	return iErrorCode;
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
	(*ptrpAsset)->sRefAssetMan = NULL;

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

	if ((*ptrpAsset)->sRefAssetMan)
		Marble_Util_HashTable_Erase((*ptrpAsset)->sRefAssetMan->sHashTable, (*ptrpAsset)->astrAssetId, *ptrpAsset, FALSE);

	switch ((*ptrpAsset)->iAssetType) {
		case Marble_AssetType_Image:      Marble_ImageAsset_Destroy(*ptrpAsset);      break;
		case Marble_AssetType_ColorTable: Marble_ColorTableAsset_Destroy(*ptrpAsset); break;
		case Marble_AssetType_Map:        Marble_MapAsset_Destroy(*ptrpAsset);        break;
	}

	free(*ptrpAsset);
	*ptrpAsset = NULL;
}

int Marble_Asset_LoadFromFile(Marble_Asset *sAsset, TCHAR const *strPath) { MARBLE_ERRNO
	extern int Marble_ImageAsset_LoadFromFile(Marble_Asset *sImage, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead);
	extern int Marble_ColorTableAsset_LoadFromFile(Marble_Asset *sColorTable, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead);
	extern int Marble_MapAsset_LoadFromFile(Marble_Asset *sMap, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead);

	if (!sAsset || !strPath || !*strPath)
		return Marble_ErrorCode_Parameter;

	Marble_Util_FileStream *sStream = NULL;
	if (iErrorCode = Marble_Util_FileStream_Open(strPath, Marble_Util_StreamPerm_Read, &sStream))
		return iErrorCode;
	
	/* Read common asset head */
	Marble_AssetHead sAssetHead = { 0 };
	Marble_Util_FileStream_ReadSize(sStream, sizeof(sAssetHead), &sAssetHead);
	Marble_Asset_Internal_CopyAssetId(&sAssetHead, sAsset->astrAssetId);

	/* Read dependency table and load all found deps */
	if (iErrorCode = Marble_Asset_Internal_EvaluateDependencyTable(sStream, &sAssetHead))
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

int Marble_Asset_CreateAndLoadFromFile(TCHAR const *strPath, Marble_Asset **ptrpAsset) {
	return Marble_ErrorCode_UnimplementedFeature;
}

int Marble_Asset_CreateAndLoadFromFileExplicit(int iAssetType, TCHAR const *strPath, void const *ptrCreateParams, Marble_Asset **ptrpAsset) { MARBLE_ERRNO
	if (!ptrpAsset || !strPath || !*strPath)
		return Marble_ErrorCode_Parameter;

	if (iErrorCode = Marble_Asset_CreateExplicit(iAssetType, ptrCreateParams, ptrpAsset))
		return iErrorCode;

	if (iErrorCode = Marble_Asset_LoadFromFile(*ptrpAsset, strPath)) {
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
/// <returns>Non-zero on error; 0 on success. </returns>
int Marble_Asset_Register(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset) { MARBLE_ERRNO
	sAssetManager = sAssetManager == Marble_DefAssetMan ? gl_sApplication.sAssets : sAssetManager;
	if (!sAssetManager || !sAsset)
		return Marble_ErrorCode_Parameter;

	if (iErrorCode = Marble_Util_HashTable_Insert(sAssetManager->sHashTable, sAsset->astrAssetId, sAsset, FALSE))
		return iErrorCode;

	sAsset->sRefAssetMan = sAssetManager;
	return Marble_ErrorCode_Ok;
}

int Marble_Asset_Unregister(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset, _Bool blDoFree) { MARBLE_ERRNO
	sAssetManager = sAssetManager == Marble_DefAssetMan ? gl_sApplication.sAssets : sAssetManager;
	if (!sAssetManager || !sAsset)
		return Marble_ErrorCode_Parameter;

	if (iErrorCode = Marble_Util_HashTable_Erase(sAssetManager->sHashTable, sAsset->astrAssetId, sAsset, FALSE))
		return iErrorCode;

	sAsset->sRefAssetMan = NULL;
	return Marble_ErrorCode_Ok;
}
#pragma endregion


#pragma region Marble_AssetManager
int Marble_AssetManager_Create(Marble_AssetManager **ptrpAssetManager) { MARBLE_ERRNO
	extern void Marble_AssetManager_Destroy(Marble_AssetManager **ptrpAssetManager);

	if (iErrorCode = Marble_System_AllocateMemory(ptrpAssetManager, sizeof(**ptrpAssetManager), FALSE, TRUE))
		return iErrorCode;

	Marble_IfError(
		CoCreateInstance(
			&CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			&IID_IWICImagingFactory2,
			&(*ptrpAssetManager)->sWICFactory
		), S_OK, {
			Marble_AssetManager_Destroy(ptrpAssetManager);

			return Marble_ErrorCode_CreateWICImagingFactory;
		}
	);

	Marble_IfError(
		Marble_Util_HashTable_Create(
			&(*ptrpAssetManager)->sHashTable,
			64, 
			(void (*)(void **))&Marble_Asset_Destroy
		), Marble_ErrorCode_Ok, {
			Marble_AssetManager_Destroy(ptrpAssetManager);

			return iErrorCode;
		}
	);

	return Marble_ErrorCode_Ok;
}

void Marble_AssetManager_Destroy(Marble_AssetManager **ptrpAssetManager) {
	if (ptrpAssetManager && *ptrpAssetManager) {
		Marble_Util_HashTable_Destroy(&(*ptrpAssetManager)->sHashTable);

		if ((*ptrpAssetManager)->sWICFactory)
			(*ptrpAssetManager)->sWICFactory->lpVtbl->Release((*ptrpAssetManager)->sWICFactory);

		free(*ptrpAssetManager);
		*ptrpAssetManager = NULL;
	}
}
#pragma endregion


