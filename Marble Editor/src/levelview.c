#include <internal/shared.h>

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
 * Structure representing userdata for the tab-view
 * control acting as the levelview. 
 */
struct mbe_levelview {
	struct mbe_tabview_udata _base; /* common tab-view userdata */
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
static int const gl_defviewtsize = 32;


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
	_In_            struct mbe_tabpage *ps_tpage,                      /* tab-page */
	_In_            struct mbe_levelview_dlgnewlvl_udata *ps_crparams, /* create-params */
	/*
	 * pointer to a pointer to receive the
	 * newly-created editor level
	 */
	_Init_(pps_lvl) struct mbe_level **pps_lvl
) { MB_ERRNO
	if (ps_crparams == NULL || pps_lvl == NULL)
		return MARBLE_EC_INTERNALPARAM;

	struct mbe_levelview *ps_lvludata = (struct mbe_levelview *)ps_tpage->ps_parent->mp_udata;
	if (ps_lvludata == NULL)
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
	(*pps_lvl)->m_pwidth  = (*pps_lvl)->m_twidth * (int)ceilf(gl_defviewtsize * ps_lvludata->_base.m_zoom);
	(*pps_lvl)->m_pheight = (*pps_lvl)->m_theight * (int)ceilf(gl_defviewtsize * ps_lvludata->_base.m_zoom);

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
) {
	return 
		mbe_tabview_inl_oncreate_common(
			ps_tview,
			p_crparams
		);
}

BOOL MB_CALLBACK mbe_levelview_int_ontviewresize(
	_Inout_ struct mbe_tabview *ps_tview, /* tab-view */
	        int nwidth,                   /* new width of page window, in pixels */
	        int nheight                   /* new height of page window, in pixels */
) {
	return
		mbe_tabview_inl_onresize_common(
			ps_tview,
			nwidth,
			nheight
		);
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
	return
		mbe_tabview_inl_ondestroy_common(
			ps_tview
		);
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
			ps_tpage,
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

	int xori, yori, tx, ty;
	struct mbe_level *ps_udata;
	struct mbe_levelview *ps_res;
	D2D1_ANTIALIAS_MODE oldaamode;

	/*
	 * Get the pointers to the structures holding
	 * the required resources.
	 */
	ps_udata = (struct mbe_level *)ps_tpage->mp_udata;
	ps_res   = (struct mbe_levelview *)ps_tpage->ps_parent->mp_udata;

	/* Calculate scaled view-tilesize. */
	int const vtsize = (int)ceilf(gl_defviewtsize * ps_res->_base.m_zoom);

	/* Render the content of the page. */
	D2DWr_RenderTarget_BeginDraw(ps_res->_base.mp_rt);

	/* Erase screen with solid white. */
	D2D1_COLOR_F s_col = { 1.0f, 1.0f, 1.0f, 1.0f };
	D2DWr_RenderTarget_Clear(ps_res->_base.mp_rt, &s_col);

	/*
	 * Compute the upper-left corner of topleft
	 * tile that is at least partly visible,
	 * in client coordinates.
	 * 
	 * **tx** and **ty** form the origin of the
	 * current view, in tile coordinates.
	 */
	tx   = ps_tpage->ms_scrinfo.ms_xscr.nPos / vtsize;
	ty   = ps_tpage->ms_scrinfo.ms_yscr.nPos / vtsize;
	xori = -ps_tpage->ms_scrinfo.ms_xscr.nPos % vtsize;
	yori = -ps_tpage->ms_scrinfo.ms_yscr.nPos % vtsize;

	/* Render currently visible view of the level. */
	/*for (int x = xori; x < ps_res->_base.ms_clrect.right; x += vtsize) {
		for (int y = yori; y < ps_res->_base.ms_clrect.bottom; y += vtsize) {
			if (tx == 0 && ty == 0 || tx == 0 && ty == ps_udata->m_theight-1 || tx == ps_udata->m_twidth-1 && ty == 0 || tx == ps_udata->m_twidth-1 && ty == ps_udata->m_theight-1) {
				D2D1_RECT_F const s_drect = {
					(float)x,
					(float)y,
					(float)x + vtsize,
					(float)y + vtsize
				};

				D2DWr_RenderTarget_FillRectangle(
					ps_res->_base.mp_rt,
					&s_drect,
					ps_res->_base.mp_brsolid
				);
			}

			++ty;
		}

		ty = ps_tpage->ms_scrinfo.ms_yscr.nPos / vtsize;
		++tx;
	}*/

	/*
	 * Disable antialiasing for straight lines.
	 * This speeds-up drawing of non-solid
	 * lines considerably.
	 */
	oldaamode = D2DWr_RenderTarget_GetAntialiasMode(ps_res->_base.mp_rt);
	D2DWr_RenderTarget_SetAntialiasMode(ps_res->_base.mp_rt, D2D1_ANTIALIAS_MODE_ALIASED);

	/* Draw vertical grid lines. */
	for (int x = xori; x < ps_res->_base.ms_clrect.right; x += vtsize)
		D2DWr_RenderTarget_DrawLine(
			ps_res->_base.mp_rt,
			(D2D1_POINT_2F){ (float)x - 0.5f, (float)yori - 0.5f },
			(D2D1_POINT_2F){ (float)x - 0.5f, (float)ps_res->_base.ms_clrect.bottom + 0.5f },
			(ID2D1Brush *)ps_res->_base.mp_brsolid,
			1.0f,
			/*
			 * Due to massive performance issues, no special stroke style
			 * is currently used. 
			 */
			NULL
		);

	/* Draw horizontal grid lines. */
	for (int y = yori; y < ps_res->_base.ms_clrect.bottom; y += vtsize)
		D2DWr_RenderTarget_DrawLine(
			ps_res->_base.mp_rt,
			(D2D1_POINT_2F){ (float)xori - 0.5f, (float)y - 0.5f },
			(D2D1_POINT_2F){ (float)ps_res->_base.ms_clrect.right + 0.5f, (float)y - 0.5f },
			(ID2D1Brush *)ps_res->_base.mp_brsolid,
			1.0f,
			NULL
		);

	/* Restore the old AA-mode. */
	D2DWr_RenderTarget_SetAntialiasMode(ps_res->_base.mp_rt, oldaamode);

	/* Present the frame. */
	D2DWr_RenderTarget_EndDraw(ps_res->_base.mp_rt, NULL, NULL);
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

	struct mbe_level *ps_udata = (struct mbe_level *)ps_tpage->mp_udata;
	if (ps_udata == NULL)
		return TRUE;

	return
		mbe_tabpage_inl_onresize_common(
			ps_tpage,
			nwidth,
			nheight,
			ps_udata->m_pwidth,
			ps_udata->m_pheight
		);
}

/*
 * Updates any level-specific userdata in the event
 * of a page-window zoom event.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
static BOOL MB_CALLBACK mbe_levelview_int_onpagezoom(
	_Inout_ struct mbe_tabpage *ps_tpage /* tab-page */
) {
	if (ps_tpage == NULL)
		return FALSE;

	struct mbe_level *ps_lvludata  = (struct mbe_level *)ps_tpage->mp_udata;
	struct mbe_levelview *ps_udata = (struct mbe_levelview *)ps_tpage->ps_parent->mp_udata;
	if (ps_lvludata == NULL || ps_udata == NULL)
		return FALSE;

	/* Update physical content dimensions. */
	ps_lvludata->m_pwidth  = ps_lvludata->m_twidth * (int)ceilf(gl_defviewtsize * ps_udata->_base.m_zoom);
	ps_lvludata->m_pheight = ps_lvludata->m_theight * (int)ceilf(gl_defviewtsize * ps_udata->_base.m_zoom);

	/*
	 * Call the "onresize" handler with the current client area
	 * dimensions and the new content dimensions, causing the
	 * scrollbars to update, reflecting the new window-content
	 * size ratio.
	 */
	return
		mbe_tabpage_inl_onresize_common(
			ps_tpage,
			ps_udata->_base.ms_clrect.right,
			ps_udata->_base.ms_clrect.bottom,
			ps_lvludata->m_pwidth,
			ps_lvludata->m_pheight
		);
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
	_In_     HWND p_hwnd,  /* dialog window */
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

/* dialog control information */
static struct mbe_dlg_ctrlinfo const glsa_newlvlctrlinfo[] = {
	{ NewLvlDlg_EDIT_Name,
		MBE_DLGCTRLTYPE_EDIT,
		offsetof(struct mbe_levelview_dlgnewlvl_udata, pz_name),
		0,
		MBE_DLGCTRLFLAG_INITIALFOCUS,
		._edit = TEXT("unnamed")
	},
	{ NewLvlDlg_EDIT_Comment,
		MBE_DLGCTRLTYPE_EDIT,
		offsetof(struct mbe_levelview_dlgnewlvl_udata, pz_cmt)
	},
	{ NewLvlDlg_SPIN_Width,
		MBE_DLGCTRLTYPE_SPINBOX,
		offsetof(struct mbe_levelview_dlgnewlvl_udata, m_width),
		sizeof(int),
		._spin = { 
			NewLvlDlg_EDIT_Width,
			64,
			1,
			0xFFFF
		}
	},
	{ NewLvlDlg_SPIN_Height,
		MBE_DLGCTRLTYPE_SPINBOX,
		offsetof(struct mbe_levelview_dlgnewlvl_udata, m_height),
		sizeof(int),
		._spin = {
			NewLvlDlg_EDIT_Height,
			64,
			1,
			0xFFFF
		} 
	},
	{ NewLvlDlg_BTN_Ok,
		MBE_DLGCTRLTYPE_BUTTON,
		._button = { 
			0,
			MBE_DLGBTNFLAG_OK,
			&mbe_levelview_newlvldlg_onok 
		} 
	},
	{ NewLvlDlg_BTN_Cancel,
		MBE_DLGCTRLTYPE_BUTTON,
		._button = {
			0,
			MBE_DLGBTNFLAG_CANCEL
		}
	},
	{ NewLvlDlg_BTN_Reset,
		MBE_DLGCTRLTYPE_BUTTON,
		._button = {
			0,
			MBE_DLGBTNFLAG_RESET
		} 
	}
};


/*
 * Shows a dialog that allows the user to enter
 * metrics for a new level.
 * 
 * Returns TRUE if the dialog ended successfully,
 * FALSE if not.
 */
BOOL mbe_levelview_newlvlbydlg(
	_In_ struct mbe_tabview *ps_tview /* level-view */
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
		.mpfn_onzoom    = &mbe_levelview_int_onpagezoom,
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


