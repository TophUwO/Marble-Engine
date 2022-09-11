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
// TODO: ADD oncreate handlers
#pragma endregion


#pragma region PAGE-CALLBACKS
/*
 * This handler is called after a new page was created by a
 * tab-view and the page's userdata structure must be
 * initialized.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
static BOOL MB_CALLBACK mbe_levelview_int_oncreate(
	_In_     struct mbe_tabpage *ps_tpage, /* page to create userdata of */
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
static BOOL MB_CALLBACK mbe_levelview_int_onpaint(
	_In_ struct mbe_tabpage *ps_tpage /* page to render */
) {
	if (ps_tpage == NULL || ps_tpage->m_isinit == FALSE)
		return FALSE;

	// TODO: add render-code

	return TRUE;
}

/*
 * Carries out operations that are supposed to happen on resize,
 * such as scrollbar range updates etc.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
static BOOL MB_CALLBACK mbe_levelview_int_onresize(
	_In_ struct mbe_tabpage *ps_tpage, /* tab-page */
	     int nwidth,                   /* new width of tab-page window */
	     int nheight                   /* new height of tab-page window */
) {
	if (ps_tpage == NULL)
		return FALSE;

	// TODO: add resize-code

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
		.mpfn_oncreate  = &mbe_levelview_int_oncreate,
		.mpfn_onpaint   = &mbe_levelview_int_onpaint,
		.mpfn_ondestroy = &mbe_levelview_int_ondestroy
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


