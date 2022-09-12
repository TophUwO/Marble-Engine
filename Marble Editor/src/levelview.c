#include <editor.h>
#include <level.h>


/*
 * Structure representing a level in the editor.
 * 
 * Used directly as the userdata field in a
 * tab-page.
 */
struct mbe_level {
	int m_twidth;  /* width, in tiles */
	int m_theight; /* height, in tiles */
	int m_pwidth;  /* physical width, in pixels */
	int m_pheight; /* physical height, in pixels */

	/*
	 * Marble asset representing the actual level data.
	 * See "level.h" in Marble Engine for more information.
	 */
	struct marble_levelasset *ps_asset;
};

/*
 * Structure representing tab-view userdata
 */
struct mbe_levelview {
	ID2D1RenderTarget    *mp_rt;       /* page-window rendertarget */
	ID2D1SolidColorBrush *mp_brblack;  /* solid black brush */
};

/*
 * Userdata for the "Create new level" dialog. 
 */
struct mbe_levelview_dlgnewlvl_udata {
	TCHAR *pz_name; /* name of tileset */
	TCHAR *pz_cmt;  /* opt. comment/description */

	int m_width;    /* width of the new level, in tiles */
	int m_height;   /* height of the new level, in tiles */
};


/*
 * The size of a tile on screen, in pixels. Tiles of a
 * different original size will be scaled-up/-down to
 * this size.
 */
static int const gl_viewtsize = 32;


#pragma region LEVEL
/*
 * Destroys the editor level structure.
 * The underlying Marble level structure is
 * also destroyed.
 * 
 * Returns nothing.
 */
static void mbe_levelview_int_destroylvl(
	_Uninit_(pps_lvl) struct mbe_level **pps_lvl /* level to destroy */
) {
	/* Destroy level asset. */
	marble_levelasset_destroy(&(*pps_lvl)->ps_asset);

	/* Destroy struct memory. */
	free(*pps_lvl);
	*pps_lvl = NULL;
}

/*
 * Creates a new and empty editor level, including an empty
 * Marble level.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ static marble_ecode_t mbe_levelview_int_newlvl(
	_In_            struct mbe_levelview_dlgnewlvl_udata *ps_crparams, /* create-params */
	/*
	 * pointer to a pointer to receive the
	 * newly-created editor level
	 */
	_Init_(pps_lvl) struct mbe_level **pps_lvl
) { MB_ERRNO
	if (ps_crparams == NULL || pps_lvl == NULL)
		return MARBLE_EC_INTERNALPARAM;

	/* Allocate memory for the level. */
	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof **pps_lvl,
		false,
		false,
		pps_lvl
	);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	/* Calculate level metrics. */
	(*pps_lvl)->m_twidth  = ps_crparams->m_width;
	(*pps_lvl)->m_theight = ps_crparams->m_height;
	(*pps_lvl)->m_pwidth  = (*pps_lvl)->m_twidth * gl_viewtsize;
	(*pps_lvl)->m_pheight = (*pps_lvl)->m_theight * gl_viewtsize;

	/* Create Marble level asset. */
	ecode = marble_levelasset_new(
		(*pps_lvl)->m_twidth,
		(*pps_lvl)->m_theight,
		&(*pps_lvl)->ps_asset
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* Add a texture layer to the level. */
	ecode = marble_levelasset_addlayer(
		(*pps_lvl)->ps_asset,
		MARBLE_LVLLAYERTYPE_TEXTURE
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

lbl_END:
	if (ecode != MARBLE_EC_OK)
		mbe_levelview_int_destroylvl(pps_lvl);

	return ecode;
}
#pragma endregion


#pragma region LEVELVIEW
/*
 * Creates the userdata of the tab-view.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
BOOL MB_CALLBACK mbe_levelview_int_ontviewcreate(
	_Inout_  struct mbe_tabview *ps_tview, /* tab-view to init userdata of */
	_In_opt_ void *p_crparams              /* optional create-params */
) { MB_ERRNO
	UNREFERENCED_PARAMETER(p_crparams);

	if (ps_tview == NULL)
		return FALSE;

	HRESULT hres;
	struct mbe_levelview *ps_lvlview = NULL;

	/* Allocate userdata memory. */
	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof *ps_lvlview,
		false,
		false,
		&ps_lvlview
	);
	if (ecode != MARBLE_EC_OK)
		return FALSE;

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
		gls_editorapp.ms_res.mps_d2dfac,
		&s_props,
		(ID2D1DCRenderTarget **)&ps_lvlview->mp_rt
	);
	if (hres != S_OK)
		return FALSE;

	/*
	 * Create a brush initially painting
	 * in solid black.
	 */
	D2D1_COLOR_F const s_cblack = { 0.0f, 0.0f, 0.0f, 1.0f };
	D2D1_BRUSH_PROPERTIES const s_brprops = { 1.0f };
	hres = D2DWr_RenderTarget_CreateSolidColorBrush(
		ps_lvlview->mp_rt,
		&s_cblack,
		&s_brprops,
		&ps_lvlview->mp_brblack
	);
	if (hres != S_OK)
		return FALSE;

	/* Update internal userdata pointer. */
	ps_tview->mp_udata = ps_lvlview;
	return TRUE;
}

/*
 * Destroys the userdata of the tab-view.
 * The function also has to deallocate the memory used
 * by the userdata structure itself.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
BOOL MB_CALLBACK mbe_levelview_int_ontviewdestroy(
	_Inout_ struct mbe_tabview *ps_tview /* tab-view to destroy userdata of */
) {
	if (ps_tview == NULL)
		return FALSE;

	struct mbe_levelview *ps_udata = (struct mbe_levelview *)ps_tview->mp_udata;
	if (ps_udata == NULL)
		return TRUE;

	/* Release the rendertarget. */
	if (ps_udata->mp_rt != NULL)
		D2DWr_RenderTarget_Release(ps_udata->mp_rt);

	if (ps_udata->mp_brblack != NULL)
		D2DWr_SolidColorBrush_Release(ps_udata->mp_brblack);

	/* Free struct memory. */
	free(ps_tview->mp_udata);
	return TRUE;
}
#pragma endregion


#pragma region PAGE-CALLBACKS
/*
 * This handler is called after a new page was created by a
 * tab-view and the page's userdata structure must be
 * initialized.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
static BOOL MB_CALLBACK mbe_levelview_int_onpagecreate(
	_Inout_  struct mbe_tabpage *ps_tpage, /* page to create userdata of */
	_In_opt_ void *p_crparams              /* opt. create-params */
) {
	if (ps_tpage == NULL)
		return FALSE;

	/* Create the level structure. */
	if (p_crparams != NULL)
		if (mbe_levelview_int_newlvl(
			(struct mbe_levelview_dlgnewlvl_udata *)p_crparams,
			(struct mbe_level **)&ps_tpage->mp_udata
		) != MARBLE_EC_OK) return FALSE;

	/* Add the page to the view. */
	return 
		mbe_tabview_addpage(
			ps_tpage->ps_parent,
			ps_tpage
		) == MARBLE_EC_OK;
}

/*
 * Renders a view of the level associated with the
 * currently-selected page.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
static BOOL MB_CALLBACK mbe_levelview_int_onpagepaint(
	_Inout_ struct mbe_tabpage *ps_tpage /* page to render */
) {
	if (ps_tpage == NULL || ps_tpage->m_isinit == FALSE)
		return FALSE;

	HDC p_hdc;
	RECT s_clrect;
	int xori, yori;
	struct mbe_levelview *ps_res;

	/*
	 * Get the pointer to the structure holding
	 * the required Direct2D resources.
	 */
	ps_res = (struct mbe_levelview *)ps_tpage->ps_parent->mp_udata;

	/* Render the content of the page. */
	p_hdc = GetDC(ps_tpage->ps_parent->mp_hwndpage);
	if (p_hdc == NULL)
		return FALSE;
	GetClientRect(ps_tpage->ps_parent->mp_hwndpage, &s_clrect);

	D2DWr_DCRenderTarget_BindDC((ID2D1DCRenderTarget *)ps_res->mp_rt, p_hdc, &s_clrect);
	D2DWr_RenderTarget_BeginDraw(ps_res->mp_rt);

	/* Erase screen with solid white. */
	D2D1_COLOR_F s_col = { 1.0f, 1.0f, 1.0f, 1.0f };
	D2DWr_RenderTarget_Clear(ps_res->mp_rt, &s_col);

	// TODO: add tile-rendering code
	xori = -ps_tpage->ms_scrinfo.ms_xscr.nPos % gl_viewtsize;
	yori = -ps_tpage->ms_scrinfo.ms_yscr.nPos % gl_viewtsize;

	/* Draw vertical grid lines. */
	for (int x = xori; x < s_clrect.right; x += gl_viewtsize)
		D2DWr_RenderTarget_DrawLine(
			ps_res->mp_rt,
			(D2D1_POINT_2F){ (float)x - 0.5f, (float)yori },
			(D2D1_POINT_2F){ (float)x - 0.5f, (float)s_clrect.bottom },
			(ID2D1Brush *)ps_res->mp_brblack,
			1.0f,
			/*
			 * Due to massive performance issues, no special stroke style
			 * is currently used. 
			 */
			NULL
		);

	/* Draw horizontal grid lines. */
	for (int y = yori; y < s_clrect.bottom; y += gl_viewtsize)
		D2DWr_RenderTarget_DrawLine(
			ps_res->mp_rt,
			(D2D1_POINT_2F){ (float)xori, (float)y - 0.5f },
			(D2D1_POINT_2F){ (float)s_clrect.right, (float)y - 0.5f },
			(ID2D1Brush *)ps_res->mp_brblack,
			1.0f,
			NULL
		);

	/* Present the frame. */
	D2DWr_RenderTarget_EndDraw(ps_res->mp_rt, NULL, NULL);

	ReleaseDC(ps_tpage->ps_parent->mp_hwndpage, p_hdc);
	return TRUE;
}

/*
 * Carries out operations that are supposed to happen on resize,
 * such as scrollbar range updates etc.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
static BOOL MB_CALLBACK mbe_levelview_int_onpageresize(
	_Inout_ struct mbe_tabpage *ps_tpage, /* tab-page */
	        int nwidth,                   /* new width of tab-page window */
	        int nheight                   /* new height of tab-page window */
) {
	if (ps_tpage == NULL)
		return FALSE;

	int maxscr;
	BOOL isxvisible, isyvisible;
	struct mbe_level *ps_lvl = (struct mbe_level *)ps_tpage->mp_udata;
	if (ps_lvl == NULL)
		return FALSE;

	/* Update vertical scrollbar. */
	if (ps_lvl->m_pheight > nheight) {
		maxscr = max(ps_lvl->m_pheight - nheight, 0); 

		ps_tpage->ms_scrinfo.ms_yscr = (SCROLLINFO){
			.cbSize = sizeof ps_tpage->ms_scrinfo.ms_yscr,
			.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS,
			.nMin   = 0,
			.nMax   = ps_lvl->m_pheight - 1,
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
	if (ps_lvl->m_pwidth > nwidth) {
		maxscr = max(ps_lvl->m_pwidth - nwidth, 0);

		ps_tpage->ms_scrinfo.ms_xscr = (SCROLLINFO){
			.cbSize = sizeof ps_tpage->ms_scrinfo.ms_xscr,
			.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS,
			.nMin   = 0,
			.nMax   = ps_lvl->m_pwidth - 1,
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

/*
 * Destroys the userdata of a tab-page holding a level object.
 * 
 * Returns nothing.
 */
static BOOL MB_CALLBACK mbe_levelview_int_onpagedestroy(
	_Inout_ struct mbe_tabpage *ps_tpage /* page to destroy userdata of */
) {
	if (ps_tpage == NULL)
		return FALSE;

	/* Destroy level structure. */
	mbe_levelview_int_destroylvl((struct mbe_level **)&ps_tpage->mp_udata);

	return TRUE;
}
#pragma endregion


#pragma region DLG-NEWLEVEL
/*
 * Executed when the user selects the OK button of the "Create new level"
 * dialog.
 * This callback is used to validate the dialog input and notify the
 * user in case there is an issue.
 * 
 * Returns TRUE if the data is valid, FALSE is there is an issue, prompting
 * the user to review their input.
 */
static BOOL MB_CALLBACK mbe_levelview_newlvldlg_onok(
	_In_     HWND p_hwnd,  /* dialog window*/
	_In_opt_ void *p_udata /* dialog userdata */
) {
	if (p_hwnd == NULL || p_udata == NULL)
		return FALSE;

	struct mbe_levelview_dlgnewlvl_udata *ps_udata = (struct mbe_levelview_dlgnewlvl_udata *)p_udata;

	if (ps_udata->pz_name == NULL || *ps_udata->pz_name == TEXT('\0')) {
		MB_LOG_WARN("No (valid) level name was entered.", 0);

		MessageBox(
			p_hwnd,
			TEXT("You must enter a level name."),
			TEXT("Error"),
			MB_ICONWARNING | MB_OK
		);

		return FALSE;
	}

	return TRUE;
}

static struct mbe_dlg_ctrlinfo const glsa_newlvlctrlinfo[] = {
	{ NewLvlDlg_EDIT_Name,    MBE_DLGCTRLTYPE_EDIT,    offsetof(struct mbe_levelview_dlgnewlvl_udata, pz_name),  0,           MBE_DLGCTRLFLAG_INITIALFOCUS, ._edit   = TEXT("unnamed")                                         },
	{ NewLvlDlg_EDIT_Comment, MBE_DLGCTRLTYPE_EDIT,    offsetof(struct mbe_levelview_dlgnewlvl_udata, pz_cmt)                                                                                                                  },
	{ NewLvlDlg_SPIN_Width,   MBE_DLGCTRLTYPE_SPINBOX, offsetof(struct mbe_levelview_dlgnewlvl_udata, m_width),  sizeof(int), 0,                            ._spin   = { NewLvlDlg_EDIT_Width,  64, 1, 1 << 16               } },
	{ NewLvlDlg_SPIN_Height,  MBE_DLGCTRLTYPE_SPINBOX, offsetof(struct mbe_levelview_dlgnewlvl_udata, m_height), sizeof(int), 0,                            ._spin   = { NewLvlDlg_EDIT_Height, 64, 1, 1 << 16               } },
	{ NewLvlDlg_BTN_Ok,       MBE_DLGCTRLTYPE_BUTTON,  0,                                                        0,           0,                            ._button = { 0, MBE_DLGBTNFLAG_OK, &mbe_levelview_newlvldlg_onok } },
	{ NewLvlDlg_BTN_Cancel,   MBE_DLGCTRLTYPE_BUTTON,  0,                                                        0,           0,                            ._button = { 0, MBE_DLGBTNFLAG_CANCEL                            } },
	{ NewLvlDlg_BTN_Reset,    MBE_DLGCTRLTYPE_BUTTON,  0,                                                        0,           0,                            ._button = { 0, MBE_DLGBTNFLAG_RESET                             } }
};


BOOL mbe_levelview_newlvlbydlg(
	_In_ struct mbe_tabview *ps_tview
) {
	if (ps_tview == NULL || ps_tview->m_isinit == FALSE)
		return FALSE;

	struct mbe_tabpage *ps_tpage;
	struct mbe_levelview_dlgnewlvl_udata s_udata;
	BOOL res;

	/* Set dialog attributes. */
	struct mbe_dlginfo const s_info = {
		.mp_hparent   = gls_editorapp.mp_hwnd,
		.map_ctrlinfo = glsa_newlvlctrlinfo,
		.m_nctrlinfo  = ARRAYSIZE(glsa_newlvlctrlinfo),
		.mp_udata     = &s_udata,
		.m_udatasz    = sizeof s_udata,
		.m_templ      = MBE_DLG_NewLvl
	};
	struct mbe_tabpage_callbacks const s_cbs = {
		.mpfn_oncreate  = &mbe_levelview_int_onpagecreate,
		.mpfn_onpaint   = &mbe_levelview_int_onpagepaint,
		.mpfn_onresize  = &mbe_levelview_int_onpageresize,
		.mpfn_ondestroy = &mbe_levelview_int_onpagedestroy
	};

	/* Run the dialog. */
	res = mbe_dialog_dodialog(&s_info);
	if (res == FALSE)
		return FALSE;

	/*
	 * Create a new page.
	 * The page is automatically added to the tab-view
	 * through the "oncreate" callback.
	 */
	res = mbe_tabview_newpage(
		ps_tview,
		s_udata.pz_name,
		s_udata.pz_cmt,
		&s_cbs,
		&s_udata,
		&ps_tpage
	);
	if (res == TRUE)
		MB_LOG_ERROR("Failed to create tab-view page.", 0);

	return res;
}
#pragma endregion


