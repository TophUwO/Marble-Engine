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

typedef struct Marble_ImageAsset {
	Marble_Asset;

	union {
		ID2D1Bitmap *sD2DBitmap;
	};
} Marble_ImageAsset;


