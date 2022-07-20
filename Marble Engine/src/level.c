#include <application.h>


#pragma region Marble_MapAsset
typedef enum Marble_MapAsset_LayerType {
	Marble_MapAsset_LayerType_Unknown = 0,

	Marble_MapAsset_LayerType_Texture
} Marble_MapAsset_LayerType;


typedef struct Marble_MapAsset {
	Marble_Asset;

	struct Marble_MapAsset_Head {
		DWORD dwWidth;
		DWORD dwHeight;
		DWORD dwNumOfLayers;
	} sHead;

	struct Marble_MapAsset_MapLayer {
		int   iLayerType;
		DWORD dwLayerId;

		Marble_Util_Array2D *sTiles;
	} *sLayers;
} Marble_MapAsset;


static int Marble_MapAsset_Internal_InitializeContainer(Marble_MapAsset *sAsset) { MARBLE_ERRNO
	if (iErrorCode = Marble_System_AllocateMemory(&sAsset->sLayers, sAsset->sHead.dwNumOfLayers * sizeof(struct Marble_MapAsset_MapLayer), TRUE, FALSE))
		return iErrorCode;

	for (DWORD dwIndex = 0; dwIndex < sAsset->sHead.dwNumOfLayers; dwIndex++) {
		struct Marble_MapAsset_MapLayer *sLayer = &sAsset->sLayers[dwIndex];

		if (iErrorCode = Marble_Util_Array2D_Create(sizeof(ULONGLONG), sAsset->sHead.dwWidth, sAsset->sHead.dwHeight, &sLayer->sTiles))
			return iErrorCode;

		sLayer->dwLayerId = dwIndex;
	}

	return 0;
}

static void Marble_MapAsset_Internal_DestroyLayer(struct Marble_MapAsset_MapLayer *sLayer) {
	if (!sLayer)
		return;

	Marble_Util_Array2D_Destroy(&sLayer->sTiles);
}


int Marble_Asset_CreateMapAsset(void const *ptrCreateParams, Marble_Asset **ptrpMapAsset) { MARBLE_ERRNO
	if (iErrorCode = Marble_System_AllocateMemory(ptrpMapAsset, sizeof(Marble_MapAsset), TRUE, FALSE))
		return iErrorCode;

	(*ptrpMapAsset)->iAssetType = Marble_AssetType_Map;

	if (ptrCreateParams) {
		Marble_MapAsset              *sMapAsset     = (Marble_MapAsset *)*ptrpMapAsset;
		Marble_MapAsset_CreateParams *sCreateParams = (Marble_MapAsset_CreateParams *)ptrCreateParams;

		/* 
			* Return with error if *sCreateParams* struct contains
			* any invalid (i.e. == 0) data.
		*/
		if (sCreateParams->dwWidth * sCreateParams->dwHeight * sCreateParams->dwNumOfLayers == 0)
			return Marble_ErrorCode_Parameter;

		/* 
			* Copy over the rest of the header 
			* information; struct layout is the same 
			* so no need to copy one-by-one.
		*/
		sMapAsset->sHead = *(struct Marble_MapAsset_Head *)&sCreateParams->dwWidth;

		if (iErrorCode = Marble_MapAsset_Internal_InitializeContainer(sMapAsset))
			return iErrorCode;
	}

	return Marble_ErrorCode_Ok;
}

void Marble_MapAsset_Destroy(Marble_MapAsset *sMapAsset) {
	if (!sMapAsset)
		return;

	if (sMapAsset->sLayers) {
		for (DWORD dwIndex = 0; dwIndex < sMapAsset->sHead.dwNumOfLayers; dwIndex++)
			Marble_MapAsset_Internal_DestroyLayer(&sMapAsset->sLayers[dwIndex]);

		free(sMapAsset->sLayers);
	}
}

int Marble_MapAsset_LoadFromFile(Marble_Asset *sMap, TCHAR const *strPath, Marble_Util_FileStream *sStream, Marble_AssetHead *sAssetHead) {
	return Marble_ErrorCode_UnimplementedFeature;
}
#pragma endregion


