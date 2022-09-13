#pragma once

#include <editor.h>


/*
 * NOTE:
 * 
 * This header provides some commonly used functionality.
 * However, it is not required to use the generic functions
 * defined here, as each control may have additional rules
 * that control the behavior of certain message handlers.
 */


/*
 * Code shared by descendents of the "mbe_tabview"
 * control
 */
#pragma region TABVIEW-SHARED
/*
 * Structure representing common userdata for
 * the tab-view control itself.
 * It has to be the first field in userdata structures
 * of descendents of the "mbe_tabview" control.
 * It must be a non-pointer field.
 */
struct mbe_tabview_udata {
	HDC  mp_hdc;                      /* page-window device context */
	RECT ms_clrect;                   /* current client dimensions */

	ID2D1RenderTarget    *mp_rt;      /* page-window rendertarget */
	ID2D1SolidColorBrush *mp_brsolid; /* solid-color brush */
};


/*
 * Function called by controls derived from "mbe_tabview" to
 * initialize common userdata/resources.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
inline BOOL MB_CALLBACK mbe_tabview_inl_oncreate_common(
	_Inout_  struct mbe_tabview *ps_tview, /* tab-view to init userdata of */
	_In_opt_ void *p_crparams              /* optional create-params */
) { MB_ERRNO
	UNREFERENCED_PARAMETER(p_crparams);

	if (ps_tview == NULL)
		return FALSE;

	HRESULT hres;
	struct mbe_tabview_udata *ps_udata = NULL;

	/* Allocate userdata memory. */
	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof *ps_udata,
		false,
		false,
		&ps_udata
	);
	if (ecode != MARBLE_EC_OK)
		return FALSE;

	/* Update internal userdata pointer. */
	ps_tview->mp_udata = ps_udata;

	/* Set rendertarget properties. */
	D2D1_RENDER_TARGET_PROPERTIES const s_props = {
		.type = D2D1_RENDER_TARGET_TYPE_HARDWARE,
		.pixelFormat = {
			.format    = DXGI_FORMAT_B8G8R8A8_UNORM,
			.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED
		},
		.usage    = D2D1_RENDER_TARGET_USAGE_NONE,
		.minLevel = D2D1_FEATURE_LEVEL_DEFAULT
	};

	/* Create Direct2D rendertarget. */
	hres = D2DWr_Factory_CreateDCRenderTarget(
		gls_editorapp.ms_res.mp_d2dfac,
		&s_props,
		(ID2D1DCRenderTarget **)&ps_udata->mp_rt
	);
	if (hres != S_OK)
		return FALSE;

	/*
	 * Create the brush initially painting
	 * in solid black.
	 */
	D2D1_COLOR_F const s_cblack = { 0.0f, 0.0f, 0.0f, 1.0f };
	D2D1_BRUSH_PROPERTIES const s_brprops = { 1.0f };
	hres = D2DWr_RenderTarget_CreateSolidColorBrush(
		ps_udata->mp_rt,
		&s_cblack,
		&s_brprops,
		&ps_udata->mp_brsolid
	);
	if (hres != S_OK)
		return FALSE;

	/* Get GDI resources for the page-window. */
	GetClientRect(ps_tview->mp_hwndpage, &ps_udata->ms_clrect);
	ps_udata->mp_hdc = GetDC(ps_tview->mp_hwndpage);

	/*
	 * Bind the DC. Because the page-window is created with
	 * CS_OWNDC, the DC will always be the same, meaning we
	 * do not have to re-bind the DC every time we prepare for
	 * rendering. This improves rendering time considerably.
	 */
	D2DWr_DCRenderTarget_BindDC(
		(ID2D1DCRenderTarget *)ps_udata->mp_rt,
		ps_udata->mp_hdc,
		&ps_udata->ms_clrect
	);

	/*
	 * We do not have to unroll our resource allocation here, as
	 * returning FALSE in the event of an error will subsequently
	 * trigger the "ondestroy" handler of our tab-page, automatically
	 * deallocating everything that's already allocated.
	 */
	return TRUE;
}

/*
 * May be called by controls derived from "mbe_tabview" when the
 * view gets resized.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
inline BOOL MB_CALLBACK mbe_tabview_inl_onresize_common(
	_Inout_ struct mbe_tabview *ps_tview, /*tab-view */
	        int nwidth,                   /* new width of page window, in pixels */
	        int nheight                   /* new height of page window, in pixels */
) {
	if (ps_tview == NULL)
		return FALSE;

	/* Get a pointer to the common userdata. */
	struct mbe_tabview_udata *ps_udata = (struct mbe_tabview_udata *)ps_tview->mp_udata;
	if (ps_udata == NULL)
		return TRUE;

	/* Get new client dimensions. */
	GetClientRect(ps_tview->mp_hwndpage, &ps_udata->ms_clrect);

	/*
	 * Bind the DC with the new client rectangle,
	 * reflecting the new size of the page-window
	 * client area.
	 */
	D2DWr_DCRenderTarget_BindDC(
		(ID2D1DCRenderTarget *)ps_udata->mp_rt,
		ps_udata->mp_hdc,
		&ps_udata->ms_clrect
	);

	return TRUE;
}

/*
 * Destroys the userdata of the tab-view.
 * The function also has to deallocate the memory used
 * by the userdata structure itself.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
BOOL MB_CALLBACK mbe_tabview_inl_ondestroy_common(
	_Inout_ struct mbe_tabview *ps_tview /* tab-view to destroy userdata of */
) {
	if (ps_tview == NULL)
		return FALSE;

	/* Get a pointer to the common userdata. */
	struct mbe_tabview_udata *ps_udata = (struct mbe_tabview_udata *)ps_tview->mp_udata;
	if (ps_udata == NULL)
		return TRUE;

	/* Release GDI resources. */
	ReleaseDC(ps_tview->mp_hwndpage, ps_udata->mp_hdc);

	/* Release Direct2D resources. */
	D2DWR_SAFERELEASE(D2DWr_RenderTarget_Release, ps_udata->mp_rt);
	D2DWR_SAFERELEASE(D2DWr_SolidColorBrush_Release, ps_udata->mp_brsolid);

	/* Free struct memory. */
	free(ps_tview->mp_udata);
	return TRUE;
}
#pragma endregion


/*
 * Code shared by tab-pages across the entire
 * program. 
 */
#pragma region TABPAGE-SHARED
 /*
 * Carries out operations that are supposed to happen on resize,
 * such as scrollbar range updates etc.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
inline BOOL MB_CALLBACK mbe_tabpage_inl_onresize_common(
	_Inout_ struct mbe_tabpage *ps_tpage, /* tab-page */
	        int nwidth,                   /* new width of tab-page window */
	        int nheight,                  /* new height of tab-page window */
	        int cwidth,                   /* width of the page's contents */
	        int cheight                   /* height of the page's contents */
) {
	int maxscr;
	BOOL isxvisible, isyvisible;

	/* Update vertical scrollbar. */
	if (cheight > nheight) {
		maxscr = max(cheight - nheight, 0); 

		ps_tpage->ms_scrinfo.ms_yscr = (SCROLLINFO){
			.cbSize = sizeof ps_tpage->ms_scrinfo.ms_yscr,
			.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS,
			.nMin   = 0,
			.nMax   = cheight - 1,
			.nPage  = nheight,
			.nPos   = min(ps_tpage->ms_scrinfo.ms_yscr.nPos, maxscr)
		};

		isyvisible = TRUE;
	} else {
		/*
		* If the scrollbar needs to be hidden, we set the position of the
		* scrollbar to 0. We can safely do that as the scrollbars will only
		* be hidden if the entire bitmap axis the scrollbar is dedicated to
		* can be displayed in the current tileset view window.
		* If we do not set the value to 0 in such a case, the view will be stuck
		* in the view that it was scrolled to previously, possibly leaving
		* a portion of the tileset bitmap still hidden even though we have enough
		* space inside the window to display it.
		*/
		ps_tpage->ms_scrinfo.ms_yscr.fMask = SIF_POS;
		ps_tpage->ms_scrinfo.ms_yscr.nPos  = 0;

		isyvisible = FALSE;
	}
	/* Submit new scrollbar info. */
	SetScrollInfo(
		ps_tpage->ps_parent->mp_hwndpage,
		SB_VERT,
		&ps_tpage->ms_scrinfo.ms_yscr,
		TRUE
	);

	/* Update horizontal scrollbar. */
	if (cwidth > nwidth) {
		maxscr = max(cwidth - nwidth, 0);

		ps_tpage->ms_scrinfo.ms_xscr = (SCROLLINFO){
			.cbSize = sizeof ps_tpage->ms_scrinfo.ms_xscr,
			.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS,
			.nMin   = 0,
			.nMax   = cwidth - 1,
			.nPage  = nwidth,
			.nPos   = min(ps_tpage->ms_scrinfo.ms_xscr.nPos, maxscr)
		};

		isxvisible = TRUE;
	} else {
		ps_tpage->ms_scrinfo.ms_xscr.fMask = SIF_POS;
		ps_tpage->ms_scrinfo.ms_xscr.nPos  = 0;

		isxvisible = FALSE;
	}
	/* Submit new scrollbar info. */
	SetScrollInfo(
		ps_tpage->ps_parent->mp_hwndpage,
		SB_HORZ,
		&ps_tpage->ms_scrinfo.ms_xscr,
		TRUE
	);

	/* Update visible states of scrollbars. */
	ShowScrollBar(ps_tpage->ps_parent->mp_hwndpage, SB_HORZ, ps_tpage->ms_scrinfo.m_xscrv = isxvisible);
	ShowScrollBar(ps_tpage->ps_parent->mp_hwndpage, SB_VERT, ps_tpage->ms_scrinfo.m_yscrv = isyvisible);

	return TRUE;
}
#pragma endregion


