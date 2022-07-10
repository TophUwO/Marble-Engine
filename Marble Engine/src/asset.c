#include <application.h>
#include <marble.h>


static ULONGLONG volatile gl_uqwGlobalAssetId = 0;


#pragma region Marble_Asset
int Marble_Asset_Create(int iAssetType, Marble_Asset **ptrpAsset, void const *ptrCreateParams) {
	extern int Marble_Atlas_Create(int iAtlasType, Marble_Atlas **ptrpAtlas);
	extern int Marble_Asset_CreateImageAsset(Marble_Asset **ptrpAsset);

	if (ptrpAsset) {
		switch (iAssetType) {
			case Marble_AssetType_Atlas: {
				struct Marble_Atlas_CreateParams *sCreateParams = (struct Marble_Atlas_CreateParams *)ptrCreateParams;
				int iErrorCode = Marble_Atlas_Create(
					sCreateParams->iAtlasType,
					(Marble_Atlas **)ptrpAsset
				);

				if (!iErrorCode) {
					((Marble_Asset *)*ptrpAsset)->iAssetType       = iAssetType;
					((Marble_Asset *)*ptrpAsset)->uqwGlobalAssetId = InterlockedIncrement64(&gl_uqwGlobalAssetId);
				}

				return iErrorCode;
			}
			case Marble_AssetType_Image: return Marble_Asset_CreateImageAsset(ptrpAsset);
		}

		*ptrpAsset = NULL;
		return Marble_ErrorCode_AssetType;
	}

	return Marble_ErrorCode_Parameter;
}

void Marble_Asset_Destroy(Marble_Asset **ptrpAsset) {
	extern void Marble_Atlas_Destroy(Marble_Atlas **ptrpAtlas);

	if (ptrpAsset && *ptrpAsset) {
		switch ((*ptrpAsset)->iAssetType) {
			case Marble_AssetType_Atlas: Marble_Atlas_Destroy((Marble_Atlas **)ptrpAsset);
		}
	}
}

int Marble_Asset_GetType(Marble_Asset *sAsset) {
	return sAsset ? sAsset->iAssetType : Marble_AssetType_Unknown;
}
#pragma endregion


#pragma region Marble_AssetManager
int Marble_AssetManager_Create(Marble_AssetManager **ptrpAssetManager) {
	extern void Marble_AssetManager_Destroy(Marble_AssetManager **ptrpAssetManager);

	if (Marble_System_AllocateMemory(ptrpAssetManager, sizeof(**ptrpAssetManager), FALSE)) {
		*ptrpAssetManager = NULL;

		return Marble_ErrorCode_InternalParameter;
	}

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

	int iErrorCode = 0;
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


