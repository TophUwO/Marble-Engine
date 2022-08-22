#pragma once

#include <error.h>
#include <base.h>


/*
 * Structure representing a logical tileset. 
 */
struct marble_editor_tset {
	int m_id;                       /* tileset ID */
	char maz_name[MBE_MAXTSETNAME]; /* tileset name */
	HWND p_hwnd;                    /* tileset view window */

	/*
	 * Information regarding the dimensions of the in-memory
	 * object representing the tileset.
	 */
	struct {
		int tsize;                   /* tile size, in pixels */

		struct marble_sizei2d ms_td; /* dimensions of tileset, in tiles */
		struct marble_sizei2d ms_pd; /* dimensions of tileset, in pixels */
	} ms_dims;
};

/*
 * Structure representing the tileset view window. Will be implemented
 * as a tab-view.
 */
struct marble_editor_tsetview {
	HWND mp_hwnd; /* tab-view window */

	struct marble_util_vec *mps_tsets; /* tilesets */
};


extern marble_ecode_t mbeditor_tsetview_init(
	HWND p_hwndparent,
	struct marble_editor_tsetview *ps_tsetview
);

extern void mbeditor_tsetview_uninit(
	struct marble_editor_tsetview *ps_tsetview
);

extern void mbeditor_tsetview_resize(
	struct marble_editor_tsetview *ps_tsetview,
	int nwidth,
	int nheight
);


