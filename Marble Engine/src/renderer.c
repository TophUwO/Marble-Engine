#pragma once

#include <application.h>


static void Marble_Renderer_Internal_GetDesktopDPI(float *fpDpiX, float *fpDpiY) {
	UINT udwDpi = GetDpiForWindow(GetDesktopWindow());

	*fpDpiX = *fpDpiY = (float)udwDpi;
}


#pragma region Direct2D Renderer
static int Marble_Renderer_Internal_Direct2DCleanup(int iErrorCode) {
#define Marble_Direct2DRenderer_SafeRelease(fn, inst)  if (inst) fn(inst)
#define Marble_Direct2DRenderer_SafeRelease_Vtbl(inst) if (inst) (inst)->lpVtbl->Release(inst)

	if (iErrorCode) {
		Marble_Direct2DRenderer_SafeRelease(D2DWr_Factory_Release, (ID2D1Factory *)gl_sApplication.sRenderer->sD2DRenderer.sD2DFactory);
		Marble_Direct2DRenderer_SafeRelease(D2DWr_DeviceContext_Release, gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext);
		Marble_Direct2DRenderer_SafeRelease(D2DWr_Device_Release, gl_sApplication.sRenderer->sD2DRenderer.sD2DDevice);
		Marble_Direct2DRenderer_SafeRelease(D2DWr_Bitmap_Release, (ID2D1Bitmap *)gl_sApplication.sRenderer->sD2DRenderer.sBitmap);
		Marble_Direct2DRenderer_SafeRelease_Vtbl(gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain);
		Marble_Direct2DRenderer_SafeRelease_Vtbl(gl_sApplication.sRenderer->sD2DRenderer.sD3D11Device);
		Marble_Direct2DRenderer_SafeRelease_Vtbl(gl_sApplication.sRenderer->sD2DRenderer.sDXGIDevice);
		Marble_Direct2DRenderer_SafeRelease_Vtbl(gl_sApplication.sRenderer->sD2DRenderer.sDXGIAdapter);
		Marble_Direct2DRenderer_SafeRelease_Vtbl(gl_sApplication.sRenderer->sD2DRenderer.sDXGIFactory);
		Marble_Direct2DRenderer_SafeRelease_Vtbl(gl_sApplication.sRenderer->sD2DRenderer.sDXGIBackbuffer);
	}

	return iErrorCode;
}

static int Marble_Renderer_Internal_InitializeDirect2DRenderer(HWND hwRenderWindow) {
#define Marble_Direct2DRenderer_Action(act, ret) if (act != S_OK) { iErrorCode = ret; goto ON_ERROR; }
#pragma region Static Init Data
	static D2D1_FACTORY_OPTIONS const sD2DFactoryOptions = { .debugLevel = D2D1_DEBUG_LEVEL_INFORMATION };
	static D3D_FEATURE_LEVEL const eaD3DFeatureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	DXGI_SWAP_CHAIN_DESC1 sDXGISwapChainDesc = {
		.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Width = 0,
		.Height = 0,
		.Flags = 0,
		.BufferCount = 2,
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
		.Scaling = DXGI_SCALING_STRETCH,
		.Stereo = FALSE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		.SampleDesc = {
			.Count   = 1,
			.Quality = 0
		}
	};
	D2D1_BITMAP_PROPERTIES1 sD2DBitmapProps = {
		.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		.pixelFormat   = {
			.format    = DXGI_FORMAT_B8G8R8A8_UNORM,
			.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED
		}
	};
	Marble_Renderer_Internal_GetDesktopDPI(&sD2DBitmapProps.dpiX, &sD2DBitmapProps.dpiY);
#pragma endregion

	int iErrorCode = Marble_ErrorCode_Ok;
	if (!(gl_sApplication.sRenderer = calloc(1, sizeof(*gl_sApplication.sRenderer))))
		return Marble_ErrorCode_MemoryAllocation;
	
	Marble_Direct2DRenderer_Action(
		D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, 
			&IID_ID2D1Factory1, 
			&sD2DFactoryOptions, 
			&gl_sApplication.sRenderer->sD2DRenderer.sD2DFactory
		), 
		Marble_ErrorCode_CreateD2DFactory
	);

	Marble_Direct2DRenderer_Action(
		D3D11CreateDevice(NULL, 
			D3D_DRIVER_TYPE_HARDWARE, 
			0, 
			D3D11_CREATE_DEVICE_BGRA_SUPPORT, 
			eaD3DFeatureLevels, 
			ARRAYSIZE(eaD3DFeatureLevels), 
			D3D11_SDK_VERSION, 
			&gl_sApplication.sRenderer->sD2DRenderer.sD3D11Device, 
			NULL, 
			NULL
		), 
		Marble_ErrorCode_CreateD3D11Device
	);

	Marble_Direct2DRenderer_Action(
		gl_sApplication.sRenderer->sD2DRenderer.sD3D11Device->lpVtbl->QueryInterface(
			gl_sApplication.sRenderer->sD2DRenderer.sD3D11Device, 
			&IID_IDXGIDevice, 
			&gl_sApplication.sRenderer->sD2DRenderer.sDXGIDevice
		), 
		Marble_ErrorCode_GetDXGIDevice
	);

	Marble_Direct2DRenderer_Action(
		D2DWr_Factory1_CreateDevice(
			gl_sApplication.sRenderer->sD2DRenderer.sD2DFactory, 
			gl_sApplication.sRenderer->sD2DRenderer.sDXGIDevice, 
			&gl_sApplication.sRenderer->sD2DRenderer.sD2DDevice
		), 
		Marble_ErrorCode_CreateD2DDevice
	);

	Marble_Direct2DRenderer_Action(
		D2DWr_Device_CreateDeviceContext(
			gl_sApplication.sRenderer->sD2DRenderer.sD2DDevice, 
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext
		), 
		Marble_ErrorCode_CreateD2DDeviceContext
	);

	Marble_Direct2DRenderer_Action(
		gl_sApplication.sRenderer->sD2DRenderer.sDXGIDevice->lpVtbl->GetAdapter(
			gl_sApplication.sRenderer->sD2DRenderer.sDXGIDevice, 
			&gl_sApplication.sRenderer->sD2DRenderer.sDXGIAdapter
		), 
		Marble_ErrorCode_GetDXGIAdapter
	);

	Marble_Direct2DRenderer_Action(
		gl_sApplication.sRenderer->sD2DRenderer.sDXGIAdapter->lpVtbl->GetParent(
			gl_sApplication.sRenderer->sD2DRenderer.sDXGIAdapter, 
			&IID_IDXGIFactory2, 
			&gl_sApplication.sRenderer->sD2DRenderer.sDXGIFactory
		), 
		Marble_ErrorCode_GetDXGIFactory
	);
	
	Marble_Direct2DRenderer_Action(
		gl_sApplication.sRenderer->sD2DRenderer.sDXGIFactory->lpVtbl->CreateSwapChainForHwnd(
			gl_sApplication.sRenderer->sD2DRenderer.sDXGIFactory, 
			(IUnknown *)gl_sApplication.sRenderer->sD2DRenderer.sD3D11Device, 
			hwRenderWindow, 
			&sDXGISwapChainDesc,
			NULL, 
			NULL, 
			&gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain
		), 
		Marble_ErrorCode_CreateDXGISwapchain
	);
	
	Marble_Direct2DRenderer_Action(
		gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain->lpVtbl->GetBuffer(
			gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain, 
			0,
			&IID_IDXGISurface, 
			&gl_sApplication.sRenderer->sD2DRenderer.sDXGIBackbuffer
		), 
		Marble_ErrorCode_GetDXGIBackbuffer
	);
	
	Marble_Direct2DRenderer_Action(
		D2DWr_DeviceContext_CreateBitmapFromDxgiSurface(
			gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext, 
			gl_sApplication.sRenderer->sD2DRenderer.sDXGIBackbuffer, 
			&sD2DBitmapProps, 
			&gl_sApplication.sRenderer->sD2DRenderer.sBitmap
		), 
		Marble_ErrorCode_CreateBitmapFromDxgiSurface
	);
	D2DWr_DeviceContext_SetTarget(
		gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext, 
		(ID2D1Image *)gl_sApplication.sRenderer->sD2DRenderer.sBitmap
	);

	gl_sApplication.sRenderer->dwActiveRendererAPI = Marble_RendererAPI_Direct2D;

ON_ERROR:
	return Marble_Renderer_Internal_Direct2DCleanup(iErrorCode);
}

static void inline Marble_Renderer_Internal_UninitializeDirect2DRenderer(void) {
	Marble_Renderer_Internal_Direct2DCleanup(Marble_ErrorCode_Unknown);
}

static void inline Marble_Renderer_Internal_Direct2DRenderer_Clear(float fRed, float fGreen, float fBlue, float fAlpha) {
	D2D1_COLOR_F const sColor = {
		.r = fRed,
		.g = fGreen,
		.b = fBlue,
		.a = fAlpha
	};

	D2DWr_DeviceContext_Clear(gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext, &sColor);
}

static void inline Marble_Renderer_Internal_Direct2DRenderer_BeginDraw(void) {
	D2DWr_DeviceContext_BeginDraw(gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext);
}

static void inline Marble_Renderer_Internal_Direct2DRenderer_EndDraw(void) {
	if (D2DWr_DeviceContext_EndDraw(gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext, NULL, NULL) == D2DERR_RECREATE_TARGET) {
		// TODO: Recreate all needed resources
	}
}
#pragma endregion


int Marble_Renderer_Initialize(DWORD dwActiveAPI, HWND hwRenderWindow) {
	if (!gl_sApplication.sRenderer) {
		switch (dwActiveAPI) {
			case Marble_RendererAPI_Direct2D: return Marble_Renderer_Internal_InitializeDirect2DRenderer(hwRenderWindow);
		}

		return Marble_ErrorCode_RendererAPI;
	}

	return Marble_ErrorCode_RendererInit;
}

void Marble_Renderer_Uninitialize(void) {
	if (gl_sApplication.sRenderer) {
		switch (gl_sApplication.sRenderer->dwActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_UninitializeDirect2DRenderer();
		}

		free(gl_sApplication.sRenderer);
	}
}

void Marble_Renderer_BeginDraw(void) {
	if (gl_sApplication.sRenderer) {
		switch (gl_sApplication.sRenderer->dwActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_Direct2DRenderer_BeginDraw();
		}
	}
}

void Marble_Renderer_EndDraw(void) {
	if (gl_sApplication.sRenderer) {
		switch (gl_sApplication.sRenderer->dwActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_Direct2DRenderer_EndDraw();
		}
	}
}

int Marble_Renderer_Present(void) {
	if (gl_sApplication.sRenderer) {
		switch (gl_sApplication.sRenderer->dwActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: 
				return (int)gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain->lpVtbl->Present(
					gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain, 
					(UINT)gl_sApplication.sMainWindow->sWndData.blIsVSync, 
					0
				);
		}

		return Marble_ErrorCode_RendererAPI;
	}

	return Marble_ErrorCode_RendererInit;
}

void Marble_Renderer_Clear(float fRed, float fGreen, float fBlue, float fAlpha) {
	if (gl_sApplication.sRenderer) {
		switch (gl_sApplication.sRenderer->dwActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_Direct2DRenderer_Clear(fRed, fGreen, fBlue, fAlpha);
		}
	}
}


