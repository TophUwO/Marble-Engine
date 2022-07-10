#include <color.h>


size_t Marble_Color_GetColorSizeByFormat(int iColorFormat) {
	static struct Marble_Internal_ColorFormatEntry { int iFormat; size_t stSizeInBytes; } const gl_saColorFormatTable[] = {
		{ Marble_ColorFormat_Unknown,   0                 },

		{ Marble_ColorFormat_RGBAfloat, 4 * sizeof(FLOAT) },
		{ Marble_ColorFormat_RGBAbyte,  4 * sizeof(BYTE)  }
	};

	if (iColorFormat < 0 || iColorFormat >= __MARBLE_NUMCOLORFORMATS__)
		return Marble_Color_GetColorSizeByFormat(Marble_ColorFormat_Unknown);

	return gl_saColorFormatTable[iColorFormat].stSizeInBytes;
}

_Bool Marble_Color_IsValidColorFormat(int iColorFormat) {
	return iColorFormat > Marble_ColorFormat_Unknown && iColorFormat < __MARBLE_NUMCOLORFORMATS__;
}


