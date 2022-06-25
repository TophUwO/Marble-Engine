#include <wr_d2d.h>


HRESULT WINAPI WrD2D_Factory_CreateDCRenderTarget(ID2D1Factory *sFactory, D2D1_RENDER_TARGET_PROPERTIES const *sProps, ID2D1DCRenderTarget **ptrpDCRenderTarget) {
	return sFactory->CreateDCRenderTarget(sProps, ptrpDCRenderTarget);
}

HRESULT WINAPI WrD2D_Factory_CreateDrawingStateBlock(ID2D1Factory *sFactory, D2D1_DRAWING_STATE_DESCRIPTION const *sDesc, IDWriteRenderingParams *sRenderingParams, ID2D1DrawingStateBlock **ptrpDrawingStateBlock) {
	return sFactory->CreateDrawingStateBlock(sDesc, sRenderingParams, ptrpDrawingStateBlock);
}

HRESULT WINAPI WrD2D_Factory_CreateDxgiSurfaceRenderTarget(ID2D1Factory *sFactory, IDXGISurface *sDxgiSurface, D2D1_RENDER_TARGET_PROPERTIES const *sProps, ID2D1RenderTarget **ptrpDxgiRenderTarget) {
	return sFactory->CreateDxgiSurfaceRenderTarget(sDxgiSurface, sProps, ptrpDxgiRenderTarget);
}

HRESULT WINAPI WrD2D_Factory_CreateEllipseGeometry(ID2D1Factory *sFactory, D2D1_ELLIPSE *sEllipse, ID2D1EllipseGeometry **ptrpEllipseGeometry) {
	return sFactory->CreateEllipseGeometry(sEllipse, ptrpEllipseGeometry);
}

HRESULT WINAPI WrD2D_Factory_CreateGeometryGroup(ID2D1Factory *sFactory, D2D1_FILL_MODE eFillMode, ID2D1Geometry **spGeometries, UINT32 udwGeomCount, ID2D1GeometryGroup **ptrpGeometryGroup) {
	return sFactory->CreateGeometryGroup(eFillMode, spGeometries, udwGeomCount, ptrpGeometryGroup);
}

HRESULT WINAPI WrD2D_Factory_CreateHwndRenderTarget(ID2D1Factory *sFactory, D2D1_RENDER_TARGET_PROPERTIES const *sRTProps, D2D1_HWND_RENDER_TARGET_PROPERTIES const *sHWNDRTProps, ID2D1HwndRenderTarget **ptrpHwndRenderTarget) {
	return sFactory->CreateHwndRenderTarget(sRTProps, sHWNDRTProps, ptrpHwndRenderTarget);
}

HRESULT WINAPI WrD2D_Factory_CreatePathGeometry(ID2D1Factory *sFactory, ID2D1PathGeometry **ptrpPathGeometry) {
	return sFactory->CreatePathGeometry(ptrpPathGeometry);
}

HRESULT WINAPI WrD2D_Factory_CreateRectangleGeometry(ID2D1Factory *sFactory, D2D1_RECT_F const *sRect, ID2D1RectangleGeometry **ptrpRectangleGeometry) {
	return sFactory->CreateRectangleGeometry(sRect, ptrpRectangleGeometry);
}

HRESULT WINAPI WrD2D_Factory_CreateRoundedRectangleGeometry(ID2D1Factory *sFactory, D2D1_ROUNDED_RECT const *sRoundedRect, ID2D1RoundedRectangleGeometry **ptrpRoundedRectangleGeometry) {
	return sFactory->CreateRoundedRectangleGeometry(sRoundedRect, ptrpRoundedRectangleGeometry);
}

HRESULT WINAPI WrD2D_Factory_CreateStrokeStyle(ID2D1Factory *sFactory, D2D1_STROKE_STYLE_PROPERTIES *const sStrokeStyleProps, FLOAT const *fpDashes, UINT32 udwDashesCount,ID2D1StrokeStyle **ptrpStrokeStyle) {
	return sFactory->CreateStrokeStyle(sStrokeStyleProps, fpDashes, udwDashesCount, ptrpStrokeStyle);
}

HRESULT WINAPI WrD2D_Factory_CreateTransformedGeometry(ID2D1Factory *sFactory, ID2D1Geometry *sSourceGeometry, D2D1_MATRIX_3X2_F const *sTransform, ID2D1TransformedGeometry **ptrpTransformedGeometry) {
	return sFactory->CreateTransformedGeometry(sSourceGeometry, sTransform, ptrpTransformedGeometry);
}

HRESULT WINAPI WrD2D_Factory_CreateWicBitmapRenderTarget(ID2D1Factory *sFactory, IWICBitmap *sTarget, D2D1_RENDER_TARGET_PROPERTIES const *sProps, ID2D1RenderTarget **ptrpWicBitmapRenderTarget) {
	return sFactory->CreateWicBitmapRenderTarget(sTarget, sProps, ptrpWicBitmapRenderTarget);
}

void WINAPI WrD2D_Factory_GetDesktopDPI(ID2D1Factory *sFactory, FLOAT *fpDpiX, FLOAT *fpDpiY) {
	sFactory->GetDesktopDpi(fpDpiX, fpDpiY);
}

HRESULT WINAPI WrD2D_Factory_ReloadSystemMetrics(ID2D1Factory *sFactory) {
	return sFactory->ReloadSystemMetrics();
}


