#pragma once

#include <error.h>
#include <base.h>


/*
 * Structure representing a logical tileset. 
 */
struct mbeditor_tset {
	int m_id;                     /* tileset ID */
	char maz_name[MBE_MAXTSNAME]; /* tileset name */
	char maz_cmt[MBE_MAXCMT];     /* comment/descriptio*/
	HWND p_hwnd;                  /* tileset view window */

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
struct mbeditor_tsetview {
	/* tab-view window */
	HWND mp_hwnd;

	int m_nts;                          /* number of tilesets */
	int m_curtsi;                       /* current tileset index */
	struct mbeditor_tset mas_tsets[32]; /* tilesets */
};


extern marble_ecode_t mbeditor_tsetview_init(
	HWND p_hwndparent,
	struct mbeditor_tsetview *ps_tsetview
);

extern void mbeditor_tsetview_uninit(
	struct mbeditor_tsetview *ps_tsetview
);

extern void mbeditor_tsetview_resize(
	struct mbeditor_tsetview *ps_tsetview,
	int nwidth,
	int nheight
);

extern marble_ecode_t mbeditor_tsetview_newtsdlg(
	struct mbeditor_tsetview *ps_tsetview
);


