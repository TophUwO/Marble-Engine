#include <internal/shared.h>

#include <editor.h>


/*
 * Structure representing a tileset, acting as
 * tab-page userdata.
 */
struct mbe_tileset {
	int m_twidth;  /* width, in tiles */
	int m_theight; /* height, in tiles */
	int m_pwidth;  /* physical width, in pixels */
	int m_pheight; /* physical height, in pixels */

	/*
	 * Bitmap holding the tileset pixels,
	 * unscaled. 
	 */
	ID2D1Bitmap *mp_bmp;
};

/*
 * Structure representing userdata shared
 * between all pages of the tileset
 * tab-view.
 */
struct mbe_tsetview {
	struct mbe_tabview_udata _base; /* common tab-view userdata */
};

/*
 * Userdata for the "Import tileset from image"
 * dialog.
 */
struct mbe_tsetview_dlgimgts_udata {
	TCHAR maz_path[MBE_MAXPATH]; /* file path */
	BOOL  m_loadall;             /* load-entire-bitmap flag */

	int m_left;   /* left margin, in tile units */
	int m_right;  /* right margin, in tile units */
	int m_top;    /* top margin, in tile units */
	int m_bottom; /* bottom margin, in tile units */
	int m_tsize;  /* tile size, in pixels */
};


#pragma region DLG-IMGTS
/*
 * This function is called whenever the user selects the "..." button. This will open a
 * standard windows "Open ..." dialog, allowing the user to select an arbitrary file, as
 * opposed to having the type the exact path manually.
 * 
 * Returns TRUE if the dialog concluded successfully, FALSE if not.
 */
static BOOL MB_CALLBACK mbe_tsetview_imgtsdlg_onbrowse(
	_In_     HWND p_hwnd,  /* dialog window */
	_In_opt_ void *p_udata /* dialog userdata */
) {
	if (p_hwnd == NULL || p_udata == NULL)
		return FALSE;

	/* Get userdata. */
	struct mbe_tsetview_dlgimgts_udata *ps_udata = (struct mbe_tsetview_dlgimgts_udata *)p_udata;

	/* Prepare the dialog. */
	ZeroMemory(ps_udata->maz_path, MBE_MAXPATH);
	OPENFILENAME s_dlginfo = {
		.lStructSize = sizeof s_dlginfo,
		.hwndOwner   = p_hwnd,
		.hInstance   = gls_editorapp.mp_hinst,
		.lpstrFilter = TEXT(
			"Supported image formats (*.bmp, *.png, *.jpg, *.jpeg, *.jfif, *.gif, *.ico, *.tiff, *.dds, *.wmp)\0*.BMP;*.PNG;*.JPG;*.JPEG;*.JFIF;*.GIF;*.ICO;*.TIFF;*.DDS;*.WMP\0"
			"Bitmaps (*.bmp)\0*.BMP\0"
			"Portable Network Graphics (*.png)\0*.PNG\0"
			"Joint Photographic Experts Group (*.jpg, *.jpeg, *.jfif)\0*.JPG;*.JPEG,*.JFIF\0"
			"Graphics Interchange Format (*.gif)\0*.GIF\0"
			"Microsoft Windows Icons (*.ico)\0*.ICO\0"
			"Tagged image format (*.tiff)\0*.TIFF\0"
			"DirectDraw surface (*.dds)\0\0"
		),
		.nFilterIndex = 1,
		.lpstrTitle   = TEXT("Import tileset from image"),
		.Flags        = OFN_PATHMUSTEXIST,
		.lpstrFile    = ps_udata->maz_path,
		.nMaxFile     = MBE_MAXPATH
	};

	/* Execute the dialog. */
	BOOL ret = GetOpenFileName(&s_dlginfo);
	if (ret == FALSE) {
		DWORD err = CommDlgExtendedError();

		switch (err) {
			case FNERR_BUFFERTOOSMALL:
				/*
				 * If the buffer is too small, display an
				 * error message. 
				 */
				MessageBox(p_hwnd, TEXT("The specified path is too long."), TEXT("Error"), MB_OK | MB_ICONERROR);

				MB_LOG_ERROR("The selected path is too long; limit=%i chars.", MBE_MAXPATH);
				break;
		}

		return FALSE;
	}

	/* Update "File Path" edit control. */
	SetWindowText(
		MBE_DLGWND(ImgTSDlg_EDIT_Path),
		ps_udata->maz_path
	);

	return TRUE;
}

/*
 * Executed when the state of the "Load entire tileset" checkbox changes.
 * 
 * Returns TRUE on success, FALSE on failure.
 */
static BOOL MB_CALLBACK mbe_tsetview_imgtsdlg_onloadall(
	_In_     HWND p_hwnd,  /* dialog window */
	         int newstate, /* new checked state */
	_In_opt_ void *p_udata /* dialog userdata */
) {
	if (p_hwnd == NULL)
		return FALSE;

	/* list of controls to change state of */
	int const a_ctrls[] = {
		ImgTSDlg_EDIT_Left,
		ImgTSDlg_EDIT_Right,
		ImgTSDlg_EDIT_Top, 
		ImgTSDlg_EDIT_Bottom,
		ImgTSDlg_SPIN_Left,
		ImgTSDlg_SPIN_Right,
		ImgTSDlg_SPIN_Top, 
		ImgTSDlg_SPIN_Bottom
	};

	/*
	 * Update enabled-state of each window
	 * in the list.
	 */
	for (int i = 0; i < ARRAYSIZE(a_ctrls); i++)
		EnableWindow(MBE_DLGWND(a_ctrls[i]), !newstate);

	return TRUE;
}

/* dialog control information */
static struct mbe_dlg_ctrlinfo const glsa_imgtsctrlinfo[] = {
	{ ImgTSDlg_EDIT_Path,
		.m_type   = MBE_DLGCTRLTYPE_EDIT,
		.m_wboff  = offsetof(struct mbe_tsetview_dlgimgts_udata, maz_path),
		.m_wbsize = MBE_MAXPATH, 
		.m_flags  = MBE_DLGCTRLFLAG_INITIALFOCUS | MBE_DLGCTRLFLAG_DROPTARGET
	},
	{ ImgTSDlg_BTN_Browse,
		.m_type  = MBE_DLGCTRLTYPE_BUTTON,
		._button = {
			.m_flags       = MBE_DLGBTNFLAG_MISC,
			.mpfn_onselect = &mbe_tsetview_imgtsdlg_onbrowse
		}
	},
	{ ImgTSDlg_SPIN_Left,
		.m_type   = MBE_DLGCTRLTYPE_SPINBOX,
		.m_wboff  = offsetof(struct mbe_tsetview_dlgimgts_udata, m_left),
		.m_wbsize = sizeof(int),
		._spin    = {
			.m_buddy = ImgTSDlg_EDIT_Left,
			.m_def   = 0,
			.m_min   = 0,
			.m_max   = 0xFFFF
		}
	},
	{ ImgTSDlg_SPIN_Right,
		.m_type   = MBE_DLGCTRLTYPE_SPINBOX,
		.m_wboff  = offsetof(struct mbe_tsetview_dlgimgts_udata, m_right),
		.m_wbsize = sizeof(int),
		._spin    = {
			.m_buddy = ImgTSDlg_EDIT_Right,
			.m_def   = 0,
			.m_min   = 0,
			.m_max   = 0xFFFF
		}
	},
	{ ImgTSDlg_SPIN_Top,
		.m_type   = MBE_DLGCTRLTYPE_SPINBOX,
		.m_wboff  = offsetof(struct mbe_tsetview_dlgimgts_udata, m_top),
		.m_wbsize = sizeof(int),
		._spin    = {
			.m_buddy = ImgTSDlg_EDIT_Top,
			.m_def   = 0,
			.m_min   = 0,
			.m_max   = 0xFFFF
		}
	},
	{ ImgTSDlg_SPIN_Bottom,
		.m_type   = MBE_DLGCTRLTYPE_SPINBOX,
		.m_wboff  = offsetof(struct mbe_tsetview_dlgimgts_udata, m_bottom),
		.m_wbsize = sizeof(int),
		._spin    = {
			.m_buddy = ImgTSDlg_EDIT_Bottom,
			.m_def   = 0,
			.m_min   = 0,
			.m_max   = 0xFFFF
		}
	},
	{ ImgTSDlg_SPIN_TSize,
		.m_type   = MBE_DLGCTRLTYPE_SPINBOX,
		.m_wboff  = offsetof(struct mbe_tsetview_dlgimgts_udata, m_tsize),
		.m_wbsize = sizeof(int),
		._spin    = {
			.m_buddy = ImgTSDlg_EDIT_TSize,
			.m_def   = 32,
			.m_min   = 1,
			.m_max   = 1024
		}
	},
	{ ImgTSDlg_BTN_Ok,
		.m_type  = MBE_DLGCTRLTYPE_BUTTON,
		._button = {
			.m_flags = MBE_DLGBTNFLAG_OK
		}
	},
	{ ImgTSDlg_BTN_Cancel,
		.m_type  = MBE_DLGCTRLTYPE_BUTTON,
		._button = {
			.m_flags = MBE_DLGBTNFLAG_CANCEL
		}
	},
	{ ImgTSDlg_BTN_Reset,
		.m_type  = MBE_DLGCTRLTYPE_BUTTON,
		._button = {
			.m_flags = MBE_DLGBTNFLAG_RESET
		}
	},
	{ ImgTSDlg_CB_LoadAll,
		.m_type   = MBE_DLGCTRLTYPE_CHECKBOX,
		.m_wboff  = offsetof(struct mbe_tsetview_dlgimgts_udata, m_loadall),
		.m_wbsize = sizeof(BOOL),
		._button  = {
			.m_defstate         = BST_CHECKED,
			.mpfn_onstatechange = &mbe_tsetview_imgtsdlg_onloadall
		}
	}
};


BOOL mbe_tsetview_imptsfromimg(
	_In_ struct mbe_tabview *ps_tview
) {
	if (ps_tview == NULL || ps_tview->m_isinit == FALSE)
		return FALSE;

	struct mbe_tabpage *ps_tpage;
	struct mbe_tsetview_dlgimgts_udata s_udata = { 0 };
	BOOL res;

	/* Set dialog attributes. */
	struct mbe_dlginfo const s_info = {
		.mp_hparent   = gls_editorapp.mp_hwnd,
		.map_ctrlinfo = glsa_imgtsctrlinfo,
		.m_nctrlinfo  = ARRAYSIZE(glsa_imgtsctrlinfo),
		.mp_udata     = &s_udata,
		.m_udatasz    = sizeof s_udata,
		.m_templ      = MBE_DLG_ImgTS
	};
	struct mbe_tabpage_callbacks const s_cbs = {
		.mpfn_oncreate  = NULL,
		.mpfn_onpaint   = NULL,
		.mpfn_onresize  = NULL,
		.mpfn_onzoom    = NULL,
		.mpfn_ondestroy = NULL
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
		s_udata.maz_path,
		NULL,
		&s_cbs,
		&s_udata,
		&ps_tpage
	);
	if (res == TRUE)
		MB_LOG_ERROR("Failed to create tileset-view page.", 0);

	return res;
}
#pragma endregion


