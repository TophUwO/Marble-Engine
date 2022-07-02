#pragma once

#include <util.h>
#include <asset.h>


enum Marble_AtlasType {
	Marble_AtlasType_Unknown,

	Marble_AtlasType_ColorAtlas
};


typedef struct Marble_ColorAtlas Marble_ColorAtlas;

MARBLE_API int        Marble_ColorAtlas_LoadFromFile(Marble_ColorAtlas *sAtlas, TCHAR const *strPath);
MARBLE_API int inline Marble_ColorAtlas_GetColorByIndex(Marble_ColorAtlas *sAtlas, size_t stIndex, void *ptrColor);


