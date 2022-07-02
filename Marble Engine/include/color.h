#pragma once

#include <api.h>


enum Marble_ColorFormat {
	Marble_ColorFormat_Unknown,

	Marble_ColorFormat_RGBAfloat,
	Marble_ColorFormat_RGBAbyte,

	__NUMCOLORFORMATS__
};


extern size_t Marble_Color_GetColorSizeByFormat(enum Marble_ColorFormat eFormat);


