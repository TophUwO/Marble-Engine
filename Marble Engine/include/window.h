#pragma once

#include <api.h>
#include <error.h>


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
		bool     m_isvsync;     /* VSync state */
		bool     m_isfscreen;   /* fullscreen state */
		bool     m_isminimized; /* window size state */
		uint64_t m_lastupdate;  /* timestamp of last update */
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
		} ms_extends;
	} ms_data;
};


/*
 * Creates and displays a window.
 * 
 * Returns non-zero on failure, 0 on success.
 */
extern int marble_window_create(
	char const *pz_title, /* window title */
	uint32_t width,       /* width in pixels */
	uint32_t height,      /* height in pixels */
	bool isvsync,         /* Should VSync be enabled? */
	/*
	 * Pointer to a pointer to a "marble_window" structure
	 * that will receive the freshly-created window. 
	 * This parameter must not be NULL.
	 */
	struct marble_window **pps_window
);

/*
 * Destroys a window.
 * 
 * If the window that is to be destroyed is the main application
 * window, the app will subsequently perform an orderly
 * shutdown.
 * 
 * Returns nothing.
 */
extern void marble_window_destroy(
	/*
	 * Pointer to a pointer to a "marble_window"
	 * structure that should be destroyed.
	 * This parameter must not be NULL. 
	 */
	struct marble_window **pps_window
);

/*
 * Sets fullscreen state of a window.
 * 
 * Returns nothing.
 */
extern void inline marble_window_setfullscreen(
	struct marble_window *ps_window, /* window to be modified */
	bool isenabled                  /* enable or disable */
);

/*
* Sets VSync state of a window.
* 
* Returns nothing.
*/
extern void inline marble_window_setvsync(
	struct marble_window *ps_window, /* window to be modified */
	bool isenabled                  /* enable or disable */
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
	struct marble_window *ps_window, /* window to be modified */
	float frametime                 /* current frametime */
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
	struct marble_window *ps_window, /* window to be modified */
	uint32_t width,                 /* width, in tiles */
	uint32_t height,                /* height, in tiles */
	uint32_t tsize                  /* tile size, in pixels */
);


