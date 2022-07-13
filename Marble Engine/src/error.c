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
	TEXT("Marble_ErrorCode_CreateD2DBitmapFromIWICBitmap"),
	TEXT("Marble_ErrorCode_CreateDWriteFactory"),
	TEXT("Marble_ErrorCode_CreateTextFormat")
};
static size_t const gl_stNumOfErrorCodeStrings = sizeof(gl_straErrorCodeStrings) / sizeof(*gl_straErrorCodeStrings);

static TCHAR const *const gl_straErrorCodeDescs[] = {
	TEXT("Ok; no error occurred."),
	TEXT("Unknown error code. Check the documentation for possible return values."),

	TEXT("Currently unimplemented feature. This will be changed in future releases."),
	TEXT("Failed to create debug console. This normally happens when there is no output stream in the system or the maximum number of concurrently open file handles has been reached."),
	TEXT("Failed to allocate memory. This happens when the system is out of available memory - Close memory-intensive apps if possible and retry."),
	TEXT("Failed to change array size. This happens when the system is out of available memory - Close memory-intensive apps if possible and retry."),
	TEXT("Invalid parameter has been passed (by the user) to a function. Check the documentation for a detailed description on how to use this function."),
	TEXT("Invalid parameter has been internally passed. This is an issue that is to be fixed by the steward of this software."),
	TEXT("Could not open file. Happens when a file or directory does not exist or if there are insufficient permissions."),
	TEXT("Failed to obtain file information. Occurs when the passed file handle is or has become invalid."),
	TEXT("Could not read from file. Causes can be software (handle invalid?)-related or can be caused by physical device loss."),
	TEXT("Could not jump to file position. Happens when the handle and/or the position is invalid (e.g. too large)."),
	TEXT("Could not register window class. Check WNDCLASSEX structure for incorrect values."),
	TEXT("Failed to initialize the component object model (COM). This error normally occurs when the system is out of usable memory or invalid parameters have passed to the function."),
	TEXT("Failed to create window. Check CreateWindow(Ex)() for incorrect values."),
	TEXT("Could not find a high-precision clock. Please buy yourself an actual PC."),
	TEXT("Operation could not be carried out due to an incorrect application state."),
	TEXT("Array subscript is out of range. Check function parameters."),
	TEXT("Invalid render API identifier. Contact the steward of the software."),
	TEXT("Renderer is not available (e.g. failed to initialize)."),
	TEXT("Could not create Direct2D factory. System may be out of memory; please contact the steward of the software."),
	TEXT("Failed to create Direct3D (11) device. Your video card (or its driver) may be too old."),
	TEXT("Failed to create DXGI device. System may be out of memory; please contact the steward of the software."),
	TEXT("Failed to create Direct2D device. System may be out of memory; please contact the steward of the software."),
	TEXT("Failed to create Direct2D device context. System may be out of memory; please contact the steward of the software."),
	TEXT("Failed to obtain DXGI adapter. System may be out of memory; please contact the steward of the software."),
	TEXT("Failed to create DXGI factory. System may be out of memory; please contact the steward of the software."),
	TEXT("Failed to create DXGI swap-chain. An object needed to create the swap-chain has failed to create. System may be out of memory; please contact the steward of the software."),
	TEXT("Failed to get DXGI buffer. System may be out of memory; please contact the steward of the software."),
	TEXT("Could not create Direct2D bitmap from DXGI surface. Check for pixel format or other property mismatches."),
	TEXT("Invalid atlas type. Check the documentation for valid atlas type identifiers."),
	TEXT("Failed to validate file head. Check file handle and function parameters. Invalid file format might have been used."),
	TEXT("Invalid asset type. Check the documentation for a list of valid asset types."),
	TEXT("Could not create WIC (windows imaging component) factory. System may be out of memory; please contact the steward of the software."),
	TEXT("Could not create WIC (windows imaging component) decoder. Check parameters. System may be out of memory; please contact the steward of the software."),
	TEXT("Could not decode frame. Check parameters and system memory consumption."),
	TEXT("Could not create WIC (windows imaging component) pixel format converter. Check parameters and memory usage."),
	TEXT("Could not convert pixel format. This is usually due to parameter inconsistencies or invalid file formats."),
	TEXT("Could not create Direct2D bitmap from WIC (windows imaging component) bitmap. There are perhaps format inconsistencies between the two bitmaps."),
	TEXT("Failed to create DirectWrite factory. Check parameters and memory consumption."),
	TEXT("Failed to create text format. Check parameters. System may be out of usable memory.")
	/* TODO: add more descriptions */
};
static size_t const gl_stNumOfErrorCodeDescs = sizeof(gl_straErrorCodeDescs) / sizeof(*gl_straErrorCodeDescs);


TCHAR const *const Marble_Error_ToString(Marble_ErrorCode eErrorCode) {
	if (eErrorCode < Marble_ErrorCode_Ok || eErrorCode >= (int)gl_stNumOfErrorCodeStrings)
		return Marble_Error_ToString(Marble_ErrorCode_UnimplementedFeature);

	return gl_straErrorCodeStrings[eErrorCode];
}

TCHAR const *const Marble_Error_ToDesc(Marble_ErrorCode eErrorCode) {
	if (eErrorCode < Marble_ErrorCode_Ok || eErrorCode >= (int)gl_stNumOfErrorCodeDescs)
		return Marble_Error_ToDesc(Marble_ErrorCode_UnimplementedFeature);

	return gl_straErrorCodeDescs[eErrorCode];
}


