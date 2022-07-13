#include <d2dwr.h>


ULONG WINAPI D2DWr_Bitmap_Release(ID2D1Bitmap *sBitmap) {
	return sBitmap->Release();
}


