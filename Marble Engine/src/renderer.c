#include <application.h>


/*
 * Generally, the "marble_renderer" structure just works as an
 * interface. Internally, it implements all supported render APIs,
 * and will call the correct routines based on the
 * renderer that is currently active.
 * Any routines that are required because of the differences between
 * render APIs will simply do nothing for any renderers
 * that do not need them.
 */


/*
 * Gets desktop DPI.
 * 
 * Returns nothing; but **p_dpi** will hold
 * the desktop DPI value.
 */
static uint32_t marble_renderer_internal_getdesktopdpi(void) {
	return (uint32_t)GetDpiForWindow(GetDesktopWindow());
}


#pragma region DIRECT2D-RENDERER
/*
 * Macro to run release functions on renderer objects. 
 * It automatically checks for validity of their parameter.
 */
#define marble_d2drenderer_release_s(ty, fn, inst)  if (inst) { fn((ty)(inst)); (inst) = NULL; }
/*
 * Macro to run release functions on renderer objects. 
 * It automatically checks for validity of their parameter.
 */
#define marble_d2drenderer_release_s_Vtbl(inst) if (inst) (inst)->lpVtbl->Release(inst)

 /*
 * Macro to shorten long swapchain queries.
 * Helps making the code a little bit clearer.
 */
#define MB_D2DSWAPCHAIN(inst)     ((inst)->ms_d2drenderer.mps_swapchain)
#define MB_D2DSWAPCHAIN_DIR(inst) ((inst)->mps_swapchain)
 /*
 * Macro to shorten long Vtbl calls of the swapchain.
 * Helps making the code a little bit clearer.
 */
#define MB_D2DSWAPCHAIN_VT(inst)     (MB_D2DSWAPCHAIN(inst)->lpVtbl)
#define MB_D2DSWAPCHAIN_VT_DIR(inst) (MB_D2DSWAPCHAIN_DIR(inst)->lpVtbl)

/*
 * Structure holding intermediate resource pointers.
 * 
 * When the operation is completed, "marble_direct2drenderer_releaseintermresources()"
 * will be called which will release everything which was required for
 * creating the renderer infrastructure, but is not needed for operation of said
 * renderer.
 */
struct marble_d2drenderer_intermres {
	IDXGISurface  *ps_dxgibuf;
	ID3D11Device  *ps_d3d11dev;
	IDXGIDevice   *ps_dxgidev;
	IDXGIAdapter  *ps_dxgiadapter;
	IDXGIFactory2 *ps_dxgifac;
	ID2D1Bitmap1  *ps_d2dbitmap;
};

/*
 * Direct2D factory options
 * 
 * Currently only used to enable debugging
 * features of Direct2D.
 */
D2D1_FACTORY_OPTIONS const gls_d2dfacprops = { 
	.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION 
};
/*
 * Feature levels that we will support
 * 
 * A graphics card not supporting any of the feature
 * levels listed below will not be able to
 * successfully initialize a Direct2D renderer.
 */
D3D_FEATURE_LEVEL const gla_d3dfeaturelvls[] = {
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0
};
/*
 * DXGI swapchain description
 * 
 * Holds all information that's needed to create a DXGI swapchain that is 
 * required to make use of a Direct2D render target.
 */
DXGI_SWAP_CHAIN_DESC1 gls_swapchaindesc = {
	.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,      /* alpha mode is determined by Direct2D backbuffer */
	.Width       = 0,                                /* same as target window */
	.Height      = 0,                                /* same as target window */
	.Flags       = 0,                                /* no additional flags */
	.BufferCount = 2,                                /* allow double-buffering */
	.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,  /* use target for output */
	.Format      = DXGI_FORMAT_B8G8R8A8_UNORM,       /* use default Direct2D's pixel format */
	.Scaling     = DXGI_SCALING_STRETCH,             /* does not matter as we will resize renderer together with the window */
	.Stereo      = FALSE,                            /* no VR for now */
	.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL, /* faster flip effect than blits */
	.SampleDesc  = {
		.Count   = 1,                                /* no multi-sampling */
		.Quality = 0                                 /* no antialiasing */
	}
};

/*
 * Properties for the Direct2D backbuffer.
 * 
 * DPI is obtained automatically (using desktop DPI in a
 * window device context).
 */
D2D1_BITMAP_PROPERTIES1 const gls_d2dbmpprops = {
	.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
	.pixelFormat   = {
		.format    = DXGI_FORMAT_B8G8R8A8_UNORM,   /* set to same pixel format as swapchain */
		.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED /* allow premultiplied alpha */
	}
};


/*
 * Release intermediate resources queried and allocated during the 
 * (re-)creation of the Direct2D renderer infrastructure.
 * 
 * Returns nothing.
 */
static void marble_d2drenderer_internal_releaseintermresources(
	struct marble_d2drenderer_intermres *ps_resources /* resources to free */
) {
	marble_d2drenderer_release_s_Vtbl(ps_resources->ps_dxgibuf);
	marble_d2drenderer_release_s_Vtbl(ps_resources->ps_dxgifac);
	marble_d2drenderer_release_s_Vtbl(ps_resources->ps_dxgidev);
	marble_d2drenderer_release_s_Vtbl(ps_resources->ps_dxgiadapter);
	marble_d2drenderer_release_s_Vtbl(ps_resources->ps_d3d11dev);
}

/*
 * Free resources occupied by the Direct2D renderer.
 * 
 * Passes through **ecode** variable as return value.
 */
static marble_ecode_t marble_d2drenderer_internal_cleanup(
	struct marble_renderer_d2d *ps_renderer, /* renderer to free resources of */
	int ecode,                               /* pass-through error code */
	HWND *p_restoredhwnd                     /* HWND used to recreate renderer if needed */
) {
	/*
	 * If the error code variable is set and **p_restoredhwnd** is non-NULL,
	 * (e.g. when we want to recreate the renderer), save the underlying window
	 * handle so we can associate our recreated renderer with it.
	 */
	if (ecode) {
		if (p_restoredhwnd && MB_D2DSWAPCHAIN_DIR(ps_renderer))
			MB_D2DSWAPCHAIN_VT_DIR(ps_renderer)->GetHwnd(MB_D2DSWAPCHAIN_DIR(ps_renderer), p_restoredhwnd);

		marble_d2drenderer_release_s(ID2D1Factory *, D2DWr_Factory_Release, ps_renderer->mp_d2dfactory);
		marble_d2drenderer_release_s_Vtbl(MB_D2DSWAPCHAIN_DIR(ps_renderer));
		marble_d2drenderer_release_s(ID2D1Bitmap *, D2DWr_Bitmap_Release, ps_renderer->mp_bitmap);
		marble_d2drenderer_release_s(ID2D1DeviceContext *, D2DWr_DeviceContext_Release, ps_renderer->mp_devicectxt);
		marble_d2drenderer_release_s(ID2D1Device *, D2DWr_Device_Release, ps_renderer->mp_device);
	}

	return ecode == MARBLE_EC_UNKNOWN ? MARBLE_EC_OK : ecode;
}

/*
 * Initializes the Direct2D renderer.
 * A Direct2D renderer always has to be associated with
 * an existing window.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t marble_d2drenderer_internal_init(
	HWND p_target, /* window to associate renderer with */
	/*
	 * Pointer to a "marble_renderer_direct2drenderer" structure whose members
	 * will be initialized one-by-one.
	 */
	struct marble_renderer_d2d *ps_renderer
) { MB_ERRNO; MB_COMERRNO
	ID3D11Device  *ps_d3d11dev    = NULL;
	IDXGIDevice   *ps_dxgidev     = NULL;
	IDXGISurface  *ps_dxgibuf     = NULL;
	IDXGIAdapter  *ps_dxgiadapter = NULL;
	IDXGIFactory2 *ps_dxgifac     = NULL;

	/* Create a Direct2D factory. */
	hres = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED, 
		&IID_ID2D1Factory1, 
		&gls_d2dfacprops,
		&ps_renderer->mp_d2dfactory
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_CREATED2DFAC;

		goto lbl_CLEANUP;
	}

	/* Create a Direct3D 11 device. */
	hres = D3D11CreateDevice(
		NULL, 
		D3D_DRIVER_TYPE_HARDWARE, 
		0, 
		D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, 
		gla_d3dfeaturelvls, 
		ARRAYSIZE(gla_d3dfeaturelvls), 
		D3D11_SDK_VERSION, 
		&ps_d3d11dev, 
		NULL, 
		NULL
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_CREATED3D11DEV;

		goto lbl_CLEANUP;
	}

	/*
	 * Get the DXGI device from the Direct3D 11 device
	 * so we can create a Direct2D compatible with it.
	 */
	hres = ps_d3d11dev->lpVtbl->QueryInterface(
		ps_d3d11dev, 
		&IID_IDXGIDevice, 
		&ps_dxgidev
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_GETDXGIDEV;

		goto lbl_CLEANUP; 
	}

	/* Create Direct2D device. */
	hres = D2DWr_Factory1_CreateDevice(
		ps_renderer->mp_d2dfactory, 
		ps_dxgidev, 
		&ps_renderer->mp_device
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_CREATED2DDEV;

		goto lbl_CLEANUP;
	}

	/* Create Direct2D device context. */
	hres = D2DWr_Device_CreateDeviceContext(
		ps_renderer->mp_device, 
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&ps_renderer->mp_devicectxt
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_CREATED2DDEVCTXT;

		goto lbl_CLEANUP;
	}

	/* Get the representation of the graphics adapter (GPU). */
	hres = ps_dxgidev->lpVtbl->GetAdapter(
		ps_dxgidev,
		&ps_dxgiadapter
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_GETDXGIADAPTER;

		goto lbl_CLEANUP;
	}

	/* Get factory so we can create a swap-chain with it. */
	hres = ps_dxgiadapter->lpVtbl->GetParent(
		ps_dxgiadapter, 
		&IID_IDXGIFactory2, 
		&ps_dxgifac
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_GETDXGIFAC;

		goto lbl_CLEANUP;
	}
	
	/* Create swap-chain. */
	hres = ps_dxgifac->lpVtbl->CreateSwapChainForHwnd(
		ps_dxgifac, 
		(IUnknown *)ps_d3d11dev, 
		p_target, 
		&gls_swapchaindesc,
		NULL, 
		NULL, 
		&ps_renderer->mps_swapchain
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_CREATESWAPCHAIN;

		goto lbl_CLEANUP;
	}
	
	/* Get DXGI backbuffer. */
	MB_D2DSWAPCHAIN_VT_DIR(ps_renderer)->GetBuffer(
		MB_D2DSWAPCHAIN_DIR(ps_renderer), 
		0,
		&IID_IDXGISurface, 
		&ps_dxgibuf
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_GETDXGIBACKBUFFER;

		goto lbl_CLEANUP;
	}
	
	/* Create a Direct2D backbuffer. */
	hres = D2DWr_DeviceContext_CreateBitmapFromDxgiSurface(
		ps_renderer->mp_devicectxt, 
		ps_dxgibuf, 
		&gls_d2dbmpprops, 
		&ps_renderer->mp_bitmap
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_CREATED2DBMPFROMDXGISURFACE;

		goto lbl_CLEANUP;
	}

	/* Associate backbuffer with render-target. */
	D2DWr_DeviceContext_SetTarget(
		ps_renderer->mp_devicectxt, 
		(ID2D1Image *)ps_renderer->mp_bitmap
	);

lbl_CLEANUP:
	/*
	 * Release all resources we do not need for
	 * the operation of the swap-chain.
	 */
	struct marble_d2drenderer_intermres s_freeres = {
		.ps_d3d11dev    = ps_d3d11dev,
		.ps_dxgidev     = ps_dxgidev,
		.ps_dxgiadapter = ps_dxgiadapter,
		.ps_dxgifac     = ps_dxgifac,
		.ps_dxgibuf     = ps_dxgibuf
	};
	marble_d2drenderer_internal_releaseintermresources(&s_freeres);
	
	return ecode;
}

/*
 * Destroys a Direct2D renderer.
 * 
 * Returns nothing.
 */
static void marble_d2drenderer_internal_destroy(
	struct marble_renderer_d2d *ps_renderer /* Direct2D renderer to destroy */
) {
	marble_d2drenderer_internal_cleanup(ps_renderer, MARBLE_EC_UNKNOWN, NULL);
}

/*
 * "Clear()" implementation for a Direct2D renderer.
 * This should be done before drawing anything meaningful
 * to the screen.
 * 
 * Returns nothing.
 */
static void marble_d2drenderer_internal_clear(
	struct marble_renderer_d2d *ps_renderer, /* Direct2D renderer */
	float fRed,                              /* red component */
	float fGreen,                            /* green component */
	float fBlue,                             /* blue component */
	float fAlpha                             /* alpha component */
) {
	D2D1_COLOR_F const sColor = {
		.r = fRed,
		.g = fGreen,
		.b = fBlue,
		.a = fAlpha
	};

	D2DWr_DeviceContext_Clear(ps_renderer->mp_devicectxt, &sColor);
}

/*
 * "BeginDraw()" implementation for a Direct2D renderer.
 * 
 * Returns nothing.
 */
static void marble_d2drenderer_internal_begindraw(
	struct marble_renderer_d2d *ps_renderer /* Direct2D renderer */
) {
	D2DWr_DeviceContext_BeginDraw(ps_renderer->mp_devicectxt);
}

/*
 * "EndDraw()" implementation for a Direct2D renderer.
 * 
 * Returns nothing.
 */
static void marble_d2drenderer_internal_enddraw(
	struct marble_renderer_d2d *ps_renderer /* Direct2D renderer */
) {
	D2DWr_DeviceContext_EndDraw(ps_renderer->mp_devicectxt, NULL, NULL);
}

/*
 * Recreates a Direct2D device context if it has become invalid (e.g. resized).
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t marble_d2drenderer_internal_recreatedevctxt(
	struct marble_renderer_d2d *ps_renderer /* Direct2D renderer to recreate */
) { MB_ERRNO
	HRESULT res = S_OK;
	IDXGISurface *ps_dxgibackbuf = NULL;

	/* Get DXGI backbuffer. */
	res = MB_D2DSWAPCHAIN_VT_DIR(ps_renderer)->GetBuffer(
		MB_D2DSWAPCHAIN_DIR(ps_renderer),
		0,
		&IID_IDXGISurface,
		&ps_dxgibackbuf
	);
	if (res != S_OK) {
		ecode = MARBLE_EC_GETDXGIBACKBUFFER;

		goto lbl_CLEANUP;
	}

	/* Create the device context. */
	res = D2DWr_Device_CreateDeviceContext(
			ps_renderer->mp_device, 
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE, 
			&ps_renderer->mp_devicectxt
	);
	if (res != S_OK) {
		ecode = MARBLE_EC_CREATED2DDEVCTXT;

		goto lbl_CLEANUP;
	}

	/* Create a Direct2D backbuffer. */
	res = D2DWr_DeviceContext_CreateBitmapFromDxgiSurface(
		ps_renderer->mp_devicectxt, 
		ps_dxgibackbuf, 
		&gls_d2dbmpprops, 
		&ps_renderer->mp_bitmap
	);
	if (res != S_OK) {
		ecode = MARBLE_EC_CREATED2DBMPFROMDXGISURFACE;

		goto lbl_CLEANUP;
	}

	/* Associate backbuffer with render-target. */
	D2DWr_DeviceContext_SetTarget(
		ps_renderer->mp_devicectxt,
		(ID2D1Image *)ps_renderer->mp_bitmap
	);

lbl_CLEANUP:
	/*
	 * If anything fails, the renderer will be
	 * destroyed completely, also destroying
	 * everything that actually succeeded to create.
	 */
	struct marble_d2drenderer_intermres s_freeres = {
		.ps_dxgibuf = ps_dxgibackbuf
	};
	marble_d2drenderer_internal_releaseintermresources(&s_freeres);

	return ecode;
}

/*
 * "Resize()" implementation for a Direct2D renderer.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t marble_d2drenderer_internal_resize(
	struct marble_renderer_d2d *ps_renderer, /* Direct2D renderer to resize */
	UINT newwidth,                           /* new width, in pixels */
	UINT newheight                           /* new height, in pixels */
) {
	/*
	 * Release outstanding references to DXGI's backbuffer,
	 * so we can resize properly.
	 */
	marble_d2drenderer_release_s(ID2D1DeviceContext *, D2DWr_DeviceContext_Release, ps_renderer->mp_devicectxt);
	marble_d2drenderer_release_s(ID2D1Bitmap *, D2DWr_Bitmap_Release, ps_renderer->mp_bitmap);

	/* Resize the backbuffers. */
	HRESULT res = MB_D2DSWAPCHAIN_VT_DIR(ps_renderer)->ResizeBuffers(
		MB_D2DSWAPCHAIN_DIR(ps_renderer),
		0,
		newwidth,
		newheight,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		0
	);
	if (res != S_OK)
		return MARBLE_EC_RESIZERENDERER;

	/* Recreate device context so we can continue rendering */
	return marble_d2drenderer_internal_recreatedevctxt(ps_renderer);
}

/*
 * Reinitializes the entire Direct2D renderer infrastructure.
 * This has to be done when the renderer becomes invalid (e.g.
 * when the Graphics Driver has to restart (e.g. after a driver update),
 * the GPU is being (physically) disconnected from the system,
 * or the GPU hangs).
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t marble_d2drenderer_internal_recreate(
	struct marble_renderer_d2d *ps_renderer
) {
	/*
	 * Because the reference to the window handle is lost when we destroy the window,
	 * we save it so we can recreate the renderer and associate it with the same
	 * window handle as before.
	 */
	HWND p_restoredwnd = NULL;
	marble_d2drenderer_internal_cleanup(ps_renderer, MARBLE_EC_UNKNOWN, &p_restoredwnd);
	
	/* Reinitialize the renderer infrastructure. */
	if (p_restoredwnd)
		return marble_d2drenderer_internal_init(p_restoredwnd, ps_renderer);

	return MARBLE_EC_INTERNALPARAM;
}
#pragma endregion


marble_ecode_t marble_renderer_create(
	enum marble_renderer_api api,
	HWND p_target,
	struct marble_renderer **pps_renderer
) { MB_ERRNO
	if (pps_renderer == NULL || p_target == NULL)
		return MARBLE_EC_INTERNALPARAM;

	ecode = marble_system_alloc(
		sizeof **pps_renderer,
		false,
		false,
		pps_renderer
	);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	switch (api) {
		case MARBLE_RENDERAPI_DIRECT2D:
			ecode = marble_d2drenderer_internal_init(
				p_target,
				&(*pps_renderer)->ms_d2drenderer
			);

			(*pps_renderer)->m_api = MARBLE_RENDERAPI_DIRECT2D;
			break;
		default: ecode = MARBLE_EC_UNIMPLFEATURE;
	}

	if (ecode == MARBLE_EC_OK)  {
		(*pps_renderer)->m_orix = 0;
		(*pps_renderer)->m_oriy = 0;

		(*pps_renderer)->m_isinit = true;
	}

	return ecode;
}

void marble_renderer_destroy(
	struct marble_renderer **pps_renderer
) {
	if (pps_renderer == NULL || *pps_renderer == NULL || (*pps_renderer)->m_isinit == false)
		return;

	/* Release render API-specific objects. */
	switch ((*pps_renderer)->m_api) {
		case MARBLE_RENDERAPI_DIRECT2D:
			marble_d2drenderer_internal_destroy(&(*pps_renderer)->ms_d2drenderer);

			break;
	}

	/* Free actual renderer instance memory. */
	free(*pps_renderer);
	*pps_renderer = NULL;
}

void marble_renderer_begindraw(
	struct marble_renderer *ps_renderer
) {
	if (ps_renderer == NULL || ps_renderer->m_isinit == false)
		return;

	/* Call implementations for specific render APIs. */
	switch (ps_renderer->m_api) {
		case MARBLE_RENDERAPI_DIRECT2D: 
			marble_d2drenderer_internal_begindraw(&ps_renderer->ms_d2drenderer);

			break;
	}
}

void marble_renderer_enddraw(
	struct marble_renderer *ps_renderer
) {
	if (ps_renderer == NULL || ps_renderer->m_isinit == false)
		return;

	/* Call implementations for specific render APIs. */
	switch (ps_renderer->m_api) {
		case MARBLE_RENDERAPI_DIRECT2D: 
			marble_d2drenderer_internal_enddraw(&ps_renderer->ms_d2drenderer);
			
			break;
	}
}

marble_ecode_t marble_renderer_present(
	struct marble_renderer **pps_renderer
) { MB_ERRNO
	if (pps_renderer == NULL || *pps_renderer == NULL) return MARBLE_EC_INTERNALPARAM;
	if ((*pps_renderer)->m_isinit == false)            return MARBLE_EC_COMPSTATE;

	/* Call implementations for specific render APIs. */
	switch ((*pps_renderer)->m_api) {
		case MARBLE_RENDERAPI_DIRECT2D: {
			HRESULT res = MB_D2DSWAPCHAIN_VT(*pps_renderer)->Present(
				MB_D2DSWAPCHAIN(*pps_renderer),
				0, /* TODO: will be VSync state of window */
				0
			);

			if (res == DXGI_ERROR_DEVICE_REMOVED || res == DXGI_ERROR_DEVICE_RESET) {
				ecode = marble_d2drenderer_internal_recreate(&(*pps_renderer)->ms_d2drenderer);

				if (ecode != MARBLE_EC_OK) {
					marble_application_raisefatalerror(MARBLE_EC_CREATERENDERER);

					return MARBLE_EC_CREATERENDERER;
				}
			}

			break;
		}
		default: 
			return MARBLE_EC_RENDERAPI;
	}

	return MARBLE_EC_OK;
}

marble_ecode_t marble_renderer_resize(
	struct marble_renderer *ps_renderer,
	uint32_t newwidth,
	uint32_t newheight
) {
	if (ps_renderer == NULL)            return MARBLE_EC_INTERNALPARAM;
	if (ps_renderer->m_isinit == false) return MARBLE_EC_COMPSTATE;

	/* Call implementations for specific render APIs. */
	switch (ps_renderer->m_api) {
		case MARBLE_RENDERAPI_DIRECT2D: return marble_d2drenderer_internal_resize(&ps_renderer->ms_d2drenderer, newwidth, newheight);
	}

	return MARBLE_EC_RENDERAPI;
}

void marble_renderer_clear(
	struct marble_renderer *ps_renderer,
	float red,
	float green,
	float blue,
	float alpha
) {
	if (ps_renderer == NULL || ps_renderer->m_isinit == false)
		return;

	/* Call implementations for specific render APIs. */
	switch (ps_renderer->m_api) {
		case MARBLE_RENDERAPI_DIRECT2D: 
			marble_d2drenderer_internal_clear(
				&ps_renderer->ms_d2drenderer,
				red,
				green,
				blue,
				alpha
			);

			break;
	}
}


