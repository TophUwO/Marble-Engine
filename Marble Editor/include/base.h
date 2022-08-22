#pragma once

#include <windows.h>
#include <commctrl.h>


#define MBE_MAXNSTR   (8)   /* max. number string length */
#define MBE_MAXTSNAME (64)  /* max. tileset name length */
#define MBE_MAXCMT    (256) /* max. comment length */

/*
 * Macro to shorten the code needed to set the window/dialog
 * userdata.
 * This macro will be placed at the start of every window procedure
 * of a window that has custom userdata set.
 */
#define MBE_WNDUSERDATA(x, type) type x = (type)GetWindowLongPtr(p_hwnd, GWLP_USERDATA)


