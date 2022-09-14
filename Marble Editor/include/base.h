#pragma once

#pragma warning (disable: 5105) /* macro expansion UB */

#include <windows.h>
#include <commctrl.h>
#include <math.h>

#include <system.h>
#include <util.h>
#include <log.h>
#include <d2dwr.h> 

/*
 * Enable visual styles to make UI look more modern
 * on modern platforms. I did not dislike the default design on
 * Windows 10, though. On Windows 11, however, things look weird
 * as some controls seem to have a more modern look by default,
 * such as scrollbars.
 * We just enable visual styles to make the look consistent.
 */
#pragma comment(linker,                       \
	"\"/manifestdependency:                   \
     type='win32'                             \
	 name='Microsoft.Windows.Common-Controls' \
     version='6.0.0.0'                        \
	 processorArchitecture='*'                \
	 publicKeyToken='6595b64144ccf1df'        \
	 language='*'\""                          \
)


/*
 * All of the following "length" definitions denote
 * lengths in characters.
 */
#define MBE_MAXIDLEN  (32)   /* max. tileset name length */
#define MBE_MAXCMT    (256)  /* max. comment length */
#define MBE_MAXTSC    (16)   /* max. number of tilesets per view */
#define MBE_MAXPATH   (1024) /* max. number of characters in a path */
#define MBE_MAXZSTEPS (32)   /* max. number of fixed zoom-steps */

/*
 * Macros to shorten the code needed to set the window/dialog
 * userdata.
 */
#define MBE_WNDUSERDATA(x, type) type x = (type)GetWindowLongPtr(p_hwnd, GWLP_USERDATA)
#define MBE_SETUDATA()           SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT *)lparam)->lpCreateParams)
/*
 * Macro serving as a shortcut to frequent calls to GetDlgItem.
 */
#define MBE_DLGWND(x)            GetDlgItem(p_hwnd, x)

/*
 * Custom window messages defined by this application.
 * Offset by WM_APP.
 */

/*
 * sent when the window has to initialize
 * its own resources
 */
#define MBE_WM_INITRES     (WM_APP + 1)
/*
 * sent to a dialog window when its controls
 * and its userdata must be initialized
 */
#define MBE_WM_INITDLGDATA (WM_APP + 2)

/*
 * Extra command for the scrollbar update function to
 * accept and process some keystrokes.
 */
#define MBE_SB_CTRL (1000)

/*
 * Component IDs 
 */
#define MBE_COMPID_LVLVIEW  (10)
#define MBE_COMPID_TSETVIEW (11)


/*
 * Structure representing scrollbar information for
 * a specific window.
 */
struct mbe_scrollinfo {
	SCROLLINFO ms_xscr; /* horizontal scrollbar info */
	SCROLLINFO ms_yscr; /* vertical scrollbar info */
	BOOL m_xscrv;       /* horizontal scrollbar visible state */
	BOOL m_yscrv;       /* vertical scrollbar visible state */
};

/*
 * Window size parameters. Used when a window is to be
 * created.
 * **m_xpos** and **m_ypos** are relative to the
 * upper-left corner of the parent window, or the screen
 * if there is none.
 */
struct mbe_wndsize {
	int m_xpos;   /* x-position, in pixels */
	int m_ypos;   /* y-positon, in pixels */
	int m_width;  /* width, in pixels */
	int m_height; /* height, in pixels */
};


/*
 * Calculates the position of a window relative
 * to the upper-left corner of its parent window.
 * 
 * Returns nothing.
 */
inline void mbe_base_getwindowpos(
	_In_  HWND p_hwnd,
	_Out_ POINT *ps_outpt
) {
	if (p_hwnd == NULL || ps_outpt == NULL)
		return;

	/* Get window position relative to screen. */
	RECT s_rect;
	GetWindowRect(p_hwnd, &s_rect);

	/*
	 * Transform screen coordinates relative
	 * to the upper-left corner of the parent
	 *  window of **p_hwnd**.
	 */
	MapWindowPoints(
		HWND_DESKTOP,
		GetParent(p_hwnd),
		(POINT *)&s_rect,
		1
	);

	*ps_outpt = *(POINT *)&s_rect;
}


