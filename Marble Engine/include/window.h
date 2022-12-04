#pragma once

#include <api.h>
#include <error.h>
#include <marble.h>


MB_BEGIN_HEADER


/*
 * Structure holding all information
 * about a window.
 * Note that some data, such as the window 
 * title, is not cached.
 */
struct marble_window {
	HWND                    mp_handle;    /* handle to the window */
	struct marble_renderer *mps_renderer; /* the window's renderer */

	/* Structure containing all miscelleneous window data */
	struct marble_window_data {
		bool     m_isfscreen;   /* fullscreen state */
		bool     m_isminimized; /* window size state */
		/* window styles (system-specific) */
		uint32_t m_style;

		/* 
		 * Structure containing all information regarding
		 * a window's dimensions.
		 */
		struct {
			uint32_t m_tsize; /* tile size, in pixels */

			/* extends of rendering area, in pixels */
			struct marble_sizei2d ms_client;
			/* extends of window area, in pixels */
			struct marble_sizei2d ms_window;
		} ms_ext;

        /* update timer */
        struct marble_util_clock ms_updt;
	} ms_data;
};


/*
 * Creates a window.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ extern marble_ecode_t marble_window_create(
	_In_            struct marble_app_settings const *ps_settings, /* window and app settings */
	                bool isvsync,                                  /* VSync state */
	_Init_(pps_wnd) struct marble_window **pps_wnd                 /* pointer to destination structure */
);

/*
 * Destroys a window and its renderer.
 * All resources occupied by this window are released.
 * 
 * Returns nothing.
 */
extern void marble_window_destroy(
	_Uninit_(pps_wnd) struct marble_window **pps_wnd
);

/*
 * Sets fullscreen state of a window.
 * 
 * Returns nothing.
 */
extern void marble_window_setfullscreen(
	_In_ struct marble_window *ps_wnd, /* window to be modified */
	     bool isenabled                /* enable or disable */
);

/*
* Sets VSync state of a window.
* 
* Returns nothing.
*/
extern void marble_window_setvsync(
	_In_ struct marble_window *ps_wnd, /* window to be modified */
	     bool isenabled                /* enable or disable */
);

/*
 * Updates window data.
 * 
 * This function also performs the rendering
 * to the window.
 * 
 * Returns nothing.
 */
extern void marble_window_update(
	_In_ struct marble_window *ps_wnd, /* window to be modified */
	     float frametime               /* current frametime */
);

/*
 * Resizes a window.
 * 
 * The function will always make sure that
 * the window fits on the screen it is currently
 * displayed on. Invalid parameters are
 * adjusted accordingly.
 * 
 * Returns nothing.
 */
extern void marble_window_resize(
	_In_ struct marble_window *ps_wnd, /* window to be modified */
	     int width,                    /* width, in tiles */
	     int height,                   /* height, in tiles */
	     int tsize                     /* tile size, in pixels */
);


/*
 * Initializes the window system. This must be done before a window
 * can be created.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ extern marble_ecode_t marble_windowsys_init(void);

/*
 * Uninitializes the window system.
 * 
 * Returns othing.
 */
extern void marble_windowsys_uninit(void);


MB_END_HEADER


