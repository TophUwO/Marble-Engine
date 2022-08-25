#include <editor.h>


/*
 * Structure representing data fields shared
 * by multiple dialogs concerning the tileset view.
 * Every "*_cp" struct has to put an instance of
 * this struct as its very first data field.
 */
struct mbe_tsviewdlg_cp {
	int  m_width;   /* width, in tiles */
	int  m_height;  /* height, in tiles */
	int  m_tsize;   /* tile size, in pixels */
};

/*
 * Shorten query for spinbox value.
 * Used when dialog field data is written back
 * to the "*_cp" buffer.
 */
#define MBE_GETSPVAL() (int)(SendMessage(p_hwnd, UDM_GETPOS32, 0, 0))


#pragma region CREATENEWTS-DLG
/*
 * Structure used for "Create new tileset" dialog.
 * Logical tileset will be created based on these
 * parameters.
 */
struct mbe_dlgnewts_cp {
	struct mbe_tsviewdlg_cp _base; /* common create parameters */

	TCHAR maz_name[MBE_MAXTSNAME]; /* tileset name */
	TCHAR maz_cmt[MBE_MAXCMT];     /* comment/description */

	BOOL m_istrans; /* transparent background? */
	/* information concerning aspect ratio lock state */
	struct {
		BOOL  m_islock; /* is locked? */
		float m_aspect; /* aspect ratio */
	} ms_aspect;
	/* background color */
	union {
		/* individual color components */
		struct {
			BYTE m_r; /* red component */
			BYTE m_g; /* green component */
			BYTE m_b; /* blue component */
			BYTE m_a; /* alpha component; unused */
		};
		COLORREF m_col; /* color as integer value */
	};
} ms_crps;


/*
 * Table representing the settings for the
 * spin controls. Also contains information useful
 * to simplify applying effects to various controls
 * at once.
 */
struct { int m_id, m_buddy; SHORT m_min, m_max; } const glas_udopts[] = {
	{ EmptyTSDlg_SPIN_Width,  EmptyTSDlg_EDIT_Width,  1, 1024 },
	{ EmptyTSDlg_SPIN_Height, EmptyTSDlg_EDIT_Height, 1, 1024 },
	{ EmptyTSDlg_SPIN_TSize,  EmptyTSDlg_EDIT_TSize,  1, 128  },
	{ EmptyTSDlg_SPIN_Red,    EmptyTSDlg_EDIT_Red,    0, 255  },
	{ EmptyTSDlg_SPIN_Green,  EmptyTSDlg_EDIT_Green,  0, 255  },
	{ EmptyTSDlg_SPIN_Blue,   EmptyTSDlg_EDIT_Blue,   0, 255  }
};
size_t const gl_nudopts = ARRAYSIZE(glas_udopts);

/*
 * Table representing maximum character counts
 * for all text fields of the dialog. When the dialog gets created,
 * all edit control's character limit will set to the value set
 * in this table.
 */
static struct { int m_id; size_t m_len; } const glas_eclimits[] = {
	{ EmptyTSDlg_EDIT_Name,    MBE_MAXTSNAME - 1 },
	{ EmptyTSDlg_EDIT_Comment, MBE_MAXCMT - 1    },
	{ EmptyTSDlg_EDIT_Width,   4                 },
	{ EmptyTSDlg_EDIT_Height,  4                 },
	{ EmptyTSDlg_EDIT_TSize,   4                 },
	{ EmptyTSDlg_EDIT_Red,     3                 },
	{ EmptyTSDlg_EDIT_Green,   3                 },
	{ EmptyTSDlg_EDIT_Blue,    3                 }
};
size_t const gl_neclimits = ARRAYSIZE(glas_eclimits);


/*
 * Sets-up controls to only accept a maximum amount
 * of input, that is, max. number of characters, range,
 * etc.
 * 
 * Returns nothing.
 */
static void mbe_dlgnewts_setupcontrols(HWND p_hwnd /* dialog window */) {
	/* Set the edit control limits. */
	for (size_t i = 0; i < gl_neclimits; i++)
		SendMessage(
			MBE_DLGWND(glas_eclimits[i].m_id),
			EM_SETLIMITTEXT,
			(WPARAM)glas_eclimits[i].m_len,
			0
		);

	/*
	 * Set default position, range, and buddy of all spin (up-down) controls.
	 * The buddy of an up-down control is a control that displays the text
	 * representation of the current (numeric) value (= position) of the
	 * up-down control.
	 */
	for (size_t i = 0; i < gl_nudopts; i++) {
		HWND const p_hdlgitem = MBE_DLGWND(glas_udopts[i].m_id);

		SendMessage(p_hdlgitem, UDM_SETRANGE, 0, MAKELONG(glas_udopts[i].m_max, glas_udopts[i].m_min));
		SendMessage(p_hdlgitem, UDM_SETBUDDY, (WPARAM)GetDlgItem(p_hwnd, glas_udopts[i].m_buddy), 0);
	}
}

/*
 * Sets the background color to transparent, disabling all controls
 * that may cause a conflict with this setting.
 * 
 * Returns nothing.
 */
static void mbe_dlgnewts_handletransparent(
	HWND p_hwnd,                     /* dialog window */
	struct mbe_dlgnewts_cp *ps_crps, /* create params */
	BOOL istrans                     /* new transparent setting */
) {
	/* Update internal state. */
	ps_crps->m_istrans = istrans;

	/* Enable/Disable necessary controls. */
	for (size_t i = 0; i < gl_nudopts; i++) {
		if (i < 3)
			continue;

		EnableWindow(MBE_DLGWND(glas_udopts[i].m_id), !ps_crps->m_istrans);
		EnableWindow(MBE_DLGWND(glas_udopts[i].m_buddy), !ps_crps->m_istrans);
	}
	EnableWindow(MBE_DLGWND(EmptyTSDlg_BTN_ChooseCol), !ps_crps->m_istrans);

	/* Update checkbox state. */
	CheckDlgButton(p_hwnd, EmptyTSDlg_CB_Transparent, (UINT)ps_crps->m_istrans);
}

/*
 * Sets dialog fields to specific values.
 * 
 * Returns nothing.
 */
static void mbe_dlgnewts_update(
	HWND p_hwnd,                    /* dialog window */
	struct mbe_dlgnewts_cp *ps_crps /* create parameters */
) {
	/*
	 * Table representing string fields.
	 * Will be used to fill all dialog edit controls
	 * with their respective values.
	 */
	struct { int m_id; TCHAR const *pz_str; } const as_strreps[] = {
		{ EmptyTSDlg_EDIT_Name,    ps_crps->maz_name },
		{ EmptyTSDlg_EDIT_Comment, ps_crps->maz_cmt  }
	};
	size_t const nreps = ARRAYSIZE(as_strreps);

	/*
	 * Fill string fields (edit controls) without an assisting
	 * spin control. 
	 */
	for (size_t i = 0; i < nreps; i++)
		SetWindowText(
			MBE_DLGWND(as_strreps[i].m_id),
			as_strreps[i].pz_str
		);

	/*
	 * Solely setting the up-down control position is enough, as the up-down controls
	 * all have a "buddy" registered which will be automatically notified of the
	 * position change and, if needed, will subsequently update its caption.
	 */
	SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Width), UDM_SETPOS, 0, (LPARAM)ps_crps->_base.m_width);
	SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Height), UDM_SETPOS, 0, (LPARAM)ps_crps->_base.m_height);
	SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_TSize), UDM_SETPOS, 0, (LPARAM)ps_crps->_base.m_tsize);
	SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Red), UDM_SETPOS, 0, (LPARAM)ps_crps->m_r);
	SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Green), UDM_SETPOS, 0, (LPARAM)ps_crps->m_g);
	SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Blue), UDM_SETPOS, 0, (LPARAM)ps_crps->m_b);

	/* Set checkbox states. */
	mbe_dlgnewts_handletransparent(p_hwnd, ps_crps, ps_crps->m_istrans);
	CheckDlgButton(p_hwnd, EmptyTSDlg_CB_LockAspect, (UINT)ps_crps->ms_aspect.m_islock);
}

/*
 * Opens a CHOOSECOLOR dialog and gets the color that the user
 * chooses, in RGB format.
 * 
 * Returns TRUE if the user chose a color, or FALSE if the
 * user cancelled the selection by
 *  (1) closing the dialog window
 *  (2) choosing the "Cancel" button of the color dialog
 * .
 */
static BOOL mbe_dlgnewts_handlecolordlg(
	HWND p_hwnd,                    /* dialog window handle */
	struct mbe_dlgnewts_cp *ps_crps /* create parameters */
) {
	/* Custom colors are not needed. */
	static COLORREF a_custcol[16] = { 0 };

	/*
	 * Specify options for the upcoming 
	 * CHOOSECOLOR dialog.
	 */
	CHOOSECOLOR s_ccol = {
		.lStructSize  = sizeof s_ccol,
		.hwndOwner    = p_hwnd,
		.lpCustColors = (LPDWORD)a_custcol,
		.rgbResult    = RGB(0, 0, 0),
		.Flags        = CC_FULLOPEN | CC_RGBINIT
	};
	
	/*
	 * Run system color dialog, updating internal state
	 * if the dialog succeeds.
	 */
	if (ChooseColor(&s_ccol) != FALSE) {
		ps_crps->m_col = s_ccol.rgbResult;

		return TRUE;
	}

	return FALSE;
}

/*
 * Executed for every child window of the
 * "Create empty tileset" dialog.
 * 
 * Returns TRUE.
 */
static BOOL mbe_tsetview_emptytsdlg_writeback_impl(
	HWND p_hwnd, /* window handle */
	LPARAM param /* create parameters */
) {
	struct mbe_dlgnewts_cp *ps_crps = (struct mbe_dlgnewts_cp *)param;

	LONG_PTR idctrl;
	switch (idctrl = GetWindowLongPtr(p_hwnd, GWLP_ID)) {
		case EmptyTSDlg_SPIN_Width:  ps_crps->_base.m_width  = MBE_GETSPVAL(); break;
		case EmptyTSDlg_SPIN_Height: ps_crps->_base.m_height = MBE_GETSPVAL(); break;
		case EmptyTSDlg_SPIN_TSize:  ps_crps->_base.m_tsize  = MBE_GETSPVAL(); break;
		case EmptyTSDlg_SPIN_Red:    ps_crps->m_r = (BYTE)MBE_GETSPVAL();      break;
		case EmptyTSDlg_SPIN_Green:  ps_crps->m_g = (BYTE)MBE_GETSPVAL();      break;
		case EmptyTSDlg_SPIN_Blue:   ps_crps->m_b = (BYTE)MBE_GETSPVAL();      break;
		case EmptyTSDlg_EDIT_Name:
		case EmptyTSDlg_EDIT_Comment:
			GetWindowText(
				p_hwnd,
				idctrl == EmptyTSDlg_EDIT_Name
					? ps_crps->maz_name
					: ps_crps->maz_cmt,
				(int)SendMessage(p_hwnd, EM_GETLIMITTEXT, 0, 0)
			);

			break;
		case EmptyTSDlg_CB_Transparent:
			ps_crps->m_istrans = (BOOL)SendMessage(p_hwnd, BM_GETCHECK, 0, 0);
			
			break;
	}

	return TRUE;
}

/*
 * Write back dialog field data into the create-params
 * buffer.
 * 
 * Returns nothing.
 */
static void mbe_tsetview_emptytsdlg_writeback(
	HWND p_hwnd,                    /* dialog window handle */
	struct mbe_dlgnewts_cp *ps_crps /* create parameters */
) {
	EnumChildWindows(
		p_hwnd,
		(WNDENUMPROC)&mbe_tsetview_emptytsdlg_writeback_impl,
		(LPARAM)ps_crps
	);
}

/*
 * Window procedure for "Create new tileset" dialog. 
 */
static BOOL CALLBACK mbe_tsetview_emptytsdlg_dlgproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	MBE_WNDUSERDATA(ps_param, struct mbe_dlgnewts_cp *);

	/* Default settings for this dialog. */
	static struct mbe_dlgnewts_cp const s_defcps = {
		._base     = {
			.m_width   = 24,
			.m_height  = 24,
			.m_tsize   = 32
		},
		.maz_name  = TEXT("unnamed"),
		.maz_cmt   = TEXT(""),
		
		.m_col     = RGB(0, 0, 0),
		.m_istrans = FALSE,
		.ms_aspect = {
			.m_islock = FALSE,
			.m_aspect = 1.0f
		}
	};

	switch (msg) {
		case WM_INITDIALOG:
			/* Copy default values into the userdata pointer. */
			CopyMemory((void *)lparam, &s_defcps, sizeof s_defcps);
			/* Set userdata. */
			SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)lparam);

			/* Initialize dialog fields. */
			mbe_dlgnewts_setupcontrols(p_hwnd);
			mbe_dlgnewts_update(
				p_hwnd,
				(struct mbe_dlgnewts_cp *)lparam
			);
			break;
		case WM_COMMAND:
			switch (wparam) {
				case EmptyTSDlg_CB_Transparent:
					mbe_dlgnewts_handletransparent(
						p_hwnd,
						ps_param,
						!ps_param->m_istrans
					);

					break;
				case EmptyTSDlg_CB_LockAspect:
					if ((ps_param->ms_aspect.m_islock = !ps_param->ms_aspect.m_islock) == TRUE)
						ps_param->ms_aspect.m_aspect = (float)ps_param->_base.m_width / (float)ps_param->_base.m_height;
					else
						ps_param->ms_aspect.m_aspect = 1.0f;

					break;
				case EmptyTSDlg_BTN_ChooseCol:
					/* Open CHOOSECOLOR dialog. */
					if (mbe_dlgnewts_handlecolordlg(p_hwnd, ps_param) == TRUE) {
						/*
						 * Update positions of spin controls. Note that this will automatically
						 * change the captions of the corresponding edit controls accordingly.
						 */
						SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Red), UDM_SETPOS, 0, ps_param->m_r);
						SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Green), UDM_SETPOS, 0, ps_param->m_g);
						SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Blue), UDM_SETPOS, 0, ps_param->m_b);
					}

					break;
				case EmptyTSDlg_BTN_Reset:
					/* Copy default values into the userdata pointer. */
					CopyMemory(ps_param, &s_defcps, sizeof s_defcps);

					mbe_dlgnewts_update(p_hwnd, ps_param);
					break;
				case EmptyTSDlg_BTN_Ok:
				case EmptyTSDlg_BTN_Cancel:
					/*
					 * If the user presses the "OK" button, update
					 * spin-less text properties. They never get
					 * written back to the "*_cp" structure
					 * throughout the lifetime of the dialog.
					 */
					if (wparam == EmptyTSDlg_BTN_Ok) {
						GetWindowText(MBE_DLGWND(EmptyTSDlg_EDIT_Name), ps_param->maz_name, MBE_MAXTSNAME);
						GetWindowText(MBE_DLGWND(EmptyTSDlg_EDIT_Comment), ps_param->maz_cmt, MBE_MAXCMT);
						
						mbe_tsetview_emptytsdlg_writeback(p_hwnd, ps_param);
					}

					/* End the dialog. */
					EndDialog(
						p_hwnd,
						wparam == EmptyTSDlg_BTN_Ok
					);

					break;
			}

			break;
		case WM_CLOSE:
			/*
			 * Closing the dialog via its system menu acts as
			 * if the user pressed the "Cancel" button.
			 */
			PostMessage(
				p_hwnd,
				WM_COMMAND,
				EmptyTSDlg_BTN_Cancel,
				0
			);

			break;
	}

	return FALSE;
}
#pragma endregion


#pragma region BMPTS-DLG
/*
 * Structure used for "Import tileset as bitmap" dialog.
 * Logical tileset will be created based on these
 * parameters.
 */
struct mbe_dlgbmpts_cp {
	struct mbe_tsviewdlg_cp _base; /* common create parameters */

	TCHAR maz_path[MBE_MAXPATH];   /* file path */
	BOOL m_iswhole;                /* Load whole bitmap? */
};


/*
 * Table representing the settings for the
 * spin controls.
 */
struct { int m_id, m_buddy; SHORT m_min, m_max; } const glas_bmptsdlg_udopts[] = {
	{ BmpTSDlg_SPIN_Width,  BmpTSDlg_EDIT_Width,  1, 1024 },
	{ BmpTSDlg_SPIN_Height, BmpTSDlg_EDIT_Height, 1, 1024 },
	{ BmpTSDlg_SPIN_TSize,  BmpTSDlg_EDIT_TSize,  1, 128, }
};
size_t const gl_bmptsdlg_nudopts = ARRAYSIZE(glas_bmptsdlg_udopts);

/*
 * Table representing maximum character counts
 * for all text fields of the dialog. When the dialog gets created,
 * all edit control's character limit will set to the value set
 * in this table.
 */
static struct { int m_id; size_t m_len; } const glas_bmptsdlg_eclimits[] = {
	{ BmpTSDlg_EDIT_Path,   MBE_MAXPATH - 1 },
	{ BmpTSDlg_EDIT_Width,  4               },
	{ BmpTSDlg_EDIT_Height, 4               },
	{ BmpTSDlg_EDIT_TSize,  4               }
};
size_t const gl_bmptsdlg_neclimits = ARRAYSIZE(glas_bmptsdlg_eclimits);


/*
 * Sets-up controls to only accept a maximum amount
 * of imput, that is, max. number of characters, range,
 * etc.
 * 
 * Returns nothing.
 */
static void mbe_dlgbmpts_setupcontrols(HWND p_hwnd /* dialog window */) {
	for (size_t i = 0; i < gl_bmptsdlg_neclimits; i++)
		SendMessage(
			MBE_DLGWND(glas_bmptsdlg_eclimits[i].m_id),
			EM_SETLIMITTEXT,
			(WPARAM)glas_bmptsdlg_eclimits[i].m_len,
			0
		);

	for (size_t i = 0; i < gl_bmptsdlg_nudopts; i++) {
		HWND const p_hdlgitem = MBE_DLGWND(glas_bmptsdlg_udopts[i].m_id);

		SendMessage(p_hdlgitem, UDM_SETRANGE, 0, MAKELONG(glas_bmptsdlg_udopts[i].m_max, glas_bmptsdlg_udopts[i].m_min));
		SendMessage(p_hdlgitem, UDM_SETBUDDY, (WPARAM)MBE_DLGWND(glas_bmptsdlg_udopts[i].m_buddy), 0);
	}
}

/*
 * Handle the event of the user clicking the "Load entire bitmap"
 * checkbox, disabling all edit boxes except for "File Path".
 * Consequently, unchecking the box has to enable the controls
 * again.
 * 
 * Returns nothing.
 */
static void mbe_dlgbmpts_handlecball(
	HWND p_hwnd,
	struct mbe_dlgbmpts_cp *ps_crps,
	BOOL newstate
) {
	/* Update internal state. */
	ps_crps->m_iswhole = newstate;

	/* Enable/Disable necessary controls. */
	for (size_t i = 0; i < gl_bmptsdlg_nudopts; i++) {
		if (glas_bmptsdlg_udopts[i].m_id == BmpTSDlg_SPIN_TSize)
			continue;

		EnableWindow(MBE_DLGWND(glas_bmptsdlg_udopts[i].m_id), !ps_crps->m_iswhole);
		EnableWindow(MBE_DLGWND(glas_bmptsdlg_udopts[i].m_buddy), !ps_crps->m_iswhole);
	}

	/* Update checkbox state. */
	CheckDlgButton(p_hwnd, BmpTSDlg_CB_LoadAll, (UINT)ps_crps->m_iswhole);
}

/*
 * Sets dialog fields to specific values.
 * 
 * Returns nothing.
 */
static void mbe_dlgbmpts_update(
	HWND p_hwnd,                    /* dialog window */
	struct mbe_dlgbmpts_cp *ps_crps /* create parameters */
) {
	/* Update path edit control. */
	SetWindowText(
		MBE_DLGWND(BmpTSDlg_EDIT_Path),
		ps_crps->maz_path
	);

	/*
	 * Solely setting the up-down control position is enough, as the up-down controls
	 * all have a "buddy" registered which will be automatically notified of the
	 * position change and, if needed, will subsequently update its caption.
	 */
	SendMessage(MBE_DLGWND(BmpTSDlg_SPIN_Width), UDM_SETPOS, 0, (LPARAM)ps_crps->_base.m_width);
	SendMessage(MBE_DLGWND(BmpTSDlg_SPIN_Height), UDM_SETPOS, 0, (LPARAM)ps_crps->_base.m_height);
	SendMessage(MBE_DLGWND(BmpTSDlg_SPIN_TSize), UDM_SETPOS, 0, (LPARAM)ps_crps->_base.m_tsize);

	/* Set checkbox states. */
	mbe_dlgbmpts_handlecball(p_hwnd, ps_crps, ps_crps->m_iswhole);
	CheckDlgButton(p_hwnd, BmpTSDlg_CB_LoadAll, (UINT)ps_crps->m_iswhole);
}

/*
 * Handle the event of the user clicking on the "..." button,
 * opening a dialog that allows the user to choose a file.
 * 
 * Returns TRUE if a file was chosen, FALSE if not.
 */
static BOOL mbe_dlgbmpts_handlebrowsebtn(
	HWND p_hwnd,                    /* dialog window */
	struct mbe_dlgbmpts_cp *ps_crps /* create parameters */
) {
	ZeroMemory(ps_crps->maz_path, MBE_MAXPATH);

	OPENFILENAME s_dlgdata = {
		.lStructSize = sizeof s_dlgdata,
		.hwndOwner   = p_hwnd,
		.lpstrTitle  = TEXT("Import tileset as bitmap"),
		.Flags       = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER,
		.lpstrFile   = ps_crps->maz_path,
		.nMaxFile    = MBE_MAXPATH

	};

	return GetOpenFileName(&s_dlgdata) != FALSE;
}

/*
 * Executed for every child window of the
 * "Import tileset from bitmap" dialog.
 * 
 * Returns TRUE.
 */
static BOOL mbe_bmptsdlg_writeback_impl(
	HWND p_hwnd, /* window handle */
	LPARAM param /* create parameters */
) {
	struct mbe_dlgbmpts_cp *ps_crps = (struct mbe_dlgbmpts_cp *)param;

	LONG_PTR idctrl;
	switch (idctrl = GetWindowLongPtr(p_hwnd, GWLP_ID)) {
		case BmpTSDlg_SPIN_Width:  ps_crps->_base.m_width  = MBE_GETSPVAL(); break;
		case BmpTSDlg_SPIN_Height: ps_crps->_base.m_height = MBE_GETSPVAL(); break;
		case BmpTSDlg_SPIN_TSize:  ps_crps->_base.m_tsize  = MBE_GETSPVAL(); break;
		case BmpTSDlg_EDIT_Path:
			GetWindowText(
				p_hwnd,
				ps_crps->maz_path,
				(int)SendMessage(p_hwnd, EM_GETLIMITTEXT, 0, 0)
			);

			break;
		case BmpTSDlg_CB_LoadAll:
			ps_crps->m_iswhole = (BOOL)SendMessage(p_hwnd, BM_GETCHECK, 0, 0);

			break;
	}

	return TRUE;
}

/*
 * Write back dialog field data into the create-params
 * buffer.
 * 
 * Returns nothing.
 */
static void mbe_bmptsdlg_writeback(
	HWND p_hwnd,                    /* dialog window handle */
	struct mbe_dlgbmpts_cp *ps_crps /* create parameters */
) {
	EnumChildWindows(
		p_hwnd,
		(WNDENUMPROC)&mbe_bmptsdlg_writeback_impl,
		(LPARAM)ps_crps
	);
}

/*
 * Window procedure for "Import tileset from bitmap" dialog. 
 */
static BOOL CALLBACK mbe_tsetview_bmptsdlg_dlgproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	MBE_WNDUSERDATA(ps_param, struct mbe_dlgbmpts_cp *);

	/* Default create parameters. */
	static struct mbe_dlgbmpts_cp const s_defcps = {
		._base     = {
			.m_width  = 1,
			.m_height = 1,
			.m_tsize  = 32
		},
		.maz_path  = TEXT(""),
		.m_iswhole = TRUE
	};

	switch (msg) {
		case WM_INITDIALOG:
			/* Copy default values into the userdata pointer. */
			CopyMemory((void *)lparam, &s_defcps, sizeof s_defcps);
			/* Set userdata. */
			SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)lparam);

			mbe_dlgbmpts_setupcontrols(p_hwnd);
			mbe_dlgbmpts_update(
				p_hwnd,
				(struct mbe_dlgbmpts_cp *)lparam
			);
			return FALSE;
		case WM_COMMAND:
			switch (wparam) {
				case BmpTSDlg_BTN_Browse: {
					if (mbe_dlgbmpts_handlebrowsebtn(p_hwnd, ps_param) == TRUE)
						SetWindowText(
							MBE_DLGWND(BmpTSDlg_EDIT_Path),
							ps_param->maz_path
						);

					break;
				}
				case BmpTSDlg_CB_LoadAll:
					mbe_dlgbmpts_handlecball(
						p_hwnd,
						ps_param,
						!ps_param->m_iswhole
					);

					break;
				case BmpTSDlg_BTN_Reset:
					/* Copy default values into the userdata pointer. */
					CopyMemory(ps_param, &s_defcps, sizeof s_defcps);

					mbe_dlgbmpts_update(p_hwnd, ps_param);
					break;

					break;
				case BmpTSDlg_BTN_Ok:
				case BmpTSDlg_BTN_Cancel:
					/*
					 * As we do never query "Path's" value,
					 * we simply ask for it before we end the dialog.
					 */
					if (wparam == BmpTSDlg_BTN_Ok) {
						mbe_bmptsdlg_writeback(p_hwnd, ps_param);

						/*
						 * Check whether the file path is empty. If that happens,
						 * display a warning message, do not let the dialog to end,
						 * and give the user the chance to correct their input.
						 */
						if (*ps_param->maz_path == TEXT('\0')) {
							MessageBox(
								p_hwnd,
								TEXT("You have to choose a file to open."),
								TEXT("Warning"),
								MB_ICONWARNING | MB_OK
							);

							break;
						}
					}

					/* End the dialog. */
					EndDialog(
						p_hwnd,
						wparam == BmpTSDlg_BTN_Ok
					);

					break;
			}

			break;
		case WM_CLOSE:
			/*
			* Closing the dialog via its system menu acts as
			* if the user pressed the "Cancel" button.
			*/
			PostMessage(
				p_hwnd,
				WM_COMMAND,
				BmpTSDlg_BTN_Cancel,
				0
			);

			break;
	}

	return FALSE;
}
#pragma endregion


#pragma region TILESET-CTRL
static int const gl_viewtsize = 32;
static TCHAR const *const glpz_tsviewwndclname = TEXT("mbe_tsview");

/*
 * Calculates current client area rectangle of tab control.
 * 
 * Returns nothing.
 */
static void mbe_tsetview_internal_getrect(
	struct mbe_tsetview *ps_tsetview, /* tileset view */
	RECT *ps_rect                     /* destination rectangle */
) {
	if (ps_tsetview == NULL || ps_rect == NULL || ps_tsetview->m_isinit == FALSE)
		return;

	GetClientRect(ps_tsetview->mp_hwnd, ps_rect);
	TabCtrl_AdjustRect(ps_tsetview->mp_hwnd, FALSE, ps_rect);
}

/*
 * Sets up the internal memory DC of tileset view.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t mbe_tset_internal_setupdc(
	struct mbe_tset *ps_tset,        /* tileset */
	struct mbe_tsviewdlg_cp *ps_crps /* create parameters */
) {
	if (ps_tset == NULL || ps_crps == NULL)
		return MARBLE_EC_PARAM;

	/* Get DC of Desktop window. */
	HDC p_hdc = GetDC(GetDesktopWindow());
	/* Create DC compatible with the desktop window. */
	ps_tset->ms_res.p_hbmpdc = CreateCompatibleDC(p_hdc);
	if (ps_tset->ms_res.p_hbmpdc == NULL)
		return MARBLE_EC_CREATEMEMDC;

	/* Calculate bitmap sizes. */
	ps_tset->ms_sz.tsize     = ps_crps->m_tsize;
	ps_tset->ms_sz.m_twidth  = ps_crps->m_width;
	ps_tset->ms_sz.m_theight = ps_crps->m_height;
	ps_tset->ms_sz.m_pwidth  = ps_crps->m_width  * gl_viewtsize;
	ps_tset->ms_sz.m_pheight = ps_crps->m_height * gl_viewtsize;

	/* Create memory bitmap. */
	ps_tset->ms_res.p_hbmpdcbmp = CreateCompatibleBitmap(
		/*
		 * This has to be the DC of the desktop window (or the DC we used to create
		 * the memory DC with), as the newly-created DC will hold a 1x1 monochrome
		 * bitmap by default, which is why it cannot be used to create a usable bitmap
		 * as it would just create another monochrome bitmap of a specific size. Unless
		 * this is exactly what we want, which it isn't in this case, we have to create
		 * a bitmap compatible with our desktop device context.
		 */
		p_hdc,
		ps_tset->ms_sz.m_pwidth,
		ps_tset->ms_sz.m_pheight
	);
	if (ps_tset->ms_res.p_hbmpdcbmp == NULL)
		return MARBLE_EC_CREATEMEMBITMAP;

	/*
	 * Select newly-created bitmap into memory DC.
	 * The default bitmap is saved as it will be restored later
	 * when the memory device context is about to be deleted.
	 */
	ps_tset->ms_res.p_hbmpdcold = SelectObject(ps_tset->ms_res.p_hbmpdc, ps_tset->ms_res.p_hbmpdcbmp);

	/* Release device context of desktop window. */
	ReleaseDC(GetDesktopWindow(), p_hdc);
	return MARBLE_EC_OK;
}

/*
 * Update scrollbars whenever the window gets resized.
 * 
 * Returns nothing.
 */
static void mbe_tset_internal_updatescrollbarinfo(
	struct mbe_tset *ps_tset, /* tileset view */
	int nwidth,               /* new width of tileset view, in pixels */
	int nheight               /* new height of tileset view, in pixels */
) {
	if (ps_tset == NULL)
		return;

	BOOL isxvisible = FALSE, isyvisible = FALSE;

	/* Update vertical scrollbar. */
	if (ps_tset->ms_sz.m_pheight > nheight) {
		int const maxscr = max(ps_tset->ms_sz.m_pheight - nheight, 0); 

		ps_tset->s_yscr = (SCROLLINFO){
			.cbSize = sizeof ps_tset->s_yscr,
			.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS,
			.nMin   = 0,
			.nMax   = ps_tset->ms_sz.m_pheight,
			.nPage  = nheight,
			.nPos   = min(ps_tset->s_yscr.nPos, maxscr)
		};

		isyvisible = TRUE;
		SetScrollInfo(ps_tset->p_hwnd, SB_VERT, &ps_tset->s_yscr, TRUE);
	}

	/* Update horizontal scrollbar. */
	if (ps_tset->ms_sz.m_pwidth > nwidth) {
		int const maxscr = max(ps_tset->ms_sz.m_pwidth - nwidth, 0);

		ps_tset->s_xscr = (SCROLLINFO){
			.cbSize = sizeof ps_tset->s_xscr,
			.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS,
			.nMin   = 0,
			.nMax   = ps_tset->ms_sz.m_pwidth,
			.nPage  = nwidth,
			.nPos   = min(ps_tset->s_xscr.nPos, maxscr)
		};

		isxvisible = TRUE;
		SetScrollInfo(ps_tset->p_hwnd, SB_HORZ, &ps_tset->s_xscr, TRUE);
	}

	/* Update visible states of scrollbars. */
	ShowScrollBar(ps_tset->p_hwnd, SB_HORZ, isxvisible);
	ShowScrollBar(ps_tset->p_hwnd, SB_VERT, isyvisible);
}

/*
 * Checks if the requested selection is out of bounds.
 * 
 * Returns TRUE if the selection is out of bounds, FALSE
 * if not.
 */
static BOOL mbe_tsetview_internal_isseloob(
	struct mbe_tset *ps_tset, /* tileset view */
	int xindex,               /* requested x-index */
	int yindex                /* requested y-index */
) {
	return 
		   xindex < 0
		|| yindex < 0
		|| xindex >= ps_tset->ms_sz.m_twidth
		|| yindex >= ps_tset->ms_sz.m_theight
	;
}

/*
 * Handles WM_HSCROLL and WM_VSCROLL messages.
 * 
 * Returns nothing.
 */
static void mbe_tsetview_internal_handlescrolling(
	UINT msg,                /* message ID */
	LONG param,              /* parameter */
	struct mbe_tset *ps_tset /* tileset view */
) {
	int newpos, delta;

	/* Get correct scrollbar information structure. */
	SCROLLINFO *ps_scrinfo = msg == WM_HSCROLL
		? &ps_tset->s_xscr
		: &ps_tset->s_yscr
	;

	switch (LOWORD(param)) {
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:    newpos = HIWORD(param); break;
		default:
			newpos = ps_scrinfo->nPos;
	}
	newpos = min(ps_scrinfo->nMax, max(0, newpos));

	/* Update scrollbar info. */
	ps_scrinfo->fMask = SIF_POS;
	delta = ps_scrinfo->nPos - newpos;
	ps_scrinfo->nPos = newpos;

	SetScrollInfo(
		ps_tset->p_hwnd,
		msg == WM_HSCROLL
		? SB_HORZ
		: SB_VERT
		, ps_scrinfo,
		TRUE
	);

	InvalidateRect(ps_tset->p_hwnd, NULL, TRUE);
}

/*
 * Handles changing the current tile selection of
 * a tileset view.
 * 
 * returns nothing.
 */
static void mbe_tsetview_internal_handleselection(
	UINT msg,                /* message identifier */
	WPARAM wparam,           /* wndproc wparam */
	LPARAM lparam,           /* wndproc lparam */
	struct mbe_tset *ps_tset /* tileset view */
) {
	if (ps_tset == NULL || ps_tset->m_isinit == FALSE)
		return;

	int reqx, reqy;

	switch (msg) {
		case WM_LBUTTONDOWN:
			/* Compute selection indices. */
			reqx = (ps_tset->s_xscr.nPos + GET_X_LPARAM(lparam)) / gl_viewtsize;
			reqy = (ps_tset->s_yscr.nPos + GET_Y_LPARAM(lparam)) / gl_viewtsize;

			/*
			 * If requested selection is out of bounds,
			 * do not change the selection and return.
			 */
			if (mbe_tsetview_internal_isseloob(ps_tset, reqx, reqy) == TRUE)
				return;

			ps_tset->ms_sel.m_xindex = reqx;
			ps_tset->ms_sel.m_yindex = reqy;

			/* Compute selection rectangle. */
			ps_tset->ms_sel.s_rsel   = (RECT){
				ps_tset->ms_sel.m_xindex * gl_viewtsize,
				ps_tset->ms_sel.m_yindex * gl_viewtsize,
				(ps_tset->ms_sel.m_xindex + 1) * gl_viewtsize,
				(ps_tset->ms_sel.m_yindex + 1) * gl_viewtsize
			};

			break;
		case WM_KEYDOWN:
			switch (wparam) {
				
			}

			break;
	}

	InvalidateRect(ps_tset->p_hwnd, NULL, TRUE);
}

/*
 * Frees all resources used by a logical tileset and its view.
 * 
 * Returns nothing.
 */
static void mbe_tset_destroy(struct mbe_tset *ps_tset /* tileset to free resources of */) {
	if (ps_tset == NULL || ps_tset->m_isinit == FALSE)
		return;

	/* Select default bitmap into memory device context. */
	SelectObject(ps_tset->ms_res.p_hbmpdc, ps_tset->ms_res.p_hbmpdcold);

	/* Free user resources. */
	DeleteDC(ps_tset->ms_res.p_hbmpdc);
	DeleteObject(ps_tset->ms_res.p_hbmpdcbmp);
}

/*
 * Window procedure for single tileset views.
 * Note that this is not for the container of the view which
 * is a tab control, but for the controls that display
 * the tiles themselves.
 */
static LRESULT CALLBACK mbe_tsetview_internal_wndproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	MBE_WNDUSERDATA(ps_udata, struct mbe_tset *);

	HDC p_hdc;
	PAINTSTRUCT s_ps;
	RECT s_rect;
	HPEN p_hpold;
	HBRUSH p_hbrold;
	int orix, oriy, oldbgmode;

	switch (msg) {
		case WM_CREATE: MBE_SETUDATA(); return FALSE;
		case WM_SIZE:
			if (ps_udata->m_isinit == TRUE)
				mbe_tset_internal_updatescrollbarinfo(
					ps_udata,
					GET_X_LPARAM(lparam),
					GET_Y_LPARAM(lparam)
				);

			return FALSE;
		case WM_HSCROLL:
		case WM_VSCROLL:
			mbe_tsetview_internal_handlescrolling(
				msg,
				(LONG)wparam,
				ps_udata
			);

			return FALSE;
		case WM_KEYDOWN:
		case WM_LBUTTONDOWN:
			mbe_tsetview_internal_handleselection(
				msg,
				wparam,
				lparam,
				ps_udata
			);

			return FALSE;
		case WM_PAINT:
			p_hdc = BeginPaint(p_hwnd, &s_ps);

			/* Draw tileset bitmap. */
			GetClientRect(p_hwnd, &s_rect);
			BitBlt(
				p_hdc,
				s_rect.left,
				s_rect.top,
				s_rect.right,
				s_rect.bottom,
				ps_udata->ms_res.p_hbmpdc,
				ps_udata->s_xscr.nPos,
				ps_udata->s_yscr.nPos,
				SRCCOPY
			);

			/* Draw dotted grid. */
			p_hpold   = SelectObject(p_hdc, gls_editorapp.ms_res.mp_hpgrid);
			oldbgmode = SetBkMode(p_hdc, TRANSPARENT);

			/*
			 * Calculate the position of the first tile border which is to the
			 * left/top of the left/top side of the tileset view and use this
			 * as the origin.
			 */
			orix = (ps_udata->s_xscr.nPos / gl_viewtsize) * gl_viewtsize - ps_udata->s_xscr.nPos;
			oriy = (ps_udata->s_yscr.nPos / gl_viewtsize) * gl_viewtsize - ps_udata->s_yscr.nPos;

			/* Draw vertical grid lines. */
			for (int x = orix; x < s_rect.right; x += gl_viewtsize) {
				MoveToEx(p_hdc, x, oriy, NULL);

				/* Draw line. */
				LineTo(p_hdc, x, s_rect.bottom);
			}

			/* Draw horizontal grid lines. */
			for (int y = oriy; y < s_rect.bottom; y += gl_viewtsize) {
				MoveToEx(p_hdc, orix, y, NULL);

				/* Draw line. */
				LineTo(p_hdc, s_rect.right, y);
			}

			/* Draw selection rectangle. */
			SelectPen(p_hdc, gls_editorapp.ms_res.mp_hpsel);
			p_hbrold = SelectBrush(p_hdc, GetStockObject(NULL_BRUSH));

			Rectangle(
				p_hdc,
				ps_udata->ms_sel.s_rsel.left - ps_udata->s_xscr.nPos,
				ps_udata->ms_sel.s_rsel.top - ps_udata->s_yscr.nPos,
				ps_udata->ms_sel.s_rsel.right - ps_udata->s_xscr.nPos + 1,
				ps_udata->ms_sel.s_rsel.bottom - ps_udata->s_yscr.nPos + 1
			);

			SelectPen(p_hdc, p_hpold);
			SelectBrush(p_hdc, p_hbrold);
			SetBkMode(p_hdc, oldbgmode);

			EndPaint(p_hwnd, &s_ps);
			return FALSE;
	}

	return DefWindowProc(p_hwnd, msg, wparam, lparam);
}

/*
 * Creates a blank window and adds a tab to the tileset view container.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t mbe_tsetview_internal_prepareview(
	struct mbe_tsetview *ps_parent, /* tileset view container */
	struct mbe_tset *ps_tset,       /* tileset */
	TCHAR const *pz_title,          /* tab caption */
	RECT *s_parentsize              /* size of tileset view window */
) {
	/* Insert tab item. */
	TCITEM const s_item = {
		.mask    = TCIF_TEXT,
		.pszText = (LPWSTR)pz_title
	};
	if (TabCtrl_InsertItem(ps_parent->mp_hwnd, ps_parent->m_nts++, &s_item) == -1)
		return MARBLE_EC_UNKNOWN;

	/* Get current size of tab view. */
	mbe_tsetview_internal_getrect(ps_parent, s_parentsize);

	/* Create window of tileset view. */
	ps_tset->p_hwnd = CreateWindowEx(
		WS_EX_COMPOSITED,
		glpz_tsviewwndclname,
		NULL,
		WS_CHILD | WS_VSCROLL,
		s_parentsize->left,
		s_parentsize->top,
		s_parentsize->right - s_parentsize->left,
		s_parentsize->bottom - s_parentsize->top,
		ps_parent->mp_hwnd,
		NULL,
		gls_editorapp.mp_hinst,
		(LPVOID)ps_tset
	);
	if (ps_tset->p_hwnd == NULL)
		return MARBLE_EC_CREATEWND;

	return MARBLE_EC_OK;
}

/*
 * Initializes selection information of a tileset view.
 * 
 * Returns nothing.
 */
static void mbe_tsetview_internal_initselection(struct mbe_tset *ps_tset /* tileset view */) {
	if (ps_tset == NULL)
		return;

	SendMessage(ps_tset->p_hwnd, WM_LBUTTONDOWN, 0, 0);
}

/*
 * Creates an empty logical tileset.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t mbe_tsetview_internal_createemptyts(
	struct mbe_tsetview *ps_parent,  /* parent window */
	struct mbe_dlgnewts_cp *ps_crps, /* tileset parameters */
	struct mbe_tset *ps_tset         /* destination */
) {
	if (ps_parent == NULL || ps_crps == NULL || ps_tset == NULL) return MARBLE_EC_INTERNALPARAM;
	if (ps_parent->m_isinit == FALSE)                            return MARBLE_EC_COMPSTATE;

	marble_ecode_t ecode = MARBLE_EC_OK;

	RECT s_parentsize;
	ecode = mbe_tsetview_internal_prepareview(
		ps_parent,
		ps_tset,
		ps_crps->maz_name,
		&s_parentsize
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* Create empty bitmap. */
	ecode = mbe_tset_internal_setupdc(ps_tset, &ps_crps->_base);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* Update init state. */
	ps_tset->m_isinit = TRUE;

	/* Initialize scrollbars. */
	mbe_tset_internal_updatescrollbarinfo(
		ps_tset,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top
	);
	/* Initialize selection. */
	mbe_tsetview_internal_initselection(ps_tset);

lbl_END:
	if (ecode != MARBLE_EC_OK) {
		--ps_parent->m_nts;

		if (ps_tset->p_hwnd != NULL)
			DestroyWindow(ps_tset->p_hwnd);

		mbe_tset_destroy(ps_tset);
	}

	return ecode;
}

/*
 * Creates a tileset bitmap from a raw bitmap file.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t mbe_tsetview_internal_createtsbmpfromsrc(
	struct mbe_tsetview *ps_parent, /* parent window */
	struct mbe_tset *ps_tset,       /* destination */
	struct mbe_dlgbmpts_cp *ps_crps /* tileset parameters */
) {
	HBITMAP p_hbmp = NULL, p_hbmpold;
	HDC p_bmpdc = NULL, p_deskdc;
	int oldbltmode;
	BITMAPINFO s_info;

	marble_ecode_t ecode = MARBLE_EC_OK;

	p_hbmp = LoadImage(
		NULL,
		ps_crps->maz_path,
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE
	);
	if (p_hbmp == NULL) {
		MessageBox(
			ps_parent->mp_hwnd, 
			TEXT("Could not load the selected file.")
			TEXT(" The file is of an unknown format."),
			TEXT("Error"),
			MB_ICONERROR | MB_OK
		);

		return MARBLE_EC_LOADRESOURCE;
	}

	/*
	 * If we specified the whole bitmap, we have to use the
	 * pixel dimensions of our loaded bitmap and substitute
	 * them for the values we got from the final update of
	 * the dialog.
	 */
	GetObject(p_hbmp, sizeof s_info, &s_info);
	if (ps_crps->m_iswhole == TRUE) {
		ps_crps->_base.m_width  = s_info.bmiHeader.biWidth  / ps_crps->_base.m_tsize;
		ps_crps->_base.m_height = s_info.bmiHeader.biHeight / ps_crps->_base.m_tsize;
	}

	ecode = mbe_tset_internal_setupdc(ps_tset, &ps_crps->_base);
	if (ecode != MARBLE_EC_OK) {
		DeleteObject(p_hbmp);

		return ecode;
	}

	/* Draw stretched bitmap. */
	oldbltmode = SetStretchBltMode(ps_tset->ms_res.p_hbmpdc, COLORONCOLOR); 

	p_deskdc = GetDC(GetDesktopWindow());
	p_bmpdc  = CreateCompatibleDC(p_deskdc);
	if (p_bmpdc == NULL) {
		ecode = MARBLE_EC_CREATEMEMDC;

		goto lbl_END;
	}
	p_hbmpold = SelectObject(p_bmpdc, p_hbmp);

	StretchBlt(
		ps_tset->ms_res.p_hbmpdc,
		0,
		0,
		ps_tset->ms_sz.m_pwidth,
		ps_tset->ms_sz.m_pheight,
		p_bmpdc,
		0,
		0,
		s_info.bmiHeader.biWidth,
		s_info.bmiHeader.biHeight,
		SRCCOPY
	);

	SelectObject(p_bmpdc, p_hbmpold);
	DeleteDC(p_bmpdc);

lbl_END:
	DeleteObject(p_hbmp);
	SetStretchBltMode(ps_tset->ms_res.p_hbmpdc, oldbltmode);

	ReleaseDC(GetDesktopWindow(), p_deskdc);
	return MARBLE_EC_OK;
}

/*
 * Creates a logical tileset from an already existing bitmap
 * file.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t mbe_tsetview_internal_createtsfrombmp(
	struct mbe_tsetview *ps_parent,  /* parent window */
	struct mbe_dlgbmpts_cp *ps_crps, /* tileset parameters */
	struct mbe_tset *ps_tset         /* destination */
) {
	marble_ecode_t ecode = MARBLE_EC_OK;

	/*
	 * Extract filename and use it as
	 * caption for the tab.
	 */
	_tsplitpath_s(
		ps_crps->maz_path,
		NULL,
		0,
		NULL,
		0,
		ps_tset->maz_name,
		MBE_MAXTSNAME,
		NULL,
		0
	);

	RECT s_parentsize;
	ecode = mbe_tsetview_internal_prepareview(
		ps_parent,
		ps_tset,
		ps_tset->maz_name,
		&s_parentsize
	);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	/* Create tileset bitmap from source bitmap. */
	ecode = mbe_tsetview_internal_createtsbmpfromsrc(
		ps_parent,
		ps_tset,
		ps_crps
	);
	/* Update init state. */
	ps_tset->m_isinit = TRUE;

	/* Initialize scrollbars. */
	mbe_tset_internal_updatescrollbarinfo(
		ps_tset,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top
	);
	/* Initialize selection. */
	mbe_tsetview_internal_initselection(ps_tset);

	return ecode;
}
#pragma endregion


/*
 * Checks whether the maximum number of tilesets per view
 * has been reached.
 * The function will display an error message and return
 * TRUE if the tileset view is full.
 * 
 * Returns TRUE if tileset view is full, FALSE if not.
 */
static BOOL mbe_tsetview_internal_isfull(struct mbe_tsetview *ps_tsetview /* tileset view */) {
	if (ps_tsetview == NULL || ps_tsetview->m_isinit == FALSE)
		return TRUE;

	if (ps_tsetview->m_nts == MBE_MAXTSC) {
		MessageBox(
			NULL,
				TEXT("Could not create tileset.")
				TEXT("\nThe maximum number of tilesets per view has been reached."),
			TEXT("Error"),
			MB_ICONERROR | MB_OK
		);

		return TRUE;
	}

	return FALSE;
}


marble_ecode_t mbe_tsetview_init(
	HWND p_hparent,
	struct mbe_tsetview *ps_tsetview
) {
	if (ps_tsetview == NULL || p_hparent == NULL) return MARBLE_EC_PARAM;
	if (ps_tsetview->m_isinit == TRUE)            return MARBLE_EC_COMPSTATE;

	marble_ecode_t ecode = MARBLE_EC_OK;

	/*
	 * To avoid any weird behavior regarding usage of uninitialized memory,
	 * fill the entire structure with zeroes.
	 */
	ZeroMemory(ps_tsetview, sizeof ps_tsetview);

	/* Register tileset view window class. */
	WNDCLASSEX s_wndclass = {
		.cbSize        = sizeof s_wndclass,
		.hInstance     = gls_editorapp.mp_hinst,
		.lpszClassName = glpz_tsviewwndclname,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.lpfnWndProc   = (WNDPROC)&mbe_tsetview_internal_wndproc,
		.hbrBackground = gls_editorapp.ms_res.mp_hbrblack
	};
	if (RegisterClassEx(&s_wndclass) == FALSE) {
		ecode = MARBLE_EC_REGWNDCLASS;

		goto lbl_END;
	}

	/* Create tab view window. */
	ps_tsetview->mp_hwnd = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		WC_TABCONTROL,
		NULL,
		WS_CHILD | WS_CLIPCHILDREN,
		0,
		0,
		200,
		700,
		p_hparent,
		NULL,
		gls_editorapp.mp_hinst,
		NULL
	);
	if (ps_tsetview->mp_hwnd == NULL)
		return MARBLE_EC_CREATEWND;

	/* Manually set tab-view userdata. */
	SetWindowLongPtr(ps_tsetview->mp_hwnd, GWLP_USERDATA, (LONG_PTR)ps_tsetview);

	/* Set tab text font. */
	SendMessage(
		ps_tsetview->mp_hwnd,
		WM_SETFONT,
		(WPARAM)gls_editorapp.ms_res.mp_hguifont,
		(LPARAM)TRUE
	);

	/* Set init state. */
	ps_tsetview->m_isinit = TRUE;

lbl_END:
	if (ecode != MARBLE_EC_OK) {
		UnregisterClass(glpz_tsviewwndclname, gls_editorapp.mp_hinst);

		if (ps_tsetview->mp_hwnd != NULL)
			DestroyWindow(ps_tsetview->mp_hwnd);

		ZeroMemory(ps_tsetview, sizeof ps_tsetview);
	}

	return ecode;
}

void mbe_tsetview_uninit(struct mbe_tsetview *ps_tsetview) {
	if (ps_tsetview == NULL || ps_tsetview->m_isinit == FALSE)
		return;

	/* Free all resources used by all tilesets. */
	for (int i = 0; i < ps_tsetview->m_nts; i++)
		mbe_tset_destroy(&ps_tsetview->mas_ts[i]);

	/* Reset state. */
	ps_tsetview->m_isinit = FALSE;
}

void mbe_tsetview_resize(
	struct mbe_tsetview *ps_tsetview,
	int nwidth,
	int nheight
) {
	if (ps_tsetview == NULL || ps_tsetview->m_isinit == FALSE)
		return;

	SetWindowPos(
		ps_tsetview->mp_hwnd,
		NULL,
		0,
		0,
		200,
		nheight,
		SWP_NOACTIVATE
	);

	RECT s_parentsize;
	mbe_tsetview_internal_getrect(ps_tsetview, &s_parentsize);

	struct mbe_tset *ps_curts = &ps_tsetview->mas_ts[ps_tsetview->m_curtsi];
	SetWindowPos(
		ps_curts->p_hwnd,
		NULL,
		s_parentsize.left,
		s_parentsize.top,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top,
		SWP_NOACTIVATE
	);
}

marble_ecode_t mbe_tsetview_newtsdlg(struct mbe_tsetview *ps_tsetview) {
	/*
	 * The init-state of the tileset view container is not that
	 * important, as it will be created when the first tileset
	 * view is being created.
	 */
	if (ps_tsetview == NULL)
		return MARBLE_EC_PARAM;

	/* Check if the tileset view still has free slots. */
	if (mbe_tsetview_internal_isfull(ps_tsetview) == TRUE)
		return MARBLE_EC_COMPSTATE;

	struct mbe_dlgnewts_cp s_crps;
	if (DialogBoxParam(
		gls_editorapp.mp_hinst,
		MAKEINTRESOURCE(MBE_DLG_EmptyTS),
		gls_editorapp.mp_hwnd,
		(DLGPROC)&mbe_tsetview_emptytsdlg_dlgproc,
		(LPARAM)&s_crps
	) != FALSE) {
		marble_ecode_t ecode = mbe_tsetview_internal_createemptyts(
			ps_tsetview,
			&s_crps,
			&ps_tsetview->mas_ts[ps_tsetview->m_nts]
		);
		if (ecode != MARBLE_EC_OK)
			return ecode;

		ShowWindow(ps_tsetview->mp_hwnd, SW_SHOW);
		mbe_tsetview_setpage(ps_tsetview, ps_tsetview->m_curtsi);
	}

	return MARBLE_EC_OK;
}

marble_ecode_t mbe_tsetview_bmptsdlg(struct mbe_tsetview *ps_tsetview) {
	if (ps_tsetview == NULL)
		return MARBLE_EC_PARAM;

	/* Check if the tileset view still has free slots. */
	if (mbe_tsetview_internal_isfull(ps_tsetview) == TRUE)
		return MARBLE_EC_COMPSTATE;

	struct mbe_dlgbmpts_cp s_crps;
	INT_PTR ret = DialogBoxParam(
		gls_editorapp.mp_hinst,
		MAKEINTRESOURCE(MBE_DLG_BmpTS),
		gls_editorapp.mp_hwnd,
		(DLGPROC)&mbe_tsetview_bmptsdlg_dlgproc,
		(LPARAM)&s_crps
	);
	
	/* Create new tileset from bitmap. */
	if (ret == TRUE) {
		marble_ecode_t ecode = mbe_tsetview_internal_createtsfrombmp(
			ps_tsetview,
			&s_crps,
			&ps_tsetview->mas_ts[ps_tsetview->m_nts]
		);
		if (ecode != MARBLE_EC_OK)
			return ecode;

		ShowWindow(ps_tsetview->mp_hwnd, SW_SHOW);
		mbe_tsetview_setpage(ps_tsetview, ps_tsetview->m_curtsi);
	}

	return MARBLE_EC_OK;
}

void mbe_tsetview_setpage(
	struct mbe_tsetview *ps_tsetview, 
	int index
) {
	if (ps_tsetview == NULL || ps_tsetview->m_isinit == FALSE)
		return;

	/* Get currently visible page. */
	struct mbe_tset *ps_oldts = &ps_tsetview->mas_ts[ps_tsetview->m_curtsi];

	/* Hide it. */
	UpdateWindow(ps_oldts->p_hwnd);
	ShowWindow(ps_oldts->p_hwnd, SW_HIDE);

	/* Update selection and show the new page. */
	ps_tsetview->m_curtsi = index;

	RECT s_parentsize;
	mbe_tsetview_internal_getrect(ps_tsetview, &s_parentsize);

	SetWindowPos(
		ps_tsetview->mas_ts[index].p_hwnd,
		NULL,
		s_parentsize.left,
		s_parentsize.top,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top,
		SWP_NOACTIVATE
	);
	UpdateWindow(ps_tsetview->mas_ts[index].p_hwnd);
	ShowWindow(ps_tsetview->mas_ts[index].p_hwnd, SW_SHOW);
}


