#pragma once

#include <api.h>


#define MARBLE_ASSETIDLEN (24)


typedef enum Marble_AssetDependencies {
	Marble_AssetDependency_Unknown = 0,

	Marble_AssetDependency_Asset
} Marble_AssetDependency;


typedef struct Marble_AssetManager {
	_Bool blIsInit;

	Marble_Util_HashTable *sHashTable;
	IWICImagingFactory    *sWICFactory;
} Marble_AssetManager;


typedef struct Marble_AssetHead {
	DWORD dwMagic;                        /* magic number "mb" + asset type id (2 + 2 bytes) */
	/*
	* dwFlags reference
	* 
	* +--------+--------+--------+--------+
	* |        |        |        |        |
	* +--------+--------+--------+--------+
	* 0        7        15       23       31
	* 
	* [0] ... asset is persistent (will not be unloaded when refcount becomes 0)
	*/
	DWORD dwFlags;                        /* flags */
	CHAR  astrAssetId[MARBLE_ASSETIDLEN]; /* asset string id */
	DWORD dwNumOfDeps;                    /* number of dependency table entries */
	DWORD dwOffAssetHead;                 /* offset to asset-specific head from begin of file */
	DWORD dwOffData;                      /* offset to asset data from begin of file */
} Marble_AssetHead;

typedef struct Marble_Asset {
	int   iAssetType;
	CHAR  astrAssetId[MARBLE_ASSETIDLEN];
	int   iRefCount;
	_Bool blIsPersist;

	Marble_Util_Vector *sDependencies;
} Marble_Asset;


extern int   Marble_Asset_Create(Marble_Asset **ptrpAsset);
extern int   Marble_Asset_CreateExplicit(int iAssetType, void const *ptrCreateParams, Marble_Asset **ptrpAsset);
extern int   Marble_Asset_LoadFromFile(Marble_Asset *sAsset, TCHAR const *strPath, void **ptrpExistingAssetPtr);
extern int   Marble_Asset_CreateAndLoadFromFile(TCHAR const *strPath, Marble_Asset **ptrpAsset, void **ptrpExistingAssetPtr);
extern int   Marble_Asset_CreateAndLoadFromFileExplicit(int iAssetType, TCHAR const *strPath, void const *ptrCreateParams, Marble_Asset **ptrpAsset, void **ptrpExistingAssetPtr);
extern void  Marble_Asset_Destroy(Marble_Asset **ptrpAsset);
extern int   Marble_Asset_GetType(Marble_Asset *sAsset);
extern CHAR *Marble_Asset_GetId(Marble_Asset *sAsset);
extern void  Marble_Asset_SetId(Marble_Asset *sAsset, CHAR const *astrId);
extern int   Marble_Asset_Register(Marble_Asset *sAsset);
extern int   Marble_Asset_Unregister(Marble_Asset *sAsset, _Bool blDoFree);
extern int   Marble_Asset_Obtain(Marble_Asset *sAsset);
extern int   Marble_Asset_Release(Marble_Asset *sAsset);


