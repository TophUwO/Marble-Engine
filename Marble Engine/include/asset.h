#pragma once

#include <api.h>


typedef struct Marble_Asset {
	ULONGLONG uqwGlobalAssetId;
	int       iAssetType;
} Marble_Asset;


typedef struct Marble_Asset_Image {
	Marble_Asset;

	union {
		ID2D1Bitmap *sD2DBitmap;
	};
} Marble_Asset_Image;


