#pragma once

#include <api.h>


enum Marble_ColorFormat {
	Marble_ColorFormat_Unknown,

	Marble_ColorFormat_RGBAfloat,
	Marble_ColorFormat_RGBAbyte,

	__MARBLE_NUMCOLORFORMATS__
};


extern size_t        Marble_Color_GetColorSizeByFormat(int iColorFormat);
extern _Bool  inline Marble_Color_IsValidColorFormat(int iColorFormat);

extern int inline Marble_ColorTable_GetColorByIndex(Marble_Asset *sColorTable, size_t stIndex, void *ptrColor);


