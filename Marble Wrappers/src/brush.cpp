#include <d2dwr.h>


ULONG WINAPI D2DWr_SolidColorBrush_Release(ID2D1SolidColorBrush *sBrush) {
	return sBrush->Release();
}


