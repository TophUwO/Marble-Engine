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

HRESULT WINAPI D2DWr_DeviceContext_CreateBitmapFromWicBitmap(ID2D1DeviceContext *sDeviceContext, IWICBitmapSource *sBitmapSource, D2D1_BITMAP_PROPERTIES1 const *sBitmapProps, ID2D1Bitmap1 **ptrpBitmap) {
	return sDeviceContext->CreateBitmapFromWicBitmap(sBitmapSource, sBitmapProps, ptrpBitmap);
}

void WINAPI D2DWr_DeviceContext_DrawBitmap(ID2D1DeviceContext *sDeviceContext, ID2D1Bitmap *sBitmap, D2D1_RECT_F const *sDstRect, FLOAT fOpacity, D2D1_INTERPOLATION_MODE eInterpolationMode, D2D1_RECT_F const *sSrcRect, D2D1_MATRIX_4X4_F const *sPerspectiveTransform) {
	sDeviceContext->DrawBitmap(sBitmap, sDstRect, fOpacity, eInterpolationMode, sSrcRect, sPerspectiveTransform);
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


