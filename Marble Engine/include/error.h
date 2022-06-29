#pragma once


typedef enum Marble_ErrorCodes {
	Marble_ErrorCode_Ok = 0,
	Marble_ErrorCode_Unknown,

	Marble_ErrorCode_CreateDebugConsole,
	Marble_ErrorCode_MemoryAllocation,
	Marble_ErrorCode_MemoryReallocation,
	Marble_ErrorCode_RegisterWindowClass,
	Marble_ErrorCode_CreateWindow,
	Marble_ErrorCode_CreateLayer,
	Marble_ErrorCode_InitHighPrecClock,
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
	Marble_ErrorCode_COMRelease
} Marble_ErrorCode;


