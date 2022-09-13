#include <internal/shared.h>


WNDPROC glfn_tabviewdefproc = NULL;
TCHAR const *const glpz_tabviewwndcl = TEXT("mbe_ctrl_tabview");
TCHAR const *const glpz_pagewndcl    = TEXT("mbe_ctrl_tabpage");


#pragma region TABPAGE-CTRL
#define MBE_TVIEW_FIXCB(member, def) (void)(member == NULL ? (member = &def) : (0))


/* default tab-view callbacks */
static BOOL MB_CALLBACK mbe_tabview_int_defoncreate(_Inout_ struct mbe_tabview *ps_tview, _In_opt_ void *p_crparams) { return TRUE; }
static BOOL MB_CALLBACK mbe_tabview_int_defonresize(_Inout_ struct mbe_tabview *ps_tview, int nwidth, int nheight) { return TRUE; }
static BOOL MB_CALLBACK mbe_tabview_int_defondestroy(_Inout_ struct mbe_tabview *ps_tview) { return TRUE; }

/* default tab-page callbacks */
static BOOL MB_CALLBACK mbe_pageview_int_defoncreate(_Inout_ struct mbe_tabpage *ps_tpage, _In_opt_ void *p_crparams) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defonpaint(_Inout_ struct mbe_tabpage *ps_tpage) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defonresize(_Inout_ struct mbe_tabpage *ps_tpage, int nwidth, int nheight) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defonzoom(_Inout_ struct mbe_tabpage *ps_tpage) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defonselect(_Inout_ struct mbe_tabpage *ps_tpage) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defonunselect(_Inout_ struct mbe_tabpage *ps_tpage) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defondestroy(_Inout_ struct mbe_tabpage *ps_tpage) { return TRUE; }

/* default tab-view callback struct */
static struct mbe_tabview_callbacks const gls_defviewcbs = {
	&mbe_tabview_int_defoncreate,
	&mbe_tabview_int_defonresize,
	&mbe_tabview_int_defondestroy
};

/* default tab-page callback struct */
static struct mbe_tabpage_callbacks const gls_defpagecbs = {
	&mbe_pageview_int_defoncreate,
	&mbe_pageview_int_defonpaint,
	&mbe_pageview_int_defonresize,
	&mbe_pageview_int_defonzoom,
	&mbe_pageview_int_defonselect,
	&mbe_pageview_int_defonunselect,
	&mbe_pageview_int_defondestroy
};

/*
 * Destroys a tab page.
 * Userdata is destroyed using a special
 * callback.
 * 
 * Returns nothing.
 */
static void mbe_tabpage_int_destroy(
	_Uninit_(pps_tpage) struct mbe_tabpage **pps_tpage /* tab page to destroy */
) {
	if (pps_tpage == NULL || *pps_tpage == NULL)
		return;

	/* Destroy userdata. */
	(*(*pps_tpage)->ms_cbs.mpfn_ondestroy)(*pps_tpage);

	/* Destroy common page data. */
	free((*pps_tpage)->mpz_name);
	free((*pps_tpage)->mpz_cmt);

	free(*pps_tpage);
	*pps_tpage = NULL;
}

/*
 * Handles the messages related to the scrollbar.
 * 
 * Returns nothing.
 */
static void mbe_tabpage_int_handlescrolling(
	_In_ struct mbe_tabview *ps_tview, /* tab-view */
	     UINT msg,                     /* exact message */
	     LONG param                    /* additional parameter */
) {
#define MBE_ISUPSCRMSG(msg)   (msg == SB_PAGEUP || msg == SB_LINEUP)
#define MBE_ISDOWNSCRMSG(msg) (msg == SB_PAGEDOWN || msg == SB_LINEDOWN)

	int newpos;
	float fac = 1.0f;

	/*
	 * Mouse wheel behavior:
	 * 
	 * Scrolling:         Vertical scroll
	 * Scrolling + SHIFT: Horizontal scroll
	 * 
	 * Scrolling occurs in (**gl_viewtsize**) * (HIWORD(**wparam**) / WHEEL_DELTA)
	 * steps.
	 * The scrolling step may be smaller than WHEEL_DELTA (120) since
	 * there are mice with a free mouse wheel for higher precision
	 * scrolling.
	 */
	switch (msg) {
		case WM_MOUSEWHEEL:
			/*
			 * Translate WM_MOUSEWHEEL into WM_HSCROLL/WM_VSCROLL
			 * + SB_LINEDOWN/SB_LINEUP combination so that the
			 * function can deal with the message normally.
			 */
			fac   = -((SHORT)HIWORD(param) / (float)WHEEL_DELTA);
			msg   = LOWORD(param) & MK_SHIFT ? WM_HSCROLL : WM_VSCROLL;
			param = MAKELONG(HIWORD(param) < 0 ? SB_LINEUP : SB_LINEDOWN, 0);

			break;
		case WM_KEYDOWN: {
			param = MAKELONG(MBE_SB_CTRL, (WORD)param);

			msg = gls_editorapp.ms_flags.mf_isshift != FALSE 
				? WM_HSCROLL
				: WM_VSCROLL
			;
			break;
		}
	}

	/* Get correct scrollbar information structure. */
	SCROLLINFO *ps_scrinfo = msg == WM_HSCROLL
		? &ps_tview->mps_cursel->ms_scrinfo.ms_xscr
		: &ps_tview->mps_cursel->ms_scrinfo.ms_yscr
	;

	/*
	 * If the current scrollbar is not visible, do not
	 * allow scrolling of the window by other means such
	 * as the mouse-wheel or even the keyboard.
	 */
	if (msg == WM_HSCROLL && ps_tview->mps_cursel->ms_scrinfo.m_xscrv == FALSE || msg == WM_VSCROLL && ps_tview->mps_cursel->ms_scrinfo.m_yscrv == FALSE)
		return;

	/*
	 * Simply ignore (yet) unknown commands by
	 * setting the new scroll position to the
	 * current one.
	 */
	newpos = ps_scrinfo->nPos;
	fac   *= MBE_ISUPSCRMSG(LOWORD(param)) ? -1.0f : 1.0f;
	switch (LOWORD(param)) {
		case MBE_SB_CTRL:
			/* Handle some custom keystrokes. */
			switch (HIWORD(param)) {
				case VK_HOME: newpos = 0; break;
				case VK_END:
					newpos = ps_scrinfo->nMax - ps_scrinfo->nPage + 1;
				
					break;
			}

			break;
		case SB_PAGEUP:
		case SB_PAGEDOWN:
			newpos = ps_scrinfo->nPos + (int)((float)ps_scrinfo->nPage * fac);

			goto lbl_CLAMP;
		case SB_LINEUP:
		case SB_LINEDOWN:
			newpos = ps_scrinfo->nPos + (int)(32.0f * fac);

			/*
			 * As clamping to 0 < newpos < ps_scrinfo->nMax would
			 * allow the user to scroll outside of the bitmap, we
			 * clamp to the origin of the last page so the last
			 * position that can be scrolled to is essentially
			 * the same position that the scrollbars scroll to.
			 */
		lbl_CLAMP:
			newpos = min(ps_scrinfo->nMax - ps_scrinfo->nPage +	1, (UINT)max(0, newpos));

			/* Skip the regular clamp. */
			goto lbl_UPDATE;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			newpos = HIWORD(param);

			break;
	}
	/* Clamp value. */
	newpos = min(ps_scrinfo->nMax, max(0, newpos));

lbl_UPDATE:
	/* Update scrollbar info. */
	ps_scrinfo->fMask = SIF_POS;
	ps_scrinfo->nPos  = newpos;

	SetScrollInfo(
		ps_tview->mp_hwndpage,
		msg == WM_HSCROLL
			? SB_HORZ
			: SB_VERT
		, ps_scrinfo,
		TRUE
	);

	/* Repaint the current page. */
	InvalidateRect(ps_tview->mp_hwndpage, NULL, FALSE);
	UpdateWindow(ps_tview->mp_hwndpage);
}

/*
 * Finds the current zoom-factor in the zoom-step list.
 * 
 * Returns the index, 0 ... MBE_MAXZSTEPS - 1, on success,
 * or -1 on error.
 */
static int mbe_tabview_int_findcurrzoomindex(
	_In_ struct mbe_tabview_udata *ps_udata /* tab-view userdata */
) {
	for (int i = 0; i < ps_udata->m_nzsteps; i++)
		if (ps_udata->map_zoomsteps[i] == ps_udata->m_zoom)
			return i;

	return -1;
}

/*
 * Handles the event of the user holding down CTRL and scrolling with
 * the mouse-wheel, i.e. a zoom.
 * The zoom factor is the same for all pages at all times.
 * 
 * Returns nothing.
 */
static void mbe_tabview_int_handlezoom(
	_In_ struct mbe_tabview *ps_tview, /* tab-view */
	     WPARAM wparam
) {
	if (ps_tview == NULL || ps_tview->m_isinit == FALSE)
		return;

	/* Get a pointer to the view userdata. */
	struct mbe_tabview_udata *ps_udata = (struct mbe_tabview_udata *)ps_tview->mp_udata;
	if (ps_udata == NULL)
		return;

	/* Get index of current zoom-factor. */
	int currindex = mbe_tabview_int_findcurrzoomindex(ps_udata);
	if (currindex == -1)
		return;

	/* Clamp new zoom-factor index. */
	int newindex = min(
		ps_udata->m_nzsteps - 1,
		max(
			0,
			currindex + ((short)HIWORD(wparam) > 0 ? 1 : -1)
		)
	);

	/*
	 * If the new zoom-factor does not differ from
	 * the previous zoom-factor, do not update the
	 * windows.
	 */
	if (newindex == currindex)
		return;

	/* Update zoom-factor. */
	ps_udata->m_zoom = ps_udata->map_zoomsteps[newindex];

	/* Let all pages react to the zoom event. */
	for (size_t i = 0; i < ps_tview->mps_pages->m_size; i++) {
		struct mbe_tabpage *ps_page = (struct mbe_tabpage *)marble_util_vec_get(ps_tview->mps_pages, i);
		if (ps_page == NULL)
			break;

		(*ps_page->ms_cbs.mpfn_onzoom)(ps_page);
	}

	/* Repaint the current page. */
	InvalidateRect(ps_tview->mp_hwndpage, NULL, FALSE);
	UpdateWindow(ps_tview->mp_hwndpage);
}

/*
 * Get display area rectangle of tab control, relative
 * to the upper-left corner of the tab control client
 * rectangle.
 * 
 * Returns nothing.
 */
static void mbe_tabview_int_getdisprect(
	_In_  HWND p_hwnd,
	_Out_ RECT *ps_outrect
) {
	if (p_hwnd == NULL || ps_outrect == NULL)
		return;

	/* Get full client area of tab view. */
	GetClientRect(p_hwnd, ps_outrect);

	/*
	 * Adjust client rectangle so it reflects only the actual
	 * display area of the tab-view, excluding the register view
	 * and other non-client features.
	 */
	TabCtrl_AdjustRect(p_hwnd, FALSE, ps_outrect);
}

/*
 * Resizes the page window of the tab-view. It also recalculates the
 * coordinates of the display rect.
 * 
 * Returns nothing.
 */
static void mbe_tabview_int_resizepagewnd(
	_In_ struct mbe_tabview *ps_tview, /* tabview to resize page window of */
	     BOOL resizewnd                /* page-window resize flag */
) {
	if (ps_tview == NULL)
		return;

	/* Get dimensions of display area. */
	mbe_tabview_int_getdisprect(ps_tview->mp_hwndtab, &ps_tview->ms_dimensions);

	/*
	 * Set position and dimensions to exactly reflect the
	 * size of the display area of the tab-view.
	 */
	if (resizewnd == TRUE)
		MoveWindow(
			ps_tview->mp_hwndpage,
			ps_tview->ms_dimensions.left,
			ps_tview->ms_dimensions.top,
			ps_tview->ms_dimensions.right - ps_tview->ms_dimensions.left,
			ps_tview->ms_dimensions.bottom - ps_tview->ms_dimensions.top,
			TRUE
		);

	/*
	 * Execute tab-view "onresize" handler. This handler is used to update
	 * resources that are related to the tab-page window, but shared by
	 * all tab-pages.
	 */
	(*ps_tview->ms_cbs.mpfn_onresize)(
		ps_tview,
		ps_tview->ms_dimensions.right - ps_tview->ms_dimensions.left,
		ps_tview->ms_dimensions.bottom - ps_tview->ms_dimensions.top
	);
}

LRESULT CALLBACK mbe_tabpage_int_wndproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	MBE_WNDUSERDATA(ps_udata, struct mbe_tabview *);

	/*
	 * If we want to process a message other than WM_CREATE,
	 * we only allow it to be processed by this function if the
	 * tab-view is properly initialized and has a page selected.
	 */
	if (msg != WM_CREATE && (ps_udata == NULL || ps_udata->m_isinit == FALSE || ps_udata->mps_cursel == NULL))
		goto lbl_DEFPROC;

	switch (msg) {
		case WM_CREATE:
			MBE_SETUDATA();

			return 0;
		case WM_SIZE:
			/*
			 * Execute "onresize" handler of the currently
			 * selected page.
			 */
			(*ps_udata->mps_cursel->ms_cbs.mpfn_onresize)(
				ps_udata->mps_cursel,
				GET_X_LPARAM(lparam),
				GET_Y_LPARAM(lparam)
			);

			/*
			 * Update the size of the display rectangle of
			 * the tab-view.
			 * 
			 * This has to be done here as well "mbe_tabview_resize()" only
			 * handles resize requests from the parent window. However,
			 * resizing can also happen from within the window, for example,
			 * when scrollbars get shown or hidden.
			 */
			mbe_tabview_int_resizepagewnd(ps_udata, FALSE);

			/* Post a WM_PAINT message. */
			InvalidateRect(p_hwnd, NULL, FALSE);
			return 0;
		case WM_VSCROLL:
		case WM_HSCROLL:
		case WM_MOUSEWHEEL:
		case WM_KEYDOWN:
			/*
			 * Holding down CTRL while moving the mouse-wheel
			 * triggers a zoom event.
			 */
			if (msg == WM_MOUSEWHEEL && LOWORD(wparam) == MK_CONTROL) {
				mbe_tabview_int_handlezoom(
					ps_udata,
					wparam
				);

				return 0;
			}

			/* Handle regular scrolling. */
			mbe_tabpage_int_handlescrolling(
				ps_udata,
				msg,
				(LONG)wparam
			);

			return 0;
		case WM_PAINT:
			/*
			 * Execute "onpaint" handler of the currently-selected
			 * page to draw the view.
			 */
			(*ps_udata->mps_cursel->ms_cbs.mpfn_onpaint)(ps_udata->mps_cursel);

			/* Mark the window update region as empty. */
			ValidateRect(p_hwnd, NULL);
			return 0;
	}

lbl_DEFPROC:
	return DefWindowProc(p_hwnd, msg, wparam, lparam);
}
#pragma endregion


#pragma region TABVIEW-CTRL
/*
 * Sets the page callbacks to the provided functions, and replace any NULL
 * pointers (i.e. undefined callbacks) with default callback functions.
 * These default callbacks normally do nothing but return TRUE, signifying
 * a successful result.
 * 
 * Returns nothing.
 */
static void mbe_tabview_int_setcbs(
	_In_     void *restrict p_cbs,       /* callback struct to init */
	_In_opt_ void const *restrict p_src, /* callback source */
	_In_     void const *restrict p_def, /* default callbacks */
	_In_     size_t structsz             /* callback struct size, in bytes */
) {
	if (p_cbs == NULL)
		return;

	/*
	 * If no callbacks were given, just set
	 * everything to default.
	 */
	if (p_src == NULL) {
		memcpy(p_cbs, p_def, structsz);

		return;
	}

	/* Calculate number of callbacks to iterate over */
	size_t const nelem = structsz / sizeof p_cbs;

	/* Begin with simply copying over the given callbacks. */
	memcpy(p_cbs, p_src, structsz);

	/*
	 * Iterate over callback struct, replacing NULL callbacks
	 * in **p_src** with the corresponding default callback
	 * provided by **p_def**.
	 */
	for (size_t index = 0; index < nelem; index++) {
		off_t const offset = (off_t)(index * sizeof p_cbs);
		void *p_givencb    = (BYTE *)p_cbs + offset;

		if (*(INT_PTR *)p_givencb == (INT_PTR)0) {
			void *p_defcb = (BYTE *)p_def + offset;

			memcpy(p_givencb, p_defcb, sizeof p_givencb);
		}
	}
}



/*
 * Window procedure for modified tabview control. This function overrides
 * some of the default behavior of the standard tabview control in order
 * to support additional features.
 */
LRESULT CALLBACK mbe_tabview_int_wndproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	switch (msg) {

	}

	/*
	 * Call original tabctrl window procedure to preserve
	 * the rest of the behavior.
	 */
	return glfn_tabviewdefproc(p_hwnd, msg, wparam, lparam);
}


_Critical_ marble_ecode_t mbe_tabview_create(
	_In_              HWND p_hparent,
	_In_              struct mbe_wndsize const *ps_size,
	_In_opt_          struct mbe_tabview_callbacks const *ps_cbs,
	_In_opt_          void *p_crparams,
	_Init_(pps_tview) struct mbe_tabview **pps_tview
) { MB_ERRNO
	if (p_hparent == NULL || pps_tview == NULL || ps_size == NULL) {
		if (pps_tview != NULL)
			*pps_tview = NULL;

		return MARBLE_EC_PARAM;
	}

	BOOL ret;

	/* Allocate memory for view structure. */
	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof **pps_tview,
		true,
		false,
		pps_tview
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* Create tab view window. */
	(*pps_tview)->mp_hwndtab = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		glpz_tabviewwndcl,
		NULL,
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN,
		ps_size->m_xpos,
		ps_size->m_ypos,
		ps_size->m_width,
		ps_size->m_height,
		p_hparent,
		NULL,
		gls_editorapp.mp_hinst,
		NULL
	);
	if ((*pps_tview)->mp_hwndtab == NULL) {
		ecode = MARBLE_EC_CREATEWND;

		goto lbl_END;
	}
	SetWindowFont(
		(*pps_tview)->mp_hwndtab,
		gls_editorapp.ms_res.mp_hguifont,
		TRUE
	);

	/* Obtain tab view window dimensions. */
	mbe_tabview_int_getdisprect((*pps_tview)->mp_hwndtab, &(*pps_tview)->ms_dimensions);

	/* Create page window. */
	(*pps_tview)->mp_hwndpage = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		glpz_pagewndcl,
		NULL,
		WS_CHILD | WS_HSCROLL | WS_VSCROLL,
		(*pps_tview)->ms_dimensions.left,
		(*pps_tview)->ms_dimensions.top,
		(*pps_tview)->ms_dimensions.right - (*pps_tview)->ms_dimensions.left,
		(*pps_tview)->ms_dimensions.bottom - (*pps_tview)->ms_dimensions.top,
		(*pps_tview)->mp_hwndtab,
		NULL,
		gls_editorapp.mp_hinst,
		*pps_tview
	);
	if ((*pps_tview)->mp_hwndpage == NULL) {
		ecode = MARBLE_EC_CREATEWND;

		goto lbl_END;
	}

	/* Create page list. */
	ecode = marble_util_vec_create(
		0,
		(void (MB_CALLBACK *)(void **))&mbe_tabpage_int_destroy,
		&(*pps_tview)->mps_pages
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* Set-up callbacks. */
	mbe_tabview_int_setcbs(
		&(*pps_tview)->ms_cbs,
		ps_cbs,
		&gls_defviewcbs,
		sizeof gls_defviewcbs
	);

	/* Initialize userdata. */
	ret = (*(*pps_tview)->ms_cbs.mpfn_oncreate)(*pps_tview, p_crparams);
	if (ret == FALSE) {
		ecode = MARBLE_EC_USERINIT;

		goto lbl_END;
	}

	/* Set init-state. */
	(*pps_tview)->m_isinit = TRUE;

lbl_END:
	if (ecode != MARBLE_EC_OK) {
		if (*pps_tview == NULL)
			return ecode;

		if ((*pps_tview)->mp_hwndtab != NULL)
			DestroyWindow((*pps_tview)->mp_hwndtab);

		mbe_tabview_destroy(pps_tview);
		return ecode;
	}

	return MARBLE_EC_OK;
}

void mbe_tabview_destroy(
	_Uninit_(pps_tview) struct mbe_tabview **pps_tview
) {
	if (pps_tview == NULL || *pps_tview == NULL)
		return;

	/* Run "ondestroy" handler. */
	(*(*pps_tview)->ms_cbs.mpfn_ondestroy)(*pps_tview);

	/* Destroy loaded pages. */
	marble_util_vec_destroy(&(*pps_tview)->mps_pages);

	free(*pps_tview);
	*pps_tview = NULL;
}

void mbe_tabview_resize(
	_In_ struct mbe_tabview *ps_tview,
	_In_ struct mbe_wndsize const *ps_dims
) {
	if (ps_tview == NULL || ps_dims == NULL)
		return;
	
	/* Set tab-view size and position. */
	MoveWindow(
		ps_tview->mp_hwndtab,
		ps_dims->m_xpos,
		ps_dims->m_ypos,
		ps_dims->m_width,
		ps_dims->m_height,
		TRUE
	);

	/*
	 * Updates the size of the tab-view page window.
	 * This will also update the size of the display rectangle of
	 * the tab-view.
	 */
	mbe_tabview_int_resizepagewnd(ps_tview, TRUE);
}

_Critical_ marble_ecode_t mbe_tabview_newpage(
	_In_              struct mbe_tabview *ps_tview,
	_In_              TCHAR *pz_title,
	_In_opt_          TCHAR *pz_comment,
	_In_opt_          struct mbe_tabpage_callbacks const *ps_cbs,
	_In_opt_          void *p_crparams,
	_Init_(pps_tpage) struct mbe_tabpage **pps_tpage
) { MB_ERRNO
	if (ps_tview == NULL || pps_tpage == NULL) return MARBLE_EC_PARAM;
	if (ps_tview->m_isinit == FALSE)           return MARBLE_EC_COMPSTATE;

	/* Allocate memory for the page. */
	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof **pps_tpage,
		true,
		false,
		pps_tpage
	);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	/* Set common init-data. */
	(*pps_tpage)->ps_parent = ps_tview;
	(*pps_tpage)->mpz_name  = pz_title;
	(*pps_tpage)->mpz_cmt   = pz_comment;
	
	/* Set callbacks and correct them, if necessary. */
	mbe_tabview_int_setcbs(
		&(*pps_tpage)->ms_cbs,
		ps_cbs,
		&gls_defpagecbs,
		sizeof gls_defpagecbs
	);

	/* Execute "oncreate" callback. */
	BOOL ret = (*(*pps_tpage)->ms_cbs.mpfn_oncreate)(*pps_tpage, p_crparams);
	if (ret == FALSE) {
		/*
		 * If the callback returns FALSE, we assume this
		 * is an error, so we destroy our page and pretend
		 * it never happened.
		 */
		mbe_tabpage_int_destroy(pps_tpage);

		return MARBLE_EC_USERINIT;
	}

	/* Update page init-state. */
	(*pps_tpage)->m_isinit = TRUE;

	return MARBLE_EC_OK;
}

marble_ecode_t mbe_tabview_addpage(
	_In_ struct mbe_tabview *ps_tview,
	_In_ struct mbe_tabpage *ps_tpage
) { MB_ERRNO
	if (ps_tview == NULL || ps_tpage == NULL) return MARBLE_EC_PARAM;
	if (ps_tview->m_isinit == FALSE)          return MARBLE_EC_COMPSTATE;

	/* Add page to list. */
	ecode = marble_util_vec_pushback(ps_tview->mps_pages, ps_tpage);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	/* Add new page to tab-view. */
	int last = (int)ps_tview->mps_pages->m_size - 1;
	TCITEM s_item = {
		.mask    = TCIF_TEXT,
		.pszText = ps_tpage->mpz_name
	};
	TabCtrl_InsertItem(ps_tview->mp_hwndtab, last, &s_item);

	/* Update selection of tab-view. */
	ps_tview->mps_cursel = ps_tpage;
	TabCtrl_SetCurSel(ps_tview->mp_hwndtab, last);

	/*
	 * Resize page window if its the first page to be added.
	 * This has to be done because the display area shrinks
	 * if the first tab is added.
	 */
	if (last == 0) {
		mbe_tabview_int_resizepagewnd(ps_tview, TRUE);
	
		/*
		 * The first added page also causes the page
		 * window to finally show.
		 */
		ShowWindow(ps_tview->mp_hwndpage, SW_SHOW);
	}

	/* Force update of page window. */
	InvalidateRect(ps_tview->mp_hwndpage, NULL, FALSE);

	return MARBLE_EC_OK;
}
#pragma endregion


