#pragma once

#include <api.h>


enum Marble_AssetType {
	Marble_AssetType_Unknown,

	Marble_AssetType_Atlas
}; 


typedef struct Marble_Asset Marble_Asset;

MARBLE_API int  Marble_Asset_Create(int iAssetType, Marble_Asset **ptrpAsset, void *ptrCreateParams);
MARBLE_API void Marble_Asset_Destroy(Marble_Asset **ptrpAsset);
MARBLE_API int  Marble_Asset_GetType(Marble_Asset *sAsset);

MARBLE_API int  Marble_AssetManager_RegisterAsset(Marble_Asset *sAsset);
MARBLE_API int  Marble_AssetManager_UnregisterAsset(Marble_Asset *sAsset);