#pragma once

#include <application.h>


static void Marble_Renderer_Internal_GetDesktopDPI(float *fpDpiX, float *fpDpiY) {
	UINT udwDpi = GetDpiForWindow(GetDesktopWindow());

	*fpDpiX = *fpDpiY = (float)udwDpi;
}


#pragma region Direct2D Renderer
static int Marble_Renderer_Internal_Direct2DCleanup(Marble_Renderer **ptrpRenderer, int iErrorCode, HWND *hwpRestoredHWND) {
#define Marble_Direct2DRenderer_SafeRelease(fn, inst)  if (inst) fn(inst)
#define Marble_Direct2DRenderer_SafeRelease_Vtbl(inst) if (inst) (inst)->lpVtbl->Release(inst)

	if (iErrorCode) {
		if (hwpRestoredHWND && (*ptrpRenderer)->sD2DRenderer.sDXGISwapchain)
			(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain->lpVtbl->GetHwnd(
				(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain,
				hwpRestoredHWND
			);

		Marble_Direct2DRenderer_SafeRelease(D2DWr_Factory_Release, (ID2D1Factory *)(*ptrpRenderer)->sD2DRenderer.sD2DFactory);
		Marble_Direct2DRenderer_SafeRelease(D2DWr_DeviceContext_Release, (*ptrpRenderer)->sD2DRenderer.sD2DDevContext);
		Marble_Direct2DRenderer_SafeRelease(D2DWr_Device_Release, (*ptrpRenderer)->sD2DRenderer.sD2DDevice);
		Marble_Direct2DRenderer_SafeRelease(D2DWr_Bitmap_Release, (ID2D1Bitmap *)(*ptrpRenderer)->sD2DRenderer.sBitmap);
		Marble_Direct2DRenderer_SafeRelease_Vtbl((*ptrpRenderer)->sD2DRenderer.sDXGISwapchain);
		Marble_Direct2DRenderer_SafeRelease_Vtbl((*ptrpRenderer)->sD2DRenderer.sD3D11Device);
		Marble_Direct2DRenderer_SafeRelease_Vtbl((*ptrpRenderer)->sD2DRenderer.sDXGIDevice);
		Marble_Direct2DRenderer_SafeRelease_Vtbl((*ptrpRenderer)->sD2DRenderer.sDXGIAdapter);
		Marble_Direct2DRenderer_SafeRelease_Vtbl((*ptrpRenderer)->sD2DRenderer.sDXGIFactory);
		Marble_Direct2DRenderer_SafeRelease_Vtbl((*ptrpRenderer)->sD2DRenderer.sDXGIBackbuffer);
	}

	return iErrorCode;
}

static int Marble_Renderer_Internal_Direct2DRenderer_Create(Marble_Renderer **ptrpRenderer, HWND hwRenderWindow) {
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
		.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Width       = 0,
		.Height      = 0,
		.Flags       = 0,
		.BufferCount = 2,
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.Format      = DXGI_FORMAT_B8G8R8A8_UNORM,
		.Scaling     = DXGI_SCALING_STRETCH,
		.Stereo      = FALSE,
		.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		.SampleDesc  = {
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
	
	Marble_Direct2DRenderer_Action(
		D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, 
			&IID_ID2D1Factory1, 
			&sD2DFactoryOptions, 
			&(*ptrpRenderer)->sD2DRenderer.sD2DFactory
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
			&(*ptrpRenderer)->sD2DRenderer.sD3D11Device, 
			NULL, 
			NULL
		), 
		Marble_ErrorCode_CreateD3D11Device
	);

	Marble_Direct2DRenderer_Action(
		(*ptrpRenderer)->sD2DRenderer.sD3D11Device->lpVtbl->QueryInterface(
			(*ptrpRenderer)->sD2DRenderer.sD3D11Device, 
			&IID_IDXGIDevice, 
			&(*ptrpRenderer)->sD2DRenderer.sDXGIDevice
		), 
		Marble_ErrorCode_GetDXGIDevice
	);

	Marble_Direct2DRenderer_Action(
		D2DWr_Factory1_CreateDevice(
			(*ptrpRenderer)->sD2DRenderer.sD2DFactory, 
			(*ptrpRenderer)->sD2DRenderer.sDXGIDevice, 
			&(*ptrpRenderer)->sD2DRenderer.sD2DDevice
		), 
		Marble_ErrorCode_CreateD2DDevice
	);

	Marble_Direct2DRenderer_Action(
		D2DWr_Device_CreateDeviceContext(
			(*ptrpRenderer)->sD2DRenderer.sD2DDevice, 
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&(*ptrpRenderer)->sD2DRenderer.sD2DDevContext
		), 
		Marble_ErrorCode_CreateD2DDeviceContext
	);

	Marble_Direct2DRenderer_Action(
		(*ptrpRenderer)->sD2DRenderer.sDXGIDevice->lpVtbl->GetAdapter(
			(*ptrpRenderer)->sD2DRenderer.sDXGIDevice, 
			&(*ptrpRenderer)->sD2DRenderer.sDXGIAdapter
		), 
		Marble_ErrorCode_GetDXGIAdapter
	);

	Marble_Direct2DRenderer_Action(
		(*ptrpRenderer)->sD2DRenderer.sDXGIAdapter->lpVtbl->GetParent(
			(*ptrpRenderer)->sD2DRenderer.sDXGIAdapter, 
			&IID_IDXGIFactory2, 
			&(*ptrpRenderer)->sD2DRenderer.sDXGIFactory
		), 
		Marble_ErrorCode_GetDXGIFactory
	);
	
	Marble_Direct2DRenderer_Action(
		(*ptrpRenderer)->sD2DRenderer.sDXGIFactory->lpVtbl->CreateSwapChainForHwnd(
			(*ptrpRenderer)->sD2DRenderer.sDXGIFactory, 
			(IUnknown *)(*ptrpRenderer)->sD2DRenderer.sD3D11Device, 
			hwRenderWindow, 
			&sDXGISwapChainDesc,
			NULL, 
			NULL, 
			&(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain
		), 
		Marble_ErrorCode_CreateDXGISwapchain
	);
	
	Marble_Direct2DRenderer_Action(
		(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain->lpVtbl->GetBuffer(
			(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain, 
			0,
			&IID_IDXGISurface, 
			&(*ptrpRenderer)->sD2DRenderer.sDXGIBackbuffer
		), 
		Marble_ErrorCode_GetDXGIBackbuffer
	);
	
	Marble_Direct2DRenderer_Action(
		D2DWr_DeviceContext_CreateBitmapFromDxgiSurface(
			(*ptrpRenderer)->sD2DRenderer.sD2DDevContext, 
			(*ptrpRenderer)->sD2DRenderer.sDXGIBackbuffer, 
			&sD2DBitmapProps, 
			&(*ptrpRenderer)->sD2DRenderer.sBitmap
		), 
		Marble_ErrorCode_CreateBitmapFromDxgiSurface
	);
	D2DWr_DeviceContext_SetTarget(
		(*ptrpRenderer)->sD2DRenderer.sD2DDevContext, 
		(ID2D1Image *)(*ptrpRenderer)->sD2DRenderer.sBitmap
	);

	DXGI_RGBA sBkgndColor = {
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};
	(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain->lpVtbl->SetBackgroundColor((*ptrpRenderer)->sD2DRenderer.sDXGISwapchain, &sBkgndColor);

	(*ptrpRenderer)->iActiveRendererAPI = Marble_RendererAPI_Direct2D;

ON_ERROR:
	return Marble_Renderer_Internal_Direct2DCleanup(ptrpRenderer, iErrorCode, NULL);
}

static void inline Marble_Renderer_Internal_Direct2DRenderer_Destroy(Marble_Renderer **ptrpRenderer) {
	Marble_Renderer_Internal_Direct2DCleanup(ptrpRenderer, Marble_ErrorCode_Unknown, NULL);
}

static void inline Marble_Renderer_Internal_Direct2DRenderer_Clear(Marble_Renderer *sRenderer, float fRed, float fGreen, float fBlue, float fAlpha) {
	D2D1_COLOR_F const sColor = {
		.r = fRed,
		.g = fGreen,
		.b = fBlue,
		.a = fAlpha
	};

	D2DWr_DeviceContext_Clear(sRenderer->sD2DRenderer.sD2DDevContext, &sColor);
}

static void inline Marble_Renderer_Internal_Direct2DRenderer_BeginDraw(Marble_Renderer *sRenderer) {
	D2DWr_DeviceContext_BeginDraw(sRenderer->sD2DRenderer.sD2DDevContext);
}

static void inline Marble_Renderer_Internal_Direct2DRenderer_EndDraw(Marble_Renderer *sRenderer) {
	D2DWr_DeviceContext_EndDraw(sRenderer->sD2DRenderer.sD2DDevContext, NULL, NULL);
}

static void inline Marble_Renderer_Internal_Direct2DRenderer_Resize(Marble_Renderer *sRenderer, UINT uiNewWidth, UINT uiNewHeight) {
	sRenderer->sD2DRenderer.sDXGISwapchain->lpVtbl->ResizeBuffers(
		gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain,
		2,
		uiNewWidth,
		uiNewHeight,
		DXGI_FORMAT_UNKNOWN,
		0
	);
}

static int Marble_Renderer_Internal_Direct2DRenderer_Recreate(Marble_Renderer **ptrpRenderer) {
	HWND hwRestoredWindow = NULL;
	Marble_Renderer_Internal_Direct2DCleanup(ptrpRenderer, Marble_ErrorCode_Unknown, &hwRestoredWindow);
	
	return Marble_Renderer_Internal_Direct2DRenderer_Create(ptrpRenderer, hwRestoredWindow);
}
#pragma endregion


int Marble_Renderer_Internal_Recreate(Marble_Renderer **ptrpRenderer) {
	if (ptrpRenderer && *ptrpRenderer) {
		switch ((*ptrpRenderer)->iActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: return Marble_Renderer_Internal_Direct2DRenderer_Recreate(ptrpRenderer);
		}

		return Marble_ErrorCode_RendererAPI;
	}

	return Marble_ErrorCode_RendererInit;
}


int Marble_Renderer_Create(Marble_Renderer **ptrpRenderer, DWORD dwActiveAPI, HWND hwRenderWindow) {
	if (ptrpRenderer) {
		if (!(*ptrpRenderer = malloc(sizeof(**ptrpRenderer))))
			return Marble_ErrorCode_MemoryAllocation;

		switch (dwActiveAPI) {
			case Marble_RendererAPI_Direct2D: return Marble_Renderer_Internal_Direct2DRenderer_Create(ptrpRenderer, hwRenderWindow);
		}

		return Marble_ErrorCode_RendererAPI;
	}

	return Marble_ErrorCode_RendererInit;
}

void Marble_Renderer_Destroy(Marble_Renderer **ptrpRenderer) {
	if (ptrpRenderer && *ptrpRenderer) {
		switch ((*ptrpRenderer)->iActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_Direct2DRenderer_Destroy(ptrpRenderer);
		}

		free(*ptrpRenderer);
		*ptrpRenderer = NULL;
	}
}

void Marble_Renderer_BeginDraw(Marble_Renderer *sRenderer) {
	if (sRenderer) {
		switch (sRenderer->iActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_Direct2DRenderer_BeginDraw(sRenderer);
		}
	}
}

void Marble_Renderer_EndDraw(Marble_Renderer *sRenderer) {
	if (sRenderer) {
		switch (sRenderer->iActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_Direct2DRenderer_EndDraw(sRenderer);
		}
	}
}

int Marble_Renderer_Present(Marble_Renderer **ptrpRenderer) {
	if (ptrpRenderer && *ptrpRenderer) {
		HRESULT hrRes = S_OK;

		switch (gl_sApplication.sRenderer->iActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: 
				hrRes = gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain->lpVtbl->Present(
					gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain, 
					(UINT)gl_sApplication.sMainWindow->sWndData.blIsVSync,
					0
				);

				if (hrRes == DXGI_ERROR_DEVICE_REMOVED || hrRes == DXGI_ERROR_DEVICE_RESET)
					return Marble_Renderer_Internal_Recreate(ptrpRenderer);

				return Marble_ErrorCode_Ok;
		}

		return Marble_ErrorCode_RendererAPI;
	}

	return Marble_ErrorCode_RendererInit;
}

void Marble_Renderer_Clear(Marble_Renderer *sRenderer, float fRed, float fGreen, float fBlue, float fAlpha) {
	if (gl_sApplication.sRenderer)
		switch (gl_sApplication.sRenderer->iActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_Direct2DRenderer_Clear(sRenderer, fRed, fGreen, fBlue, fAlpha); break;
		}
}

void Marble_Renderer_Resize(Marble_Renderer *sRenderer, UINT uiNewWidth, UINT uiNewHeight) {
	if (sRenderer)
		switch (sRenderer->iActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_Direct2DRenderer_Resize(sRenderer, uiNewWidth, uiNewHeight); break;
		}
}


