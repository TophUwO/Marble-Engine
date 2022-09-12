#include <d2dwr.h>


void WINAPI D2DWr_RenderTarget_BeginDraw(ID2D1RenderTarget *sRenderTarget) {
	sRenderTarget->BeginDraw();
}

void WINAPI D2DWr_RenderTarget_Clear(ID2D1RenderTarget *sRenderTarget, D2D1_COLOR_F const *sClearColor) {
	sRenderTarget->Clear(sClearColor);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateBitmap(ID2D1RenderTarget *sRenderTarget, D2D1_SIZE_U sSize, void const *ptrSrcData, UINT32 udwPitch, D2D1_BITMAP_PROPERTIES const *sProps, ID2D1Bitmap **ptrpBitmap) {
	return sRenderTarget->CreateBitmap(sSize, ptrSrcData, udwPitch, sProps, ptrpBitmap);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateBitmapBrush(ID2D1RenderTarget *sRenderTarget, ID2D1Bitmap *sBitmap, D2D1_BITMAP_BRUSH_PROPERTIES const *sBitmapBrushProps, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1BitmapBrush **ptrpBitmapBrush) {
	return sRenderTarget->CreateBitmapBrush(sBitmap, sBitmapBrushProps, sBrushProps, ptrpBitmapBrush);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateBitmapFromWicBitmap(ID2D1RenderTarget *sRenderTarget, IWICBitmapSource *sSourceBitmap, D2D1_BITMAP_PROPERTIES const *sBitmapProps, ID2D1Bitmap **ptrpBitmap) {
	return sRenderTarget->CreateBitmapFromWicBitmap(sSourceBitmap, sBitmapProps, ptrpBitmap);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateCompatibleRenderTarget(ID2D1RenderTarget *sRenderTarget, D2D1_SIZE_F const *sDesiredSize, D2D1_SIZE_U const *sDesiredPixelSize, D2D1_PIXEL_FORMAT const *sDesiredPixelFormat, D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS eOptions, ID2D1BitmapRenderTarget **ptrpCompBitmapRenderTarget) {
	return sRenderTarget->CreateCompatibleRenderTarget(sDesiredSize, sDesiredPixelSize, sDesiredPixelFormat, eOptions, ptrpCompBitmapRenderTarget);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateGradientStopCollection(ID2D1RenderTarget *sRenderTarget, D2D1_GRADIENT_STOP const *sGradientStops, UINT32 udwGradientStopCount, D2D1_GAMMA eColorInterpolationGamma, D2D1_EXTEND_MODE eExtendMode, ID2D1GradientStopCollection **ptrpGradientStopCollection) {
	return sRenderTarget->CreateGradientStopCollection(sGradientStops, udwGradientStopCount, eColorInterpolationGamma, eExtendMode, ptrpGradientStopCollection);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateLayer(ID2D1RenderTarget *sRenderTarget, D2D1_SIZE_F const *sSize, ID2D1Layer **ptrpLayer) {
	return sRenderTarget->CreateLayer(sSize, ptrpLayer);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateMesh(ID2D1RenderTarget *sRenderTarget, ID2D1Mesh **ptrpMesh) {
	return sRenderTarget->CreateMesh(ptrpMesh);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateLinearGradientBrush(ID2D1RenderTarget *sRenderTarget, D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES const *sLGBrushProps, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1GradientStopCollection *sGradientStopCollection, ID2D1LinearGradientBrush **ptrpLinearGradientBrush) {
	return sRenderTarget->CreateLinearGradientBrush(sLGBrushProps, sBrushProps, sGradientStopCollection, ptrpLinearGradientBrush);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateRadialGradientBrush(ID2D1RenderTarget *sRenderTarget, D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES const *sRBrushProps, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1GradientStopCollection *sGradientStopCollection, ID2D1RadialGradientBrush **ptrpRadialGradientBrush) {
	return sRenderTarget->CreateRadialGradientBrush(sRBrushProps, sBrushProps, sGradientStopCollection, ptrpRadialGradientBrush);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateSharedBitmap(ID2D1RenderTarget *sRenderTarget, IID const *ptrIID, void *ptrData,D2D1_BITMAP_PROPERTIES const *sBitmapProps, ID2D1Bitmap **ptrpBitmap) {
	return sRenderTarget->CreateSharedBitmap(*ptrIID, ptrData, sBitmapProps, ptrpBitmap);
}

HRESULT WINAPI D2DWr_RenderTarget_CreateSolidColorBrush(ID2D1RenderTarget *sRenderTarget, D2D1_COLOR_F const *sColor, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1SolidColorBrush **ptrpSolidColorBrush) {
	return sRenderTarget->CreateSolidColorBrush(sColor, sBrushProps, ptrpSolidColorBrush);
}

void WINAPI D2DWr_RenderTarget_DrawBitmap(ID2D1RenderTarget *sRenderTarget, ID2D1Bitmap *sBitmap, D2D1_RECT_F const *sDestinationRect, FLOAT fOpacity, D2D1_BITMAP_INTERPOLATION_MODE eInterpolationMode, D2D1_RECT_F const *sSourceRect) {
	sRenderTarget->DrawBitmap(sBitmap, sDestinationRect, fOpacity, eInterpolationMode, sSourceRect);
}

void WINAPI D2DWr_RenderTarget_DrawEllipse(ID2D1RenderTarget *sRenderTarget, D2D1_ELLIPSE const *sEllipse, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle) {
	sRenderTarget->DrawEllipse(sEllipse, sBrush, fStrokeWidth, sStrokeStyle);
}

void WINAPI D2DWr_RenderTarget_DrawGeometry(ID2D1RenderTarget *sRenderTarget, ID2D1Geometry *sGeometry, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle) {
	sRenderTarget->DrawGeometry(sGeometry, sBrush, fStrokeWidth, sStrokeStyle);
}

void WINAPI D2DWr_RenderTarget_DrawGlyphRun(ID2D1RenderTarget *sRenderTarget, D2D1_POINT_2F sBaselineOrigin, DWRITE_GLYPH_RUN const *sGlyphRun, ID2D1Brush *sBrush, DWRITE_MEASURING_MODE eMeasuringMode) {
	sRenderTarget->DrawGlyphRun(sBaselineOrigin, sGlyphRun, sBrush, eMeasuringMode);
}

void WINAPI D2DWr_RenderTarget_DrawLine(ID2D1RenderTarget *sRenderTarget, D2D1_POINT_2F sPoint0, D2D1_POINT_2F sPoint1, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle) {
	sRenderTarget->DrawLine(sPoint0, sPoint1, sBrush, fStrokeWidth, sStrokeStyle);
}

void WINAPI D2DWr_RenderTarget_DrawRectangle(ID2D1RenderTarget *sRenderTarget, D2D1_RECT_F const *sRectangle, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle) {
	sRenderTarget->DrawRectangle(sRectangle, sBrush, fStrokeWidth, sStrokeStyle);
}

void WINAPI D2DWr_RenderTarget_DrawRoundedRectangle(ID2D1RenderTarget *sRenderTarget, D2D1_ROUNDED_RECT const *sRoundedRectangle, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle) {
	sRenderTarget->DrawRoundedRectangle(sRoundedRectangle, sBrush, fStrokeWidth, sStrokeStyle);
}

void WINAPI D2DWr_RenderTarget_DrawText(ID2D1RenderTarget *sRenderTarget, WCHAR const *strText, UINT32 udwStringLength, IDWriteTextFormat *sTextFormat, D2D1_RECT_F const *sLayoutRect, ID2D1Brush *sBrush, D2D1_DRAW_TEXT_OPTIONS eOptions, DWRITE_MEASURING_MODE eMeasuringMode) {
	sRenderTarget->DrawTextW(strText, udwStringLength, sTextFormat, sLayoutRect, sBrush, eOptions, eMeasuringMode);
}

void WINAPI D2DWr_RenderTarget_DrawTextLayout(ID2D1RenderTarget *sRenderTarget, D2D1_POINT_2F sOrigin, IDWriteTextLayout *sTextLayout, ID2D1Brush *sBrush, D2D1_DRAW_TEXT_OPTIONS eOptions) {
	sRenderTarget->DrawTextLayout(sOrigin, sTextLayout, sBrush, eOptions);
}

HRESULT WINAPI D2DWr_RenderTarget_EndDraw(ID2D1RenderTarget *sRenderTarget, D2D1_TAG *uqwpTag1, D2D1_TAG *uqwpTag2) {
	return sRenderTarget->EndDraw(uqwpTag1, uqwpTag2);
}

void WINAPI D2DWr_RenderTarget_FillEllipse(ID2D1RenderTarget *sRenderTarget, D2D1_ELLIPSE const *sEllipse, ID2D1Brush *sBrush) {
	sRenderTarget->FillEllipse(sEllipse, sBrush);
}

void WINAPI D2DWr_RenderTarget_FillGeometry(ID2D1RenderTarget *sRenderTarget, ID2D1Geometry *sGeometry, ID2D1Brush *sBrush, ID2D1Brush *sOpacityBrush) {
	sRenderTarget->FillGeometry(sGeometry, sBrush, sOpacityBrush);
}

void WINAPI D2DWr_RenderTarget_FillMesh(ID2D1RenderTarget *sRenderTarget, ID2D1Mesh *sMesh, ID2D1Brush *sBrush) {
	sRenderTarget->FillMesh(sMesh, sBrush);
}

void WINAPI D2DWr_RenderTarget_FillOpacityMask(ID2D1RenderTarget *sRenderTarget, ID2D1Bitmap *sOpacityMask, ID2D1Brush *sBrush, D2D1_OPACITY_MASK_CONTENT eContent, D2D1_RECT_F const *sDestinationRect, D2D1_RECT_F const *sSourceRect) {
	sRenderTarget->FillOpacityMask(sOpacityMask, sBrush, eContent, sDestinationRect, sSourceRect);
}

void WINAPI D2DWr_RenderTarget_FillRectangle(ID2D1RenderTarget *sRenderTarget, D2D1_RECT_F const *sRectangle, ID2D1Brush *sBrush) {
	sRenderTarget->FillRectangle(sRectangle, sBrush);
}

void WINAPI D2DWr_RenderTarget_FillRoundedRectangle(ID2D1RenderTarget *sRenderTarget, D2D1_ROUNDED_RECT const *sRoundedRectangle, ID2D1Brush *sBrush) {
	sRenderTarget->FillRoundedRectangle(sRoundedRectangle, sBrush);
}

HRESULT WINAPI D2DWr_RenderTarget_Flush(ID2D1RenderTarget *sRenderTarget, D2D1_TAG *uqwpTag1, D2D1_TAG *uqwpTag2) {
	return sRenderTarget->Flush(uqwpTag1, uqwpTag2);
}

D2D1_ANTIALIAS_MODE WINAPI D2DWr_RenderTarget_GetAntialiasMode(ID2D1RenderTarget *sRenderTarget) {
	return sRenderTarget->GetAntialiasMode();
}

void WINAPI D2DWr_RenderTarget_GetDPI(ID2D1RenderTarget *sRenderTarget, FLOAT *fpDpiX, FLOAT *fpDpiY) {
	sRenderTarget->GetDpi(fpDpiX, fpDpiY);
}

UINT32 WINAPI D2DWr_RenderTarget_GetMaximumBitmapSize(ID2D1RenderTarget *sRenderTarget) {
	return sRenderTarget->GetMaximumBitmapSize();
}

D2D1_PIXEL_FORMAT WINAPI D2DWr_RenderTarget_GetPixelFormat(ID2D1RenderTarget *sRenderTarget) {
	return sRenderTarget->GetPixelFormat();
}

D2D1_SIZE_U WINAPI D2DWr_RenderTarget_GetPixelSize(ID2D1RenderTarget *sRenderTarget) {
	return sRenderTarget->GetPixelSize();
}

D2D1_SIZE_F WINAPI D2DWr_RenderTarget_GetSize(ID2D1RenderTarget *sRenderTarget) {
	return sRenderTarget->GetSize();
}

void WINAPI D2DWr_RenderTarget_GetTags(ID2D1RenderTarget *sRenderTarget, D2D1_TAG *uqwpTag1, D2D1_TAG *uqwpTag2) {
	sRenderTarget->GetTags(uqwpTag1, uqwpTag2);
}

D2D1_TEXT_ANTIALIAS_MODE WINAPI D2DWr_RenderTarget_GetTextAntialiasMode(ID2D1RenderTarget *sRenderTarget) {
	return sRenderTarget->GetTextAntialiasMode();
}

void WINAPI D2DWr_RenderTarget_GetTextRenderingParams(ID2D1RenderTarget *sRenderTarget, IDWriteRenderingParams **ptrpTextRenderingParams) {
	sRenderTarget->GetTextRenderingParams(ptrpTextRenderingParams);
}

void WINAPI D2DWr_RenderTarget_GetTransform(ID2D1RenderTarget *sRenderTarget, D2D1_MATRIX_3X2_F *sTransform) {
	sRenderTarget->GetTransform(sTransform);
}

BOOL WINAPI D2DWr_RenderTarget_IsSupported(ID2D1RenderTarget *sRenderTarget, D2D1_RENDER_TARGET_PROPERTIES const *sProps) {
	return sRenderTarget->IsSupported(sProps);
}

void WINAPI D2DWr_RenderTarget_PopAxisAlignedClip(ID2D1RenderTarget *sRenderTarget) {
	sRenderTarget->PopAxisAlignedClip();
}

void WINAPI D2DWr_RenderTarget_PopLayer(ID2D1RenderTarget *sRenderTarget) {
	sRenderTarget->PopLayer();
}

void WINAPI D2DWr_RenderTarget_PushAxisAlignedClip(ID2D1RenderTarget *sRenderTarget, D2D1_RECT_F const *sClipRect, D2D1_ANTIALIAS_MODE eAntialiasingMode) {
	sRenderTarget->PushAxisAlignedClip(sClipRect, eAntialiasingMode);
}

void WINAPI D2DWr_RenderTarget_PushLayer(ID2D1RenderTarget *sRenderTarget, D2D1_LAYER_PARAMETERS const *sLayerParams, ID2D1Layer *sLayer) {
	sRenderTarget->PushLayer(sLayerParams, sLayer);
}

void WINAPI D2DWr_RenderTarget_RestoreDrawingState(ID2D1RenderTarget *sRenderTarget, ID2D1DrawingStateBlock *sDrawingStateBlock) {
	sRenderTarget->RestoreDrawingState(sDrawingStateBlock);
}

void WINAPI D2DWr_RenderTarget_SaveDrawingState(ID2D1RenderTarget *sRenderTarget, ID2D1DrawingStateBlock *sDrawingStateBlock) {
	sRenderTarget->SaveDrawingState(sDrawingStateBlock);
}

void WINAPI D2DWr_RenderTarget_SetAntialiasMode(ID2D1RenderTarget *sRenderTarget, D2D1_ANTIALIAS_MODE eAntialiasingMode) {
	sRenderTarget->SetAntialiasMode(eAntialiasingMode);
}

void WINAPI D2DWr_RenderTarget_SetDPI(ID2D1RenderTarget *sRenderTarget, FLOAT flDpiX, FLOAT flDpiY) {
	sRenderTarget->SetDpi(flDpiX, flDpiY);
}

void WINAPI D2DWr_RenderTarget_SetTags(ID2D1RenderTarget *sRenderTarget, D2D1_TAG uqwTag1, D2D1_TAG uqwTag2) {
	sRenderTarget->SetTags(uqwTag1, uqwTag2);
} 

void WINAPI D2DWr_RenderTarget_SetTextAntialiasMode(ID2D1RenderTarget *sRenderTarget, D2D1_TEXT_ANTIALIAS_MODE eTextAntialiasingMode) {
	sRenderTarget->SetTextAntialiasMode(eTextAntialiasingMode);
}

void WINAPI D2DWr_RenderTarget_SetTextRenderingParams(ID2D1RenderTarget *sRenderTarget, IDWriteRenderingParams *sTextRenderingParams) {
	sRenderTarget->SetTextRenderingParams(sTextRenderingParams);
}

void WINAPI D2DWr_RenderTarget_SetTransform(ID2D1RenderTarget *sRenderTarget, D2D1_MATRIX_3X2_F *sTransform) {
	sRenderTarget->SetTransform(sTransform);
}

ULONG WINAPI D2DWr_RenderTarget_Release(ID2D1RenderTarget *sRenderTarget) {
	return sRenderTarget->Release();
}


HRESULT WINAPI D2DWr_DCRenderTarget_BindDC(ID2D1DCRenderTarget *sRenderTarget, HDC pHDC, RECT const *sRect) {
	return sRenderTarget->BindDC(pHDC, sRect);
}


