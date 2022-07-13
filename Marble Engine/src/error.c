#include <application.h>


static TCHAR const *const gl_straErrorCodeStrings[] = {
	TEXT("Marble_ErrorCode_Ok"),
	TEXT("Marble_ErrorCode_Unknown"),

	TEXT("Marble_ErrorCode_UnimplementedFeature"), 
	TEXT("Marble_ErrorCode_CreateDebugConsole"),
	TEXT("Marble_ErrorCode_MemoryAllocation"),
	TEXT("Marble_ErrorCode_MemoryReallocation"),
	TEXT("Marble_ErrorCode_Parameter"),
	TEXT("Marble_ErrorCode_InternalParameter"),
	TEXT("Marble_ErrorCode_OpenFile"),
	TEXT("Marble_ErrorCode_GetFileInfo"),
	TEXT("Marble_ErrorCode_ReadFromFile"),
	TEXT("Marble_ErrorCode_GotoFilePosition"),
	TEXT("Marble_ErrorCode_RegisterWindowClass"),
	TEXT("Marble_ErrorCode_COMInit"),
	TEXT("Marble_ErrorCode_CreateWindow"),
	TEXT("Marble_ErrorCode_InitHighPrecClock"),
	TEXT("Marble_ErrorCode_InitState"),
	TEXT("Marble_ErrorCode_ArraySubscript"),
	TEXT("Marble_ErrorCode_RendererAPI"),
	TEXT("Marble_ErrorCode_RendererInit"),
	TEXT("Marble_ErrorCode_CreateD2DFactory"),
	TEXT("Marble_ErrorCode_CreateD3D11Device"),
	TEXT("Marble_ErrorCode_GetDXGIDevice"),
	TEXT("Marble_ErrorCode_CreateD2DDevice"),
	TEXT("Marble_ErrorCode_CreateD2DDeviceContext"),
	TEXT("Marble_ErrorCode_GetDXGIAdapter"),
	TEXT("Marble_ErrorCode_GetDXGIFactory"),
	TEXT("Marble_ErrorCode_CreateDXGISwapchain"),
	TEXT("Marble_ErrorCode_GetDXGIBackbuffer"),
	TEXT("Marble_ErrorCode_CreateBitmapFromDxgiSurface"),
	TEXT("Marble_ErrorCode_AtlasType"),
	TEXT("Marble_ErrorCode_HeadValidation"),
	TEXT("Marble_ErrorCode_AssetType"),
	TEXT("Marble_ErrorCode_CreateWICImagingFactory"),
	TEXT("Marble_ErrorCode_CreateWICDecoder"),
	TEXT("Marble_ErrorCode_WICDecoderGetFrame"),
	TEXT("Marble_ErrorCode_WICCreateFormatConv"),
	TEXT("Marble_ErrorCode_WICFormatConversion"),
	TEXT("Marble_ErrorCode_CreateD2DBitmapFromIWICBitmap")
};
static size_t const gl_stNumOfErrorCodeStrings = sizeof(gl_straErrorCodeStrings) / sizeof(*gl_straErrorCodeStrings);

static TCHAR const *const gl_straErrorCodeDescs[] = {
	TEXT("Ok; no error occurred."),
	TEXT("Unknown error code. Check documentation for possible return values."),

	TEXT("Currently unimplemented feature. This will be changed in future releases."),
	TEXT("Failed to create debug console. This normally happens when there is no output stream in the system or the maximum number of concurrently open file handles has been reached."),
	TEXT("Failed to allocate memory. This happens when the system is out of available memory -- Close memory-intensive apps if possible and retry."),
	TEXT("Failed to change array size. This happens when the system is out of available memory -- Close memory-intensive apps if possible and retry."),
	TEXT("Invalid parameter has been passed (by the user) to a function. Check documentation for a detailed description on how to use this function."),
	TEXT("Invalid parameter has been internally passed. This is an issue that is to be fixed by the steward of this software.")
	/* TODO: add more descriptions */
};
static size_t const gl_stNumOfErrorCodeDescs = sizeof(gl_straErrorCodeDescs) / sizeof(*gl_straErrorCodeDescs);


TCHAR const *const Marble_Error_ToString(Marble_ErrorCode eErrorCode) {
	if (eErrorCode < Marble_ErrorCode_Ok || eErrorCode >= gl_stNumOfErrorCodeStrings)
		return Marble_Error_ToString(Marble_ErrorCode_UnimplementedFeature);

	return gl_straErrorCodeStrings[eErrorCode];
}

TCHAR const *const Marble_Error_ToDesc(Marble_ErrorCode eErrorCode) {
	if (eErrorCode < Marble_ErrorCode_Ok || eErrorCode >= gl_stNumOfErrorCodeDescs)
		return Marble_Error_ToDesc(Marble_ErrorCode_UnimplementedFeature);

	return gl_straErrorCodeDescs[eErrorCode];
}


