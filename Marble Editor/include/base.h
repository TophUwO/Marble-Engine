#pragma once

#include <windows.h>
#include <commctrl.h>


#define MBE_MAXTSNAME (32)  /* max. tileset name length */
#define MBE_MAXCMT    (256) /* max. comment length */
#define MBE_MAXTSC    (16)  /* max. number of tilesets per view */

/*
 * Macro to shorten the code needed to set the window/dialog
 * userdata.
 * This macro will be placed at the start of every window procedure
 * of a window that has custom userdata set.
 */
#define MBE_WNDUSERDATA(x, type) type x = (type)GetWindowLongPtr(p_hwnd, GWLP_USERDATA)

/*
 * Custom window messages defined by this application.
 * Offset by WM_USER.
 */
#define MBE_WM_INITRES (WM_USER + 1)


