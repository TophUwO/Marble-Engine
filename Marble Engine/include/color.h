#pragma once

#include <api.h>


typedef enum Marble_ColorFormats {
	Marble_ColorFormat_Unknown,

	Marble_ColorFormat_RGBAfloat,
	Marble_ColorFormat_RGBAbyte,

	__MARBLE_NUMCOLORFORMATS__
} Marble_ColorFormat;

typedef struct Marble_ColorTableAsset Marble_ColorTableAsset;


extern size_t        Marble_Color_GetColorSizeByFormat(int iColorFormat);
extern _Bool  inline Marble_Color_IsValidColorFormat(int iColorFormat);

extern int        Marble_ColorTable_InitializeContainer(Marble_ColorTableAsset *sColorTable, int iColorFormat, size_t *stpColorEntrySize);
extern int inline Marble_ColorTable_GetColorByIndex(Marble_ColorTableAsset *sColorTable, size_t stIndex, void *ptrColor);


