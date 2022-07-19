#pragma once

#include <api.h>


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
	Marble_ErrorCode_InitState,
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
	Marble_ErrorCode_AtlasType,
	Marble_ErrorCode_HeadValidation,
	Marble_ErrorCode_AssetType,
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


