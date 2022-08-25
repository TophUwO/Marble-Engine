#pragma once

#include <error.h>
#include <base.h>


/*
 * Structure representing a logical tileset. 
 */
struct mbe_tset {
	int m_id;                      /* tileset ID */
	TCHAR maz_name[MBE_MAXTSNAME]; /* tileset name */
	TCHAR maz_cmt[MBE_MAXCMT];     /* comment/descriptio*/
	HWND p_hwnd;                   /* tileset view window */
	BOOL m_isinit;                 /* init state */

	/*
	 * Information regarding the dimensions of the in-memory
	 * object representing the tileset.
	 */
	struct {
		int tsize;     /* tile size, in pixels */

		int m_pwidth;  /* width, in pixels */
		int m_pheight; /* height, in pixels */
		int m_twidth;  /* width, in tiles */
		int m_theight; /* height, in tiles */
	} ms_sz;

	/*
	 * Information regarding the current position of
	 * the cursor on the bitmap, and regarding the
	 * last selection.
	 */
	struct {
		/* Current cursor position; 1 tile */
		int m_xindex;
		int m_yindex;

		/*
		 * Current selection rectangle.
		 * Used for drawing.
		 */
		RECT s_rsel;
	} ms_sel;

	/* GDI resources */
	struct {
		HDC p_hbmpdc;        /* bitmap device context */
		HBITMAP p_hbmpdcold; /* default DC bitmap */
		HBITMAP p_hbmpdcbmp; /* DC bitmap */

		/*
		 * Background color; only used when the tileset
		 * is saved to a file as a literal bitmap.
		 */
		COLORREF m_bkgnd;
	} ms_res;

	/* scrollbar information */
	SCROLLINFO s_xscr;
	SCROLLINFO s_yscr;
};

/*
 * Structure representing the tileset view window. Will be
 * implemented as a tab-view.
 */
struct mbe_tsetview {
	/* tab-view window */
	HWND mp_hwnd;

	BOOL m_isinit;              /* init state */
	int  m_nts;                 /* number of tilesets */
	int  m_curtsi;              /* current tileset index */
	struct mbe_tset mas_ts[32]; /* tilesets */
};


/*
 * Initializes tileset view.
 * This function does not setup the tileset view container itself,
 * which will be created when the first tileset is added to
 * the view.
 * 
 * Returns 0 on success, non-zero on failure.
 */
extern marble_ecode_t mbe_tsetview_init(
	HWND p_hparent,                  /* parent window */
	struct mbe_tsetview *ps_tsetview /* tileset view to init */
);

/*
 * Frees all resources occupied by tileset control
 * and uninitializes it.
 * 
 * Returns nothing.
 */
extern void mbe_tsetview_uninit(
	struct mbe_tsetview *ps_tsetview /* tilset view to uninit */
);

/*
 * Resizes tileset view container. The control will propagate
 * the message to its children as well.
 * 
 * Returns nothing.
 */
extern void mbe_tsetview_resize(
	struct mbe_tsetview *ps_tsetview, /* tileset view */
	int nwidth,                       /* new width, in pixels */
	int nheight                       /* new height, in pixels */
);

/*
 * Opens a dialog that allows the user to create a new,
 * blank tileset from scratch.
 * 
 * Returns 0 on success, non-zero on failure.
 */
extern marble_ecode_t mbe_tsetview_newtsdlg(
	struct mbe_tsetview *ps_tsetview
);

/*
 * Opens a dialog that allows the user to import a
 * bitmap as a tileset.
 * 
 * Returns 0 on success, non-zero on failure.
 */
extern marble_ecode_t mbe_tsetview_bmptsdlg(
	struct mbe_tsetview *ps_tsetview
);

/*
 * Sets the currently visible page of the tileset view container.
 * 
 * Returns nothing.
 */
extern void mbe_tsetview_setpage(
	struct mbe_tsetview *ps_tsetview, /* tileset view */
	int index                         /* new page index; 0 ... n */
);


