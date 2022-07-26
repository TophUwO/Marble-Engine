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


// TODO: proper struct packing
typedef struct Marble_CommonAssetHead {
	union { /* magic number "mb" + asset type id (2 + 2 bytes) */
		DWORD dwMagic;

		struct {
			WORD uMagic;
			WORD uAssetType;
		};
	};
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
	CHAR  astrAssetID[MARBLE_ASSETIDLEN]; /* asset string id */
	DWORD dwNumOfDeps;                    /* number of dependency table entries */
	DWORD dwOffAssetHead;                 /* offset to asset-specific head from begin of file */
	DWORD dwOffData;                      /* offset to asset data from begin of file */
} Marble_CommonAssetHead;

typedef struct Marble_Asset {
	int   iAssetType;
	CHAR  astrAssetID[MARBLE_ASSETIDLEN];
	int   iRefCount;

	Marble_Util_Vector *sDependencies;
} Marble_Asset;


extern int   Marble_Asset_Create(int iAssetType, CHAR const *strID, void const *ptrCreateParams, Marble_Asset **ptrpAsset, Marble_Asset **ptrpExistingAssetPtr);
extern int   Marble_Asset_LoadFromFile(TCHAR const *strPath, Marble_Asset **ptrpAsset, Marble_Asset **ptrpExistingAssetPtr);
extern void  Marble_Asset_Destroy(Marble_Asset **ptrpAsset, _Bool blIsInternal);
extern int   Marble_Asset_GetType(Marble_Asset *sAsset);
extern CHAR *Marble_Asset_GetId(Marble_Asset *sAsset);
extern int   Marble_Asset_Register(Marble_Asset *sAsset);
extern int   Marble_Asset_Unregister(Marble_Asset *sAsset, _Bool blDoFree);
extern int   Marble_Asset_Obtain(Marble_Asset *sAsset);
extern int   Marble_Asset_Release(Marble_Asset *sAsset);
extern int   Marble_Asset_Render(Marble_Asset *sAsset, Marble_Renderer *sRenderer);


