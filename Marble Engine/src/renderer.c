#pragma once

#include <application.h>


static void Marble_Renderer_Internal_GetDesktopDPI(float *fpDpiX, float *fpDpiY) {
	UINT udwDpi = GetDpiForWindow(GetDesktopWindow());

	*fpDpiX = *fpDpiY = (float)udwDpi;
}


#pragma region Direct2D Renderer
#define Marble_Direct2DRenderer_SafeRelease(fn, inst)  if (inst) { fn(inst); (inst) = NULL; }
#define Marble_Direct2DRenderer_SafeRelease_Vtbl(inst) if (inst) (inst)->lpVtbl->Release(inst)
#define Marble_Direct2DRenderer_Action(act, ret)       if (iErrorCode == Marble_ErrorCode_Ok) { if (act != S_OK) { iErrorCode = ret; } }


struct Marble_Renderer_Internal_Direct2DFreeIntermediateResources {
	IDXGISurface  *sDXGIBackbuffer;
	ID3D11Device  *sD3D11Device;
	IDXGIDevice   *sDXGIDevice;
	IDXGIAdapter  *sDXGIAdapter;
	IDXGIFactory2 *sDXGIFactory;
	ID2D1Bitmap1  *sD2DBitmap;
};


static void Marble_Renderer_Internal_Direct2DRenderer_FreeIntermediateResources(struct Marble_Renderer_Internal_Direct2DFreeIntermediateResources *sIntermediateResources) {
	Marble_Direct2DRenderer_SafeRelease_Vtbl(sIntermediateResources->sDXGIBackbuffer);
	Marble_Direct2DRenderer_SafeRelease_Vtbl(sIntermediateResources->sDXGIFactory);
	Marble_Direct2DRenderer_SafeRelease_Vtbl(sIntermediateResources->sDXGIDevice);
	Marble_Direct2DRenderer_SafeRelease_Vtbl(sIntermediateResources->sDXGIAdapter);
	Marble_Direct2DRenderer_SafeRelease_Vtbl(sIntermediateResources->sD3D11Device);
}

static int Marble_Renderer_Internal_Direct2DCleanup(Marble_Renderer **ptrpRenderer, int iErrorCode, HWND *hwpRestoredHWND) {
	if (iErrorCode) {
		if (hwpRestoredHWND && (*ptrpRenderer)->sD2DRenderer.sDXGISwapchain)
			(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain->lpVtbl->GetHwnd(
				(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain,
				hwpRestoredHWND
			);

		Marble_Direct2DRenderer_SafeRelease(D2DWr_Factory_Release, (ID2D1Factory *)(*ptrpRenderer)->sD2DRenderer.sD2DFactory);
		Marble_Direct2DRenderer_SafeRelease_Vtbl((*ptrpRenderer)->sD2DRenderer.sDXGISwapchain);
		Marble_Direct2DRenderer_SafeRelease(D2DWr_Bitmap_Release, (ID2D1Bitmap *)(*ptrpRenderer)->sD2DRenderer.sD2DBitmap);
		Marble_Direct2DRenderer_SafeRelease(D2DWr_DeviceContext_Release, (*ptrpRenderer)->sD2DRenderer.sD2DDevContext);
		Marble_Direct2DRenderer_SafeRelease(D2DWr_Device_Release, (*ptrpRenderer)->sD2DRenderer.sD2DDevice);
	}

	return iErrorCode == Marble_ErrorCode_Unknown ? Marble_ErrorCode_Ok : iErrorCode;
}

static int Marble_Renderer_Internal_Direct2DRenderer_Create(Marble_Renderer **ptrpRenderer, HWND hwRenderWindow) {
	extern void Marble_Renderer_Internal_Direct2DRenderer_SetBackgroundColor(Marble_Renderer *sRenderer, int iErrorCode);
	extern int  Marble_Renderer_Internal_SetActiveRendererAPI(Marble_Renderer *sRenderer, int iActiveAPI, int iErrorCode);

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

	ID3D11Device *sD3D11Device = NULL;
	Marble_Direct2DRenderer_Action(
		D3D11CreateDevice(NULL, 
			D3D_DRIVER_TYPE_HARDWARE, 
			0, 
			D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, 
			eaD3DFeatureLevels, 
			ARRAYSIZE(eaD3DFeatureLevels), 
			D3D11_SDK_VERSION, 
			&sD3D11Device, 
			NULL, 
			NULL
		), 
		Marble_ErrorCode_CreateD3D11Device
	);

	IDXGIDevice *sDXGIDevice = NULL;
	Marble_Direct2DRenderer_Action(
		sD3D11Device->lpVtbl->QueryInterface(
			sD3D11Device, 
			&IID_IDXGIDevice, 
			&sDXGIDevice
		), 
		Marble_ErrorCode_GetDXGIDevice
	);

	Marble_Direct2DRenderer_Action(
		D2DWr_Factory1_CreateDevice(
			(*ptrpRenderer)->sD2DRenderer.sD2DFactory, 
			sDXGIDevice, 
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

	IDXGIAdapter *sDXGIAdapter = NULL;
	Marble_Direct2DRenderer_Action(
		sDXGIDevice->lpVtbl->GetAdapter(
			sDXGIDevice, 
			&sDXGIAdapter
		), 
		Marble_ErrorCode_GetDXGIAdapter
	);

	IDXGIFactory2 *sDXGIFactory = NULL;
	Marble_Direct2DRenderer_Action(
		sDXGIAdapter->lpVtbl->GetParent(
			sDXGIAdapter, 
			&IID_IDXGIFactory2, 
			&sDXGIFactory
		), 
		Marble_ErrorCode_GetDXGIFactory
	);
	
	Marble_Direct2DRenderer_Action(
		sDXGIFactory->lpVtbl->CreateSwapChainForHwnd(
			sDXGIFactory, 
			(IUnknown *)sD3D11Device, 
			hwRenderWindow, 
			&sDXGISwapChainDesc,
			NULL, 
			NULL, 
			&(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain
		), 
		Marble_ErrorCode_CreateDXGISwapchain
	);
	
	IDXGISurface *sDXGIBackbuffer = NULL;
	Marble_Direct2DRenderer_Action(
		(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain->lpVtbl->GetBuffer(
			(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain, 
			0,
			&IID_IDXGISurface, 
			&sDXGIBackbuffer
		), 
		Marble_ErrorCode_GetDXGIBackbuffer
	);
	
	Marble_Direct2DRenderer_Action(
		D2DWr_DeviceContext_CreateBitmapFromDxgiSurface(
			(*ptrpRenderer)->sD2DRenderer.sD2DDevContext, 
			sDXGIBackbuffer, 
			&sD2DBitmapProps, 
			&(*ptrpRenderer)->sD2DRenderer.sD2DBitmap
		), 
		Marble_ErrorCode_CreateBitmapFromDxgiSurface
	);

	D2DWr_DeviceContext_SetTarget(
		(*ptrpRenderer)->sD2DRenderer.sD2DDevContext, 
		(ID2D1Image *)(*ptrpRenderer)->sD2DRenderer.sD2DBitmap
	);

	struct Marble_Renderer_Internal_Direct2DFreeIntermediateResources sFreeRes = {
		.sD3D11Device    = sD3D11Device,
		.sDXGIDevice     = sDXGIDevice,
		.sDXGIAdapter    = sDXGIAdapter,
		.sDXGIFactory    = sDXGIFactory,
		.sDXGIBackbuffer = sDXGIBackbuffer
	};
	Marble_Renderer_Internal_Direct2DRenderer_FreeIntermediateResources(&sFreeRes);
	Marble_Renderer_Internal_Direct2DRenderer_SetBackgroundColor(*ptrpRenderer, iErrorCode);

	return Marble_Renderer_Internal_SetActiveRendererAPI(*ptrpRenderer, Marble_RendererAPI_Direct2D, iErrorCode);
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

static int Marble_Renderer_Internal_Direct2DRenderer_RecreateDeviceContext(Marble_Renderer **ptrpRenderer) {
	D2D1_BITMAP_PROPERTIES1 const sD2DBitmapProps = {
		.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		.pixelFormat   = {
			.format    = DXGI_FORMAT_B8G8R8A8_UNORM,
			.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED
		}
	};

	int iErrorCode = Marble_ErrorCode_Ok;

	/* Get DXGI backbuffer */
	IDXGISurface *sDXGIBackbuffer = NULL;
	Marble_Direct2DRenderer_Action(
		(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain->lpVtbl->GetBuffer(
			(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain, 
			0,
			&IID_IDXGISurface, 
			&sDXGIBackbuffer
		), 
		Marble_ErrorCode_GetDXGIBackbuffer
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
		D2DWr_DeviceContext_CreateBitmapFromDxgiSurface(
			(*ptrpRenderer)->sD2DRenderer.sD2DDevContext, 
			sDXGIBackbuffer, 
			&sD2DBitmapProps, 
			&(*ptrpRenderer)->sD2DRenderer.sD2DBitmap
		),
		Marble_ErrorCode_CreateBitmapFromDxgiSurface
	);

	D2DWr_DeviceContext_SetTarget(
		(*ptrpRenderer)->sD2DRenderer.sD2DDevContext,
		(ID2D1Image *)(*ptrpRenderer)->sD2DRenderer.sD2DBitmap
	);

	struct Marble_Renderer_Internal_Direct2DFreeIntermediateResources sFreeRes = {
		.sDXGIBackbuffer = sDXGIBackbuffer
	};
	Marble_Renderer_Internal_Direct2DRenderer_FreeIntermediateResources(&sFreeRes);
	Marble_Renderer_Internal_Direct2DRenderer_SetBackgroundColor(*ptrpRenderer, iErrorCode);

	return Marble_Renderer_Internal_SetActiveRendererAPI(*ptrpRenderer, Marble_RendererAPI_Direct2D, iErrorCode);
}

static int Marble_Renderer_Internal_Direct2DRenderer_Resize(Marble_Renderer **ptrpRenderer, UINT uiNewWidth, UINT uiNewHeight) {
	/* Release outstanding references to DXGI's backbuffer, so we can resize properly. */
	Marble_Direct2DRenderer_SafeRelease(D2DWr_DeviceContext_Release, (*ptrpRenderer)->sD2DRenderer.sD2DDevContext);
	Marble_Direct2DRenderer_SafeRelease(D2DWr_Bitmap_Release, (ID2D1Bitmap *)(*ptrpRenderer)->sD2DRenderer.sD2DBitmap);

	(*ptrpRenderer)->sD2DRenderer.sDXGISwapchain->lpVtbl->ResizeBuffers(
		gl_sApplication.sRenderer->sD2DRenderer.sDXGISwapchain,
		0,
		uiNewWidth,
		uiNewHeight,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		0
	);

	/* Recreate device context so we can continue rendering */
	return Marble_Renderer_Internal_Direct2DRenderer_RecreateDeviceContext(ptrpRenderer);
}

static int Marble_Renderer_Internal_Direct2DRenderer_Recreate(Marble_Renderer **ptrpRenderer) {
	HWND hwRestoredWindow = NULL;
	Marble_Renderer_Internal_Direct2DCleanup(ptrpRenderer, Marble_ErrorCode_Unknown, &hwRestoredWindow);
	
	return Marble_Renderer_Internal_Direct2DRenderer_Create(ptrpRenderer, hwRestoredWindow);
}

static void Marble_Renderer_Internal_Direct2DRenderer_SetBackgroundColor(Marble_Renderer *sRenderer, int iErrorCode) {
	if (iErrorCode == Marble_ErrorCode_Ok) {
		DXGI_RGBA sBkgndColor = {
			0.0f,
			0.0f,
			0.0f,
			1.0f
		};

		sRenderer->sD2DRenderer.sDXGISwapchain->lpVtbl->SetBackgroundColor(sRenderer->sD2DRenderer.sDXGISwapchain, &sBkgndColor);
	}
}
#pragma endregion


static int inline Marble_Renderer_Internal_SetActiveRendererAPI(Marble_Renderer *sRenderer, int iActiveAPI, int iErrorCode) {
	if (iErrorCode == Marble_ErrorCode_Ok)
		sRenderer->iActiveRendererAPI = iActiveAPI;

	return iErrorCode;
}


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

void Marble_Renderer_Resize(Marble_Renderer **ptrpRenderer, UINT uiNewWidth, UINT uiNewHeight) {
	if (ptrpRenderer && *ptrpRenderer)
		switch ((*ptrpRenderer)->iActiveRendererAPI) {
			case Marble_RendererAPI_Direct2D: Marble_Renderer_Internal_Direct2DRenderer_Resize(ptrpRenderer, uiNewWidth, uiNewHeight); break;
		}
}


