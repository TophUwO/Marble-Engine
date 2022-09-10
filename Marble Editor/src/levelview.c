#include <editor.h>
#include <level.h>


struct mbe_level {
	int m_twidth;
	int m_theight;
	int m_pwidth;
	int m_pheight;

	struct marble_levelasset *ps_asset;
};

struct mbe_levelview_dlgnewlvl_udata {
	TCHAR *pz_name;
	TCHAR *pz_cmt;

	int m_width;
	int m_height;
};


/*
 * The size of a tile on screen, in pixels. Tiles of a
 * different original size will be scaled-up/-down to
 * this size.
 */
static int const gl_viewtsize = 32;


#pragma region LEVEL
static void mbe_levelview_int_destroylvl(
	_Uninit_(pps_lvl) struct mbe_level **pps_lvl /* level to destroy */
) {
	/* Destroy level asset. */
	marble_levelasset_destroy(&(*pps_lvl)->ps_asset);

	/* Destroy struct memory. */
	free(*pps_lvl);
	*pps_lvl = NULL;
}

_Critical_ static marble_ecode_t mbe_levelview_int_newlvl(
	_In_            struct mbe_levelview_dlgnewlvl_udata *ps_crparams,
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

lbl_END:
	if (ecode != MARBLE_EC_OK)
		mbe_levelview_int_destroylvl(pps_lvl);

	return ecode;
}
#pragma endregion


#pragma region LEVELVIEW
// TODO: ADD oncreate handlers
#pragma endregion


#pragma region PAGE-CALLBACKS
static BOOL MB_CALLBACK mbe_levelview_int_oncreate(
	_In_     struct mbe_tabpage *ps_tpage,
	_In_opt_ void *p_crparams
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

static BOOL MB_CALLBACK mbe_levelview_int_onresize(
	_In_ struct mbe_tabpage *ps_tpage,
	     int nwidth,
	     int nheight
) {
	return TRUE;
}

/*
 * Destroys the userdata of a tab-page holding a level object.
 * 
 * Returns nothing.
 */
static BOOL MB_CALLBACK mbe_levelview_int_ondestroy(
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
static BOOL MB_CALLBACK mbe_levelview_newlvldlg_onok(HWND p_hwnd, void *p_udata) {
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
	{ NewLvlDlg_EDIT_Name,    MBE_DLGCTRLTYPE_EDIT,    offsetof(struct mbe_levelview_dlgnewlvl_udata, pz_name),  0,           TRUE, ._edit   = TEXT("unnamed")                                         },
	{ NewLvlDlg_EDIT_Comment, MBE_DLGCTRLTYPE_EDIT,    offsetof(struct mbe_levelview_dlgnewlvl_udata, pz_cmt),   0,           TRUE                                                                     },
	{ NewLvlDlg_SPIN_Width,   MBE_DLGCTRLTYPE_SPINBOX, offsetof(struct mbe_levelview_dlgnewlvl_udata, m_width),  sizeof(int), TRUE, ._spin   = { NewLvlDlg_EDIT_Width,  64, 1, 1 << 16               } },
	{ NewLvlDlg_SPIN_Height,  MBE_DLGCTRLTYPE_SPINBOX, offsetof(struct mbe_levelview_dlgnewlvl_udata, m_height), sizeof(int), TRUE, ._spin   = { NewLvlDlg_EDIT_Height, 64, 1, 1 << 16               } },
	{ NewLvlDlg_BTN_Ok,       MBE_DLGCTRLTYPE_BUTTON,  0,                                                        0,           TRUE, ._button = { 0, MBE_DLGBTNFLAG_OK, &mbe_levelview_newlvldlg_onok } },
	{ NewLvlDlg_BTN_Cancel,   MBE_DLGCTRLTYPE_BUTTON,  0,                                                        0,           TRUE, ._button = { 0, MBE_DLGBTNFLAG_CANCEL                            } },
	{ NewLvlDlg_BTN_Reset,    MBE_DLGCTRLTYPE_BUTTON,  0,                                                        0,           TRUE, ._button = { 0, MBE_DLGBTNFLAG_RESET                             } }
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
		.mpfn_oncreate  = &mbe_levelview_int_oncreate,
		.mpfn_ondestroy = &mbe_levelview_int_ondestroy
	};

	/* Run the dialog. */
	res = mbe_dialog_dodialog(&s_info);
	if (res == FALSE)
		return FALSE;

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


