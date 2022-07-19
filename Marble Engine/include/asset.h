#pragma once

#include <api.h>


#define MARBLE_ASSETIDLEN (24)


typedef enum Marble_AssetDependencies {
	Marble_AssetDependency_Unknown = 0,

	Marble_AssetDependency_Asset
} Marble_AssetDependency;


typedef struct Marble_AssetManager {
	Marble_Util_HashTable *sHashTable;

	IWICImagingFactory *sWICFactory;
} Marble_AssetManager;


typedef struct Marble_AssetHead {
	DWORD dwMagic;
	CHAR  astrAssetId[MARBLE_ASSETIDLEN];
	DWORD dwNumOfDeps;
	DWORD dwOffDepTable;
	DWORD dwOffData;
} Marble_AssetHead;

typedef struct Marble_Asset {
	int  iAssetType;
	CHAR astrAssetId[MARBLE_ASSETIDLEN];
} Marble_Asset;


