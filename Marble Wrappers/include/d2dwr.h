#pragma once

#include <wrbase.h>
#include <d2d1_1.h>


typedef D2D1_ANTIALIAS_MODE      D2D_AAMode;
typedef D2D1_PIXEL_FORMAT        D2D_PxFmt;
typedef D2D1_TEXT_ANTIALIAS_MODE D2D_TxtAAMode;


_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateDCRenderTarget(ID2D1Factory *sFactory, D2D1_RENDER_TARGET_PROPERTIES const *sProps, ID2D1DCRenderTarget **ptrpDCRenderTarget);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateDrawingStateBlock(ID2D1Factory *sFactory, D2D1_DRAWING_STATE_DESCRIPTION const *sDesc, IDWriteRenderingParams *sRenderingParams, ID2D1DrawingStateBlock **ptrpDrawingStateBlock);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateDxgiSurfaceRenderTarget(ID2D1Factory *sFactory, IDXGISurface *sDxgiSurface, D2D1_RENDER_TARGET_PROPERTIES const *sProps, ID2D1RenderTarget **ptrpDxgiRenderTarget);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateEllipseGeometry(ID2D1Factory *sFactory, D2D1_ELLIPSE *sEllipse, ID2D1EllipseGeometry **ptrpEllipseGeometry);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateGeometryGroup(ID2D1Factory *sFactory, D2D1_FILL_MODE eFillMode, ID2D1Geometry **spGeometries, UINT32 udwGeomCount, ID2D1GeometryGroup **ptrpGeometryGroup);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateHwndRenderTarget(ID2D1Factory *sFactory, D2D1_RENDER_TARGET_PROPERTIES const *sRTProps, D2D1_HWND_RENDER_TARGET_PROPERTIES const *sHWNDRTProps, ID2D1HwndRenderTarget **ptrpHwndRenderTarget);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreatePathGeometry(ID2D1Factory *sFactory, ID2D1PathGeometry **ptrpPathGeometry);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateRectangleGeometry(ID2D1Factory *sFactory, D2D1_RECT_F const *sRect, ID2D1RectangleGeometry **ptrpRectangleGeometry);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateRoundedRectangleGeometry(ID2D1Factory *sFactory, D2D1_ROUNDED_RECT const *sRoundedRect, ID2D1RoundedRectangleGeometry **ptrpRoundedRectangleGeometry);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateStrokeStyle(ID2D1Factory *sFactory, D2D1_STROKE_STYLE_PROPERTIES *const sStrokeStyleProps, FLOAT const *fpDashes, UINT32 udwDashesCount,ID2D1StrokeStyle **ptrpStrokeStyle);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateTransformedGeometry(ID2D1Factory *sFactory, ID2D1Geometry *sSourceGeometry, D2D1_MATRIX_3X2_F const *sTransform, ID2D1TransformedGeometry **ptrpTransformedGeometry);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_CreateWicBitmapRenderTarget(ID2D1Factory *sFactory, IWICBitmap *sTarget, D2D1_RENDER_TARGET_PROPERTIES const *sProps, ID2D1RenderTarget **ptrpWicBitmapRenderTarget);
_WR_EXTERN_C_ void    WINAPI D2DWr_Factory_GetDesktopDPI(ID2D1Factory *sFactory, FLOAT *fpDpiX, FLOAT *fpDpiY);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory_ReloadSystemMetrics(ID2D1Factory *sFactory);
_WR_EXTERN_C_ ULONG   WINAPI D2DWr_Factory_Release(ID2D1Factory *sFactory);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Factory1_CreateDevice(ID2D1Factory1 *sFactory, IDXGIDevice *sDXGIDevice, ID2D1Device **sD2DDevice);

_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_BeginDraw(ID2D1RenderTarget *sRenderTarget);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_Clear(ID2D1RenderTarget *sRenderTarget, D2D1_COLOR_F const *sClearColor);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateBitmap(ID2D1RenderTarget *sRenderTarget, D2D1_SIZE_U sSize, void const *ptrSrcData, UINT32 udwPitch, D2D1_BITMAP_PROPERTIES const *sProps, ID2D1Bitmap **ptrpBitmap);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateBitmapBrush(ID2D1RenderTarget *sRenderTarget, ID2D1Bitmap *sBitmap, D2D1_BITMAP_BRUSH_PROPERTIES const *sBitmapBrushProps, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1BitmapBrush **ptrpBitmapBrush);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateBitmapFromWicBitmap(ID2D1RenderTarget *sRenderTarget, IWICBitmapSource *sSourceBitmap, D2D1_BITMAP_PROPERTIES const *sBitmapProps, ID2D1Bitmap **ptrpBitmap);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateCompatibleRenderTarget(ID2D1RenderTarget *sRenderTarget, D2D1_SIZE_F const *sDesiredSize, D2D1_SIZE_U const *sDesiredPixelSize, D2D_PxFmt const *sDesiredPixelFormat, D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS eOptions, ID2D1BitmapRenderTarget **ptrpCompBitmapRenderTarget);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateGradientStopCollection(ID2D1RenderTarget *sRenderTarget, D2D1_GRADIENT_STOP const *sGradientStops, UINT32 udwGradientStopCount, D2D1_GAMMA eColorInterpolationGamma, D2D1_EXTEND_MODE eExtendMode, ID2D1GradientStopCollection **ptrpGradientStopCollection);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateLayer(ID2D1RenderTarget *sRenderTarget, D2D1_SIZE_F const *sSize, ID2D1Layer **ptrpLayer);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateMesh(ID2D1RenderTarget *sRenderTarget, ID2D1Mesh **ptrpMesh);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateLinearGradientBrush(ID2D1RenderTarget *sRenderTarget, D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES const *sLGBrushProps, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1GradientStopCollection *sGradientStopCollection, ID2D1LinearGradientBrush **ptrpLinearGradientBrush);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateRadialGradientBrush(ID2D1RenderTarget *sRenderTarget, D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES const *sRBrushProps, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1GradientStopCollection *sGradientStopCollection, ID2D1RadialGradientBrush **ptrpRadialGradientBrush);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateSharedBitmap(ID2D1RenderTarget *sRenderTarget, IID const *ptrIID, void *ptrData,D2D1_BITMAP_PROPERTIES const *sBitmapProps, ID2D1Bitmap **ptrpBitmap);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_CreateSolidColorBrush(ID2D1RenderTarget *sRenderTarget, D2D1_COLOR_F const *sColor, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1SolidColorBrush **ptrpSolidColorBrush);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_DrawBitmap(ID2D1RenderTarget *sRenderTarget, ID2D1Bitmap *sBitmap, D2D1_RECT_F const *sDestinationRect, FLOAT fOpacity, D2D1_BITMAP_INTERPOLATION_MODE eInterpolationMode, D2D1_RECT_F const *sSourceRect);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_DrawEllipse(ID2D1RenderTarget *sRenderTarget, D2D1_ELLIPSE const *sEllipse, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_DrawGeometry(ID2D1RenderTarget *sRenderTarget, ID2D1Geometry *sGeometry, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_DrawGlyphRun(ID2D1RenderTarget *sRenderTarget, D2D1_POINT_2F sBaselineOrigin, DWRITE_GLYPH_RUN const *sGlyphRun, ID2D1Brush *sBrush, DWRITE_MEASURING_MODE eMeasuringMode);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_DrawLine(ID2D1RenderTarget *sRenderTarget, D2D1_POINT_2F sPoint0, D2D1_POINT_2F sPoint1, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_DrawRectangle(ID2D1RenderTarget *sRenderTarget, D2D1_RECT_F const *sRectangle, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_DrawRoundedRectangle(ID2D1RenderTarget *sRenderTarget, D2D1_ROUNDED_RECT const *sRoundedRectangle, ID2D1Brush *sBrush, FLOAT fStrokeWidth, ID2D1StrokeStyle *sStrokeStyle);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_DrawText(ID2D1RenderTarget *sRenderTarget, WCHAR const *strText, UINT32 udwStringLength, IDWriteTextFormat *sTextFormat, D2D1_RECT_F const *sLayoutRect, ID2D1Brush *sBrush, D2D1_DRAW_TEXT_OPTIONS eOptions, DWRITE_MEASURING_MODE eMeasuringMode);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_DrawTextLayout(ID2D1RenderTarget *sRenderTarget, D2D1_POINT_2F sOrigin, IDWriteTextLayout *sTextLayout, ID2D1Brush *sBrush, D2D1_DRAW_TEXT_OPTIONS eOptions);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_EndDraw(ID2D1RenderTarget *sRenderTarget, D2D1_TAG *uqwpTag1, D2D1_TAG *uqwpTag2);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_FillEllipse(ID2D1RenderTarget *sRenderTarget, D2D1_ELLIPSE const *sEllipse, ID2D1Brush *sBrush);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_FillGeometry(ID2D1RenderTarget *sRenderTarget, ID2D1Geometry *sGeometry, ID2D1Brush *sBrush, ID2D1Brush *sOpacityBrush);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_FillMesh(ID2D1RenderTarget *sRenderTarget, ID2D1Mesh *sMesh, ID2D1Brush *sBrush);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_FillOpacityMask(ID2D1RenderTarget *sRenderTarget, ID2D1Bitmap *sOpacityMask, ID2D1Brush *sBrush, D2D1_OPACITY_MASK_CONTENT eContent, D2D1_RECT_F const *sDestinationRect, D2D1_RECT_F const *sSourceRect);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_FillRectangle(ID2D1RenderTarget *sRenderTarget, D2D1_RECT_F const *sRectangle, ID2D1Brush *sBrush);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_FillRoundedRectangle(ID2D1RenderTarget *sRenderTarget, D2D1_ROUNDED_RECT const *sRoundedRectangle, ID2D1Brush *sBrush);
_WR_EXTERN_C_ HRESULT       WINAPI D2DWr_RenderTarget_Flush(ID2D1RenderTarget *sRenderTarget, D2D1_TAG *uqwpTag1, D2D1_TAG *uqwpTag2);
_WR_EXTERN_C_ D2D_AAMode    WINAPI D2DWr_RenderTarget_GetAntialiasMode(ID2D1RenderTarget *sRenderTarget);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_GetDPI(ID2D1RenderTarget *sRenderTarget, FLOAT *fpDpiX, FLOAT *fpDpiY);
_WR_EXTERN_C_ UINT32        WINAPI D2DWr_RenderTarget_GetMaximumBitmapSize(ID2D1RenderTarget *sRenderTarget);
_WR_EXTERN_C_ D2D_PxFmt     WINAPI D2DWr_RenderTarget_GetPixelFormat(ID2D1RenderTarget *sRenderTarget);
_WR_EXTERN_C_ D2D1_SIZE_U   WINAPI D2DWr_RenderTarget_GetPixelSize(ID2D1RenderTarget *sRenderTarget);
_WR_EXTERN_C_ D2D1_SIZE_F   WINAPI D2DWr_RenderTarget_GetSize(ID2D1RenderTarget *sRenderTarget);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_GetTags(ID2D1RenderTarget *sRenderTarget, D2D1_TAG *uqwpTag1, D2D1_TAG *uqwpTag2);
_WR_EXTERN_C_ D2D_TxtAAMode WINAPI D2DWr_RenderTarget_GetTextAntialiasMode(ID2D1RenderTarget *sRenderTarget);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_GetTextRenderingParams(ID2D1RenderTarget *sRenderTarget, IDWriteRenderingParams **ptrpTextRenderingParams);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_GetTransform(ID2D1RenderTarget *sRenderTarget, D2D1_MATRIX_3X2_F *sTransform);
_WR_EXTERN_C_ BOOL          WINAPI D2DWr_RenderTarget_IsSupported(ID2D1RenderTarget *sRenderTarget, D2D1_RENDER_TARGET_PROPERTIES const *sProps);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_PopAxisAlignedClip(ID2D1RenderTarget *sRenderTarget);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_PopLayer(ID2D1RenderTarget *sRenderTarget);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_PushAxisAlignedClip(ID2D1RenderTarget *sRenderTarget, D2D1_RECT_F const *sClipRect, D2D_AAMode eAntialiasingMode);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_PushLayer(ID2D1RenderTarget *sRenderTarget, D2D1_LAYER_PARAMETERS const *sLayerParams, ID2D1Layer *sLayer);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_RestoreDrawingState(ID2D1RenderTarget *sRenderTarget, ID2D1DrawingStateBlock *sDrawingStateBlock);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_SaveDrawingState(ID2D1RenderTarget *sRenderTarget, ID2D1DrawingStateBlock *sDrawingStateBlock);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_SetAntialiasMode(ID2D1RenderTarget *sRenderTarget, D2D_AAMode eAntialiasingMode);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_SetDPI(ID2D1RenderTarget *sRenderTarget, FLOAT flDpiX, FLOAT flDpiY);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_SetTags(ID2D1RenderTarget *sRenderTarget, D2D1_TAG uqwTag1, D2D1_TAG uqwTag2);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_SetTextAntialiasMode(ID2D1RenderTarget *sRenderTarget, D2D_TxtAAMode eTextAntialiasingMode);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_SetTextRenderingParams(ID2D1RenderTarget *sRenderTarget, IDWriteRenderingParams *sTextRenderingParams);
_WR_EXTERN_C_ void          WINAPI D2DWr_RenderTarget_SetTransform(ID2D1RenderTarget *sRenderTarget, D2D1_MATRIX_3X2_F *sTransform);

_WR_EXTERN_C_ HRESULT WINAPI D2DWr_Device_CreateDeviceContext(ID2D1Device *sDevice, D2D1_DEVICE_CONTEXT_OPTIONS eOptions, ID2D1DeviceContext **ptrpDeviceContext);
_WR_EXTERN_C_ ULONG   WINAPI D2DWr_Device_Release(ID2D1Device *sDevice);

_WR_EXTERN_C_ HRESULT WINAPI D2DWr_DeviceContext_CreateBitmapFromDxgiSurface(ID2D1DeviceContext *sDeviceContext, IDXGISurface *sDXGISurface, D2D1_BITMAP_PROPERTIES1 const *sBitmapProps, ID2D1Bitmap1 **ptrpBitmap);
_WR_EXTERN_C_ void    WINAPI D2DWr_DeviceContext_SetTarget(ID2D1DeviceContext *sDeviceContext, ID2D1Image *sImage);
_WR_EXTERN_C_ void    WINAPI D2DWr_DeviceContext_Clear(ID2D1DeviceContext *sDeviceContext, D2D1_COLOR_F const *sClearColor);
_WR_EXTERN_C_ void    WINAPI D2DWr_DeviceContext_BeginDraw(ID2D1DeviceContext *sDeviceContext);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_DeviceContext_EndDraw(ID2D1DeviceContext *sDeviceContext, D2D1_TAG *uqwpTag1, D2D1_TAG *uqwpTag2);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_DeviceContext_CreateSolidColorBrush(ID2D1DeviceContext *sDeviceContext, D2D1_COLOR_F const *sColor, D2D1_BRUSH_PROPERTIES const *sBrushProps, ID2D1SolidColorBrush **ptrpBrush);
_WR_EXTERN_C_ HRESULT WINAPI D2DWr_DeviceContext_CreateBitmapFromWicBitmap(ID2D1DeviceContext *sDeviceContext, IWICBitmapSource *sBitmapSource, D2D1_BITMAP_PROPERTIES1 const *sBitmapProps, ID2D1Bitmap1 **ptrpBitmap);
_WR_EXTERN_C_ void    WINAPI D2DWr_DeviceContext_DrawBitmap(ID2D1DeviceContext *sDeviceContext, ID2D1Bitmap *sBitmap, D2D1_RECT_F const *sDstRect, FLOAT fOpacity, D2D1_INTERPOLATION_MODE eInterpolationMode, D2D1_RECT_F const *sSrcRect, D2D1_MATRIX_4X4_F const *sPerspectiveTransform);
_WR_EXTERN_C_ void    WINAPI D2DWr_DeviceContext_DrawRectangle(ID2D1DeviceContext *sDeviceContext, D2D1_RECT_F const *sRect, ID2D1Brush *sBrush, FLOAT fStrokeWith, ID2D1StrokeStyle *sStrokeStyle);
_WR_EXTERN_C_ void    WINAPI D2DWr_DeviceContext_FillRectangle(ID2D1DeviceContext *sDeviceContext, D2D1_RECT_F const *sRect, ID2D1Brush *sBrush);
_WR_EXTERN_C_ void    WINAPI D2DWr_DeviceContext_DrawText(ID2D1DeviceContext *sDeviceContext, WCHAR const *wstrString, UINT32 ui32Length, IDWriteTextFormat *sFormat, D2D1_RECT_F const *sLayoutRect, ID2D1Brush *sFillBrush, D2D1_DRAW_TEXT_OPTIONS eOptions, DWRITE_MEASURING_MODE eMeasuringMode);
_WR_EXTERN_C_ ULONG   WINAPI D2DWr_DeviceContext_Release(ID2D1DeviceContext *sDeviceContext);

_WR_EXTERN_C_ ULONG WINAPI D2DWr_Bitmap_Release(ID2D1Bitmap *sBitmap);

_WR_EXTERN_C_ ULONG WINAPI D2DWr_SolidColorBrush_Release(ID2D1SolidColorBrush *sBrush);


