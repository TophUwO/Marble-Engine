#include <d2dwr.h>


HRESULT WINAPI D2DWr_StrokeStyle_Release(ID2D1StrokeStyle *sStrokeStyle) {
	return sStrokeStyle->Release();
}


