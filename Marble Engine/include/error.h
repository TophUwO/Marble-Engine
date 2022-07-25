#pragma once

#include <api.h>


#define MB_IFNTRUE_RET_CODE(expr, code)      { if (!(expr)) { return (int)code; } }
#define MB_IFNOK_RET_CODE(expr)              { if (iErrorCode = (int)(expr)) { return iErrorCode; } }
#define MB_IFNOK_GOTO_LBL(expr, label)       { if (iErrorCode = (int)(expr)) { goto label; } }
#define MB_IFNOK_DO_BODY(expr, body)         { if (iErrorCode = (int)(expr)) { body; } }


typedef enum Marble_ErrorCodes {
	Marble_ErrorCode_Ok = 0,
	Marble_ErrorCode_Unknown,

	Marble_ErrorCode_UnimplementedFeature, 
	Marble_ErrorCode_CreateDebugConsole,
	Marble_ErrorCode_MemoryAllocation,
	Marble_ErrorCode_MemoryReallocation,
	Marble_ErrorCode_Parameter,
	Marble_ErrorCode_InternalParameter,
	Marble_ErrorCode_OpenFile,
	Marble_ErrorCode_GetFileInfo,
	Marble_ErrorCode_ReadFromFile,
	Marble_ErrorCode_GotoFilePosition,
	Marble_ErrorCode_RegisterWindowClass,
	Marble_ErrorCode_COMInit,
	Marble_ErrorCode_CreateWindow,
	Marble_ErrorCode_InitHighPrecClock,
	Marble_ErrorCode_AppState,
	Marble_ErrorCode_ComponentInitState,
	Marble_ErrorCode_ArraySubscript,
	Marble_ErrorCode_RendererAPI,
	Marble_ErrorCode_RendererInit,
	Marble_ErrorCode_CreateD2DFactory,
	Marble_ErrorCode_CreateD3D11Device,
	Marble_ErrorCode_GetDXGIDevice,
	Marble_ErrorCode_CreateD2DDevice,
	Marble_ErrorCode_CreateD2DDeviceContext,
	Marble_ErrorCode_GetDXGIAdapter,
	Marble_ErrorCode_GetDXGIFactory,
	Marble_ErrorCode_CreateDXGISwapchain,
	Marble_ErrorCode_GetDXGIBackbuffer,
	Marble_ErrorCode_CreateBitmapFromDxgiSurface,
	Marble_ErrorCode_ResizeRendererBuffers,
	Marble_ErrorCode_HeadValidation,
	Marble_ErrorCode_AssetType,
	Marble_ErrorCode_AssetID,
	Marble_ErrorCode_CreateWICImagingFactory,
	Marble_ErrorCode_CreateWICDecoder,
	Marble_ErrorCode_WICDecoderGetFrame,
	Marble_ErrorCode_WICCreateFormatConv,
	Marble_ErrorCode_WICFormatConversion,
	Marble_ErrorCode_CreateD2DBitmapFromIWICBitmap,
	Marble_ErrorCode_CreateDWriteFactory,
	Marble_ErrorCode_CreateTextFormat,
	Marble_ErrorCode_ElementNotFound,
	Marble_ErrorCode_DuplicatesNotAllowed
} Marble_ErrorCode;


MARBLE_API TCHAR const *const Marble_Error_ToString(Marble_ErrorCode eErrorCode);
MARBLE_API TCHAR const *const Marble_Error_ToDesc(Marble_ErrorCode eErrorCode);


