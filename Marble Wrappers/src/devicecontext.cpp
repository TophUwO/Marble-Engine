#include <d2dwr.h>


HRESULT WINAPI D2DWr_DeviceContext_CreateBitmapFromDxgiSurface(ID2D1DeviceContext *sDeviceContext, IDXGISurface *sDXGISurface, D2D1_BITMAP_PROPERTIES1 const *sBitmapProps, ID2D1Bitmap1 **ptrpBitmap) {
	return sDeviceContext->CreateBitmapFromDxgiSurface(sDXGISurface, sBitmapProps, ptrpBitmap);
}

void WINAPI D2DWr_DeviceContext_SetTarget(ID2D1DeviceContext *sDeviceContext, ID2D1Image *sImage) {
	sDeviceContext->SetTarget(sImage);
}

void WINAPI D2DWr_DeviceContext_Clear(ID2D1DeviceContext *sDeviceContext, D2D1_COLOR_F const *sClearColor) {
	sDeviceContext->Clear(sClearColor);
}

void WINAPI D2DWr_DeviceContext_BeginDraw(ID2D1DeviceContext *sDeviceContext) {
	sDeviceContext->BeginDraw();
}

HRESULT WINAPI D2DWr_DeviceContext_EndDraw(ID2D1DeviceContext *sDeviceContext, D2D1_TAG *uqwpTag1, D2D1_TAG *uqwpTag2) {
	return sDeviceContext->EndDraw(uqwpTag1, uqwpTag2);
}

HRESULT WINAPI D2DWr_DeviceContext_CreateSolidColorBrush(ID2D1DeviceContext *sDeviceContext, D2D1_COLOR_F const *sColor, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1SolidColorBrush **ptrpBrush) {
	return sDeviceContext->CreateSolidColorBrush(sColor, sBrushProps, ptrpBrush);
}

void WINAPI D2DWr_DeviceContext_DrawRectangle(ID2D1DeviceContext *sDeviceContext, D2D1_RECT_F const *sRect, ID2D1Brush *sBrush, FLOAT fStrokeWith, ID2D1StrokeStyle *sStrokeStyle) {
	sDeviceContext->DrawRectangle(sRect, sBrush, fStrokeWith, sStrokeStyle);
}

void WINAPI D2DWr_DeviceContext_FillRectangle(ID2D1DeviceContext *sDeviceContext, D2D1_RECT_F const *sRect, ID2D1Brush *sBrush) {
	sDeviceContext->FillRectangle(sRect, sBrush);
}

ULONG WINAPI D2DWr_DeviceContext_Release(ID2D1DeviceContext *sDeviceContext) {
	return sDeviceContext->Release();
}


