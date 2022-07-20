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

	Marble_AssetManager *sRefAssetMan;
} Marble_Asset;


extern int   Marble_Asset_Create(Marble_Asset **ptrpAsset);
extern int   Marble_Asset_CreateExplicit(int iAssetType, void const *ptrCreateParams, Marble_Asset **ptrpAsset);
extern int   Marble_Asset_CreateAndLoadFromFile(TCHAR const *strPath, Marble_Asset **ptrpAsset);
extern int   Marble_Asset_CreateAndLoadFromFileExplicit(int iAssetType, TCHAR const *strPath, void const *ptrCreateParams, Marble_Asset **ptrpAsset);
extern int   Marble_Asset_LoadFromFile(Marble_Asset *sAsset, TCHAR const *strPath);
extern void  Marble_Asset_Destroy(Marble_Asset **ptrpAsset);
extern int   Marble_Asset_GetType(Marble_Asset *sAsset);
extern CHAR *Marble_Asset_GetId(Marble_Asset *sAsset);
extern void  Marble_Asset_SetId(Marble_Asset *sAsset, CHAR const *astrId);
extern int   Marble_Asset_Register(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset);
extern int   Marble_Asset_Unregister(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset, _Bool blDoFree);


