#pragma once

#include <api.h>
#include <error.h>


struct marble_tilebrowser {
	HWND mp_handle;

	HBRUSH p_hbkgnd;
};


extern marble_ecode_t marble_tilebrowser_create(
	struct marble_tilebrowser **pps_tbrowser
);


extern void marble_tilebrowser_destroy(
	struct marble_tilebrowser **pps_tbrowser
);

extern void marble_tilebrowser_resize(
	struct marble_tilebrowser *ps_tbrowser,
	int nwidth,
	int nheight
);


