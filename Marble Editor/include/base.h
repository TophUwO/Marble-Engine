#pragma once

#include <windows.h>
#include <commctrl.h>


/*
 * All of the following "length" definitions denote
 * lengths in characters.
 */
#define MBE_MAXTSNAME (32)   /* max. tileset name length */
#define MBE_MAXCMT    (256)  /* max. comment length */
#define MBE_MAXTSC    (16)   /* max. number of tilesets per view */
#define MBE_MAXPATH   (1024) /* max. number of characters in a path */

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
 * Offset by WM_USER.
 */
#define MBE_WM_INITRES (WM_USER + 1) /* sent when the window has to initialize its own resources */

/*
 * Extra command for the scrollbar update function to
 * accept and process some keystrokes.
 */
#define MBE_SB_CTRL (1000)


/*
 * Structure representing scrollbar information for
 * a specific window.
 */
struct mbe_scrollinfo {
	BOOL m_isshow; /* visible state */
	int m_min;     /* min. scroll value */
	int m_cur;     /* current scroll value */
	int m_max;     /* max. scroll value */
};


