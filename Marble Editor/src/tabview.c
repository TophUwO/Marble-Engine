#include <editor.h>


WNDPROC glfn_tabviewdefproc = NULL;
TCHAR const *const glpz_tabviewwndcl = TEXT("mbe_ctrl_tabview");
TCHAR const *const glpz_pagewndcl    = TEXT("mbe_ctrl_tabpage");


#pragma region TABPAGE-CTRL
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
	switch (msg) {

	}

	return DefWindowProc(p_hwnd, msg, wparam, lparam);
}
#pragma region


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
	_Init_(pps_tview) struct mbe_tabview **pps_tview
) { MB_ERRNO
	if (p_hparent == NULL || pps_tview == NULL || ps_size == NULL) {
		if (pps_tview != NULL)
			*pps_tview = NULL;

		return MARBLE_EC_PARAM;
	}

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
		NULL
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

	/* Update display area metrics. */
	mbe_tabview_int_getdisprect(ps_tview->mp_hwndtab, &ps_tview->ms_dimensions);
}
#pragma endregion


