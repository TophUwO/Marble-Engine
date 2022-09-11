#include <editor.h>


WNDPROC glfn_tabviewdefproc = NULL;
TCHAR const *const glpz_tabviewwndcl = TEXT("mbe_ctrl_tabview");
TCHAR const *const glpz_pagewndcl    = TEXT("mbe_ctrl_tabpage");


#pragma region TABPAGE-CTRL
#define MBE_TVIEW_FIXCB(member, def) (void)(member == NULL ? (member = &def) : (0))


/* default tab-view callbacks */
static BOOL MB_CALLBACK mbe_tabview_int_defoncreate(_In_ struct mbe_tabview *ps_tview, _In_opt_ void *p_crparams) { return TRUE; }
static BOOL MB_CALLBACK mbe_tabview_int_defondestroy(_In_ struct mbe_tabview *ps_tview) { return TRUE; }

/* default tab-page callbacks */
static BOOL MB_CALLBACK mbe_pageview_int_defoncreate(_In_ struct mbe_tabpage *ps_tpage, _In_opt_ void *p_crparams) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defonpaint(_In_ struct mbe_tabpage *ps_tpage) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defonresize(_In_ struct mbe_tabpage *ps_tpage, int nwidth, int nheight) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defonselect(_In_ struct mbe_tabpage *ps_tpage) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defonunselect(_In_ struct mbe_tabpage *ps_tpage) { return TRUE; }
static BOOL MB_CALLBACK mbe_pageview_int_defondestroy(_In_ struct mbe_tabpage *ps_tpage) { return TRUE; }

/* default tab-view callback struct */
static struct mbe_tabview_callbacks const gls_defviewcbs = {
	&mbe_tabview_int_defoncreate,
	&mbe_tabview_int_defondestroy
};

/* default tab-page callback struct */
static struct mbe_tabpage_callbacks const gls_defpagecbs = {
	&mbe_pageview_int_defoncreate,
	&mbe_pageview_int_defonpaint,
	&mbe_pageview_int_defonresize,
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
			/* Execute "onresize" handler. */
			(*ps_udata->mps_cursel->ms_cbs.mpfn_onresize)(
				ps_udata->mps_cursel,
				GET_X_LPARAM(lparam),
				GET_Y_LPARAM(lparam)
			);

			/* Post a WM_PAINT message. */
			InvalidateRect(p_hwnd, NULL, FALSE);
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
 * Resizes the page window of the tab-view. It also recalculates the
 * coordinates of the display rect.
 * 
 * Returns nothing.
 */
static void mbe_tabview_int_resizepagewnd(_In_ struct mbe_tabview *ps_tview /* tabview to resize page window of */) {
	if (ps_tview == NULL)
		return;

	/* Get dimensions of display area. */
	mbe_tabview_int_getdisprect(ps_tview->mp_hwndtab, &ps_tview->ms_dimensions);

	/*
	 * Set position and dimensions to exactly reflect the
	 * size of the display area of the tab-view.
	 */
	MoveWindow(
		ps_tview->mp_hwndpage,
		ps_tview->ms_dimensions.left,
		ps_tview->ms_dimensions.top,
		ps_tview->ms_dimensions.right - ps_tview->ms_dimensions.left,
		ps_tview->ms_dimensions.bottom - ps_tview->ms_dimensions.top,
		TRUE
	);
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
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
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
		WS_CHILD | WS_CLIPSIBLINGS,
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
	mbe_tabview_int_resizepagewnd(ps_tview);
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
	if (last == 0)
		mbe_tabview_int_resizepagewnd(ps_tview);

	/* Force update of page window. */
	InvalidateRect(ps_tview->mp_hwndpage, NULL, FALSE);

	return MARBLE_EC_OK;
}
#pragma endregion


