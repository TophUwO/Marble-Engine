#include <editor.h>


#pragma region PAGE-CALLBACKS
static BOOL MB_CALLBACK mbe_levelview_int_oncreate(_In_ struct mbe_tabpage *ps_tpage) {
	if (ps_tpage == NULL)
		return FALSE;

	ps_tpage->mp_udata = NULL;

	return 
		mbe_tabview_addpage(
			ps_tpage->ps_parent,
			ps_tpage
		) == MARBLE_EC_OK;
}
#pragma endregion


#pragma region DLG-NEWLEVEL
struct mbe_levelview_dlgnewlvl_udata {
	TCHAR *pz_name;
	TCHAR *pz_cmt;

	int m_width;
	int m_height;
};


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
	struct mbe_dlginfo s_info = {
		.mp_hparent   = gls_editorapp.mp_hwnd,
		.map_ctrlinfo = glsa_newlvlctrlinfo,
		.m_nctrlinfo  = ARRAYSIZE(glsa_newlvlctrlinfo),
		.mp_udata     = &s_udata,
		.m_udatasz    = sizeof s_udata,
		.m_templ      = MBE_DLG_NewLvl
	};
	struct mbe_tabpage_callbacks s_cbs = {
		.mpfn_oncreate = &mbe_levelview_int_oncreate
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
		&ps_tpage
	);
	if (res == TRUE)
		MB_LOG_ERROR("Failed to create tab-view page.", 0);

	return res;
}
#pragma endregion


