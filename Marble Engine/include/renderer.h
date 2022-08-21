#pragma once

#include <dxgi1_2.h>
#include <d3d11.h>
#include <d2dwr.h>

#include <api.h>


/* 
 * Renderer API IDs 
 * 
 * Tells the system how to treat the renderer. Internally,
 * the renderer is abstracted away to support multiple
 * rendering APIs.
*/
enum marble_renderer_api {
	MARBLE_RENDERAPI_UNKNOWN = 0, /* reserved */

	MARBLE_RENDERAPI_DIRECT2D,    /* Direct2D renderer */

	__MARBLE_NUMRENDERAPIS__      /* for internal use */
};


/* Direct2D renderer structure */
struct marble_renderer_d2d {
	ID2D1Factory1      *mp_d2dfactory; /* Direct2D factory */
	ID2D1DeviceContext *mp_devicectxt; /* Device Context; acts as render target */
	IDXGISwapChain1    *mps_swapchain; /* Direct3D11 swapchain */
	ID2D1Device        *mp_device;     /* Direct2D device */
	ID2D1Bitmap1       *mp_bitmap;     /* Direct2D backbuffer */
};

/* Generic renderer structure */
struct marble_renderer {
	enum marble_renderer_api m_api; /* API */

    float m_orix;    /* x-Origin of drawing area */
    float m_oriy;    /* y-Origin of drawing area */
	bool  m_isinit;  /* Is renderer ready? */
	bool  m_isvsync; /* VSync state */

	/* Collection of renderer API structures */
	union {
		/* Direct2D renderer instance */
		struct marble_renderer_d2d ms_d2drenderer;
	};
};


/*
 * Creates a renderer.
 * 
 * A renderer is always associated and owned by a window.
 * 
 * If the renderer could be created successfully, the return value
 * is 0, otherwise non-zero.
 */
_Critical_ extern marble_ecode_t marble_renderer_create(
	                     enum marble_renderer_api api, /* API the renderer will use */
	                     bool isvsync,                 /* VSync enabled? */
	_In_                 HWND p_target,                /* window handle the renderer will be associated with */
	/*
	 * Pointer to a pointer to a "marble_renderer" structure that will receive
	 * a pointer to the freshly-created renderer structure.
	 * After "marble_renderer_create()" returned successfully, the renderer
	 * is ready to be used.
	 */
	_Init_(pps_renderer) struct marble_renderer **pps_renderer
);

/*
 * Unitializes a render and release all the resources it used.
 * 
 * Returns nothing.
 */
extern void marble_renderer_destroy(
	_Uninit_(pps_renderer) struct marble_renderer **pps_renderer /* pointer to the renderer instance */
);

/*
 * Prepares a renderer for drawing.
 * This function must be used in conjunction with "marble_renderer_enddraw()",
 * or else the state of the render may be indeterminate, causing unstable
 * behavior.
 * 
 * Returns nothing.
 */
extern void marble_renderer_begindraw(
	_In_ struct marble_renderer *ps_renderer /* renderer instance */
);

/*
 * Finalizes drawing.
 * This function must be used in conjunction with "marble_renderer_begindraw()",
 * or else the state of the render may be indeterminate, causing unstable
 * behavior.
 * 
 * Returns nothing.
 */
extern void marble_renderer_enddraw(
	_In_ struct marble_renderer *ps_renderer /* renderer instance */
);

/*
 * Presents a frame to the window manager.
 * 
 * Note: If the device became unusable during the last call to "marble_renderer_enddraw()",
 *       the renderer needs to be recreated. If this fails, the function will return non-zero
 *       and the renderer instance passed to this function will become invalid. In such a
 *       case, "marble_renderer_destroy()" is automatically called.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ extern marble_ecode_t marble_renderer_present(
	_In_ struct marble_renderer **pps_renderer /* pointer to the renderer instance */
);

/*
 * Resizes the internal buffers of a renderer.
 * 
 * Note: The function may fail to do so, invalidating the renderer. In
 *       this case, the renderer must be recreated. If this fails, the
 *       renderer instance passed to this function will 
 *       become invalid. However, "marble_renderer_destroy()"
 *       is automatically called.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ extern marble_ecode_t marble_renderer_resize(
	_In_ struct marble_renderer *ps_renderer, /* pointer to the renderer instance */
	     uint32_t newwidth,                   /* new width, in pixels */
	     uint32_t newheight                   /* new height, in pixels */
);


/*
 * Fills the render area with the color specified in RGBA.
 * 
 * Returns nothing.
 */
extern void marble_renderer_clear(
	_In_ struct marble_renderer *ps_renderer, /* renderer instance */
	     float red,                           /* red component (0.0 ... 1.0) */
	     float green,                         /* green component (0.0 ... 1.0) */
	     float blue,                          /* blue component (0.0 ... 1.0) */
	     float alpha                          /* alpha component (0.0 ... 1.0) */
);


