#pragma once

#include <api.h>


enum Marble_ColorFormat {
	Marble_ColorFormat_Unknown,

	Marble_ColorFormat_RGBAfloat,
	Marble_ColorFormat_RGBAbyte,

	__NUMCOLORFORMATS__
};


extern size_t        Marble_Color_GetColorSizeByFormat(int iColorFormat);
extern _Bool  inline Marble_Color_IsValidColorFormat(int iColorFormat);


