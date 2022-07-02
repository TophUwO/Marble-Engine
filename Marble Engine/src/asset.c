#include <application.h>


#pragma region Marble_Asset
struct Marble_Internal_AtlasCP { int iAtlasType; };


int Marble_Asset_Create(int iAssetType, Marble_Asset **ptrpAsset, void *ptrCreateParams) {
	extern int Marble_Atlas_Create(int iAtlasType, Marble_Atlas **ptrpAtlas);

	if (ptrpAsset) {
		switch (iAssetType) {
			case Marble_AssetType_Atlas: {
				struct Marble_Internal_AtlasCP *sCreateParams = (struct Marble_Internal_AtlasCP *)ptrCreateParams;

				return Marble_Atlas_Create(
					sCreateParams->iAtlasType, 
					(Marble_Atlas **)ptrpAsset
				);
			}
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
int Marble_AssetManager_Create(void) {
	extern void Marble_AssetManager_Destroy(void);

	if (gl_sApplication.sAssets = malloc(sizeof(*gl_sApplication.sAssets))) {
		int iErrorCode = Marble_ErrorCode_Ok;

		Marble_IfError(
			Marble_Util_Vector_Create(
				&gl_sApplication.sAssets->sAtlases,
				64, 
				(void (*)(void **))&Marble_Asset_Destroy),
			Marble_ErrorCode_Ok, {
				Marble_AssetManager_Destroy();

				return iErrorCode;
			}
		);

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_MemoryAllocation;
}

void Marble_AssetManager_Destroy(void) {
	if (gl_sApplication.sAssets) {
		Marble_Util_Vector_Destroy(&gl_sApplication.sAssets->sAtlases);

		free(gl_sApplication.sAssets);
		gl_sApplication.sAssets = NULL;
	}
}

int Marble_AssetManager_RegisterAsset(Marble_Asset *sAsset) {

}

int Marble_AssetManager_UnregisterAsset(Marble_Asset *sAsset) {

}
#pragma endregion


