#include <application.h>
#include <marble.h>


#pragma region Marble_Asset
static void Marble_Asset_Internal_CopyAssetId(Marble_AssetHead *sAssetHead, CHAR const *astrId) {
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
	if (iErrorCode = Marble_Asset_CreateAndLoadFromFile(wstrAssetPath, NULL, &sAsset))
		return iErrorCode;

	if (iErrorCode = Marble_Asset_Register(gl_sApplication.sAssets, sAsset)) {
		Marble_Asset_Destroy(&sAsset);

		return iErrorCode;
	}

	return Marble_ErrorCode_Ok;
}

static int Marble_Asset_Internal_EvaluateDependencyTable(Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead) { MARBLE_ERRNO
	static struct Marble_Asset_Internal_DependencyTableEntry { BYTE bCommand; BYTE bNumOfParams; } const gl_sAssetDepTableEntries[] = {
		{ Marble_AssetDependency_Unknown, 0 },

		{ Marble_AssetDependency_Asset,   2 }
	};

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


int Marble_Asset_Create(int iAssetType, void const *ptrCreateParams, Marble_Asset **ptrpAsset) { MARBLE_ERRNO
	extern int Marble_Asset_CreateImageAsset(Marble_Asset **ptrpImageAsset);
	extern int Marble_Asset_CreateColorTableAsset(Marble_Asset **ptrpColorTable);

	if (ptrpAsset) {
		switch (iAssetType) {
			case Marble_AssetType_Image:      iErrorCode = Marble_Asset_CreateImageAsset(ptrpAsset); break;
			case Marble_AssetType_ColorTable: iErrorCode = Marble_Asset_CreateColorTableAsset(ptrpAsset); break;
			default:
				*ptrpAsset = NULL;

				return Marble_ErrorCode_AssetType;
		}

		/* Free generic asset structure */
		if (iErrorCode)
			Marble_Asset_Destroy(ptrpAsset);

		return iErrorCode;
	}

	return Marble_ErrorCode_Parameter;
}

void Marble_Asset_Destroy(Marble_Asset **ptrpAsset) {
	extern void Marble_ImageAsset_Destroy(Marble_Asset *sImage);
	extern void Marble_ColorTableAsset_Destroy(Marble_Asset *sColorTable);

	if (!ptrpAsset || !*ptrpAsset)
		return;

	switch ((*ptrpAsset)->iAssetType) {
		case Marble_AssetType_Image:      Marble_ImageAsset_Destroy(*ptrpAsset);
		case Marble_AssetType_ColorTable: Marble_ColorTableAsset_Destroy(*ptrpAsset);
	}

	free(*ptrpAsset);
	*ptrpAsset = NULL;
}

int Marble_Asset_LoadFromFile(Marble_Asset *sAsset, TCHAR const *strPath) { MARBLE_ERRNO
	extern int Marble_ImageAsset_LoadFromFile(Marble_Asset *sImage, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead);
	extern int Marble_ColorTableAsset_LoadFromFile(Marble_Asset *sColorTable, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead);

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
		case Marble_AssetType_Image:      iErrorCode = Marble_ImageAsset_LoadFromFile(sAsset, strPath, sStream, &sAssetHead); break;
		case Marble_AssetType_ColorTable: iErrorCode = Marble_ColorTableAsset_LoadFromFile(sAsset, strPath, sStream, &sAssetHead); break;
		default:
			iErrorCode = Marble_ErrorCode_AssetType;
	}

	Marble_Util_FileStream_Destroy(&sStream);
	return iErrorCode;
}

int Marble_Asset_CreateAndLoadFromFileExplicit(int iAssetType, TCHAR const *strPath, void const *ptrCreateParams, Marble_Asset **ptrpAsset) { MARBLE_ERRNO
	if (!ptrpAsset || !strPath || !*strPath)
		return Marble_ErrorCode_Parameter;

	if (iErrorCode = Marble_Asset_Create(iAssetType, ptrCreateParams, ptrpAsset))
		return iErrorCode;

	if (iErrorCode = Marble_Asset_LoadFromFile(*ptrpAsset, strPath)) {
		Marble_Asset_Destroy(ptrpAsset);

		return iErrorCode;
	}

	return Marble_ErrorCode_Ok;
}

int Marble_Asset_CreateAndLoadFromFile(TCHAR const *strPath, void const *ptrCreateParams, Marble_Asset **ptrpAsset) {

}

int Marble_Asset_GetType(Marble_Asset *sAsset) {
	return sAsset ? sAsset->iAssetType : Marble_AssetType_Unknown;
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
		Marble_Util_Vector_Create(
			&(*ptrpAssetManager)->sAtlases,
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
		Marble_Util_Vector_Destroy(&(*ptrpAssetManager)->sAtlases);

		if ((*ptrpAssetManager)->sWICFactory)
			(*ptrpAssetManager)->sWICFactory->lpVtbl->Release((*ptrpAssetManager)->sWICFactory);

		free(*ptrpAssetManager);
		*ptrpAssetManager = NULL;
	}
}

int Marble_Asset_Register(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset) {
	if (sAsset && sAssetManager)
		return Marble_Util_Vector_PushBack(sAssetManager->sAtlases, sAsset); // for testing

	return Marble_ErrorCode_Parameter;
}

int Marble_Asset_Unregister(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset, _Bool blDoFree) {
	return Marble_ErrorCode_UnimplementedFeature;
}
#pragma endregion


