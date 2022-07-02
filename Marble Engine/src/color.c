#include <color.h>


size_t Marble_Color_GetColorSizeByFormat(enum Marble_ColorFormat eFormat) {
	static struct Marble_Internal_ColorFormatEntry { int iFormat; size_t stSizeInBytes; } const gl_saColorFormatTable[] = {
		{ Marble_ColorFormat_Unknown,   0                 },

		{ Marble_ColorFormat_RGBAfloat, 4 * sizeof(FLOAT) },
		{ Marble_ColorFormat_RGBAbyte,  4 * sizeof(BYTE)  }
	};

	if (eFormat < 0 || eFormat >= __NUMCOLORFORMATS__)
		return Marble_Color_GetColorSizeByFormat(Marble_ColorFormat_Unknown);

	return gl_saColorFormatTable[eFormat].stSizeInBytes;
}


