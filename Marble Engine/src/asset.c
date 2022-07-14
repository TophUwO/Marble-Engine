#include <application.h>
#include <marble.h>


static ULONGLONG volatile gl_uqwGlobalAssetId = 0;


#pragma region Marble_Asset
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

int Marble_Asset_LoadFromFile(Marble_Asset *sAsset, TCHAR const *strPath) {
	extern int Marble_ImageAsset_LoadFromFile(Marble_Asset *sImage, TCHAR const *strPath);
	extern int Marble_ColorTableAsset_LoadFromFile(Marble_Asset *sColorTable, TCHAR const *strPath);

	if (!sAsset || !strPath || !*strPath)
		return Marble_ErrorCode_Parameter;

	switch (sAsset->iAssetType) {
		case Marble_AssetType_Image:      return Marble_ImageAsset_LoadFromFile(sAsset, strPath);
		case Marble_AssetType_ColorTable: return Marble_ColorTableAsset_LoadFromFile(sAsset, strPath);
	}

	return Marble_ErrorCode_AssetType;
}

int Marble_Asset_CreateAndLoadFromFile(int iAssetType, TCHAR const *strPath, void const *ptrCreateParams, Marble_Asset **ptrpAsset) { MARBLE_ERRNO
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

ULONGLONG volatile Marble_AssetManager_RequestAssetId(void) {
	return InterlockedIncrement64(&gl_uqwGlobalAssetId);
}
#pragma endregion


