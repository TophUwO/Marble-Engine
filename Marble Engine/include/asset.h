#pragma once

#include <api.h>


typedef struct Marble_AssetManager {
	Marble_Util_Vector *sAtlases;

	IWICImagingFactory *sWICFactory;
} Marble_AssetManager;


typedef struct Marble_Asset {
	ULONGLONG uqwGlobalAssetId;
	int       iAssetType;
} Marble_Asset;


