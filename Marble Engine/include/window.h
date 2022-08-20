#pragma once

#include <api.h>
#include <error.h>
#include <marble.h>


/*
 * Structure holding information on the window manager. The only instance of
 * this structure is within "marble_application".
 */
struct marble_windowman {
	bool m_isinit;                       /* Is the window manager present? */

	struct marble_util_htable *mps_ht;   /* list of windows */
	struct marble_window   *mps_mainwnd; /* ref to the main window */
};

/*
 * Structure holding all information
 * about a window.
 * Note that some data, such as the window 
 * title, is not cached.
 */
struct marble_window {
	HWND                    mp_handle;                 /* handle to the window */
	char                    maz_strid[MB_STRINGIDMAX]; /* string id */
	struct marble_renderer *mps_renderer;              /* the window's renderer */

	/* Structure containing all miscelleneous window data */
	struct marble_window_data {
		bool     m_isvsync;     /* VSync state */
		bool     m_isfscreen;   /* fullscreen state */
		bool     m_isminimized; /* window size state */
		bool     m_ismainwnd;   /* Is the window the main window? */
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
		} ms_ext;
	} ms_data;
};


/*
 * Sets fullscreen state of a window.
 * 
 * Returns nothing.
 */
extern void inline marble_window_setfullscreen(
	_In_ struct marble_window *ps_wnd, /* window to be modified */
	     bool isenabled                /* enable or disable */
);

/*
* Sets VSync state of a window.
* 
* Returns nothing.
*/
extern void inline marble_window_setvsync(
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
	     uint32_t width,               /* width, in tiles */
	     uint32_t height,              /* height, in tiles */
	     uint32_t tsize                /* tile size, in pixels */
);


#pragma region WINDOWMAN
_Critical_ extern marble_ecode_t marble_windowman_init(
	_Pre_valid_ _Maybe_out_ struct marble_windowman *ps_wndman
);

extern void marble_windowman_uninit(
	struct marble_windowman *ps_wndman
);


/*
 * Creates a window.
 * 
 * Returns non-zero on failure, 0 on success.
 */
_Critical_ extern marble_ecode_t marble_windowman_createwindow(
	/* window manager */
	_In_            struct marble_windowman *ps_wndman,
	/* window and application settings */
	_In_            struct marble_app_settings const *ps_settings,
	                bool isvsync,     /* Should VSync be enabled? */
	                bool ismainwnd,   /* Should the window be the main window? */
	_In_z_          char const *pz_id /* window identifier */
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
extern void marble_windowman_destroywindow(
	_In_   struct marble_windowman *ps_wndman, /* window manager */
	_In_z_ char const *pz_id                   /* window id */
);

/*
 * Finds a window based on its string identifier.
 * 
 * Returns window handle is the window could be found,
 * or NULL if not or if an error occurred.
 */
extern struct marble_window *marble_windowman_findwindow(
	_In_   struct marble_windowman *ps_wndman, /* window manager */
	_In_z_ char const *pz_id                   /* window id */
);
#pragma endregion


