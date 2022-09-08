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

	TCHAR maz_name[MBE_MAXIDLEN];  /* tileset name */
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
	{ EmptyTSDlg_EDIT_Name,    MBE_MAXIDLEN - 1  },
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
						GetWindowText(MBE_DLGWND(EmptyTSDlg_EDIT_Name), ps_param->maz_name, MBE_MAXIDLEN);
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


#pragma region RENAMETS-DLG
/*
 * Structure representing userdata for "Rename tileset view"
 * dialog.
 */
struct mbe_renametsdlg_cp {
	BOOL  m_renamefile;           /* rename file on disk? */
	TCHAR maz_name[MBE_MAXIDLEN]; /* new name */
};


/*
 * Checks whether a string that's about to be used as a filename
 * contains any invalid characters.
 * Invalid characters in all filenames are:
 *    - control characters
 *    - / \ : * ? " < > |
 * 
 * The input string must be NUL-terminated.
 * Note that Unicode control characters may also be invalid,
 * but are not caught by this function.
 * 
 * Returns TRUE if the filename is valid, 0 if not.
 */
static BOOL mbe_tsetview_renametsdlg_internal_valfname(
	TCHAR const *pz_fname /* string to validate */
) {
	if (pz_fname == NULL || *pz_fname == TEXT('\0'))
		return FALSE;

	TCHAR *p_res;
	p_res = _tcspbrk(
		pz_fname, 
		TEXT("/\\:*?\"<>|")
		TEXT("\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B")
		TEXT("\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16")
		TEXT("\x17\x18\x19\x1A\x1B\x1C\x1D\x1D\x1E\x1F\x7F")
	);

	return p_res == NULL;
}

/*
 * Window procedure for "Rename tileset view"
 * dialog.
 */
static BOOL CALLBACK mbe_tsetview_renametsdlg_dlgproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	MBE_WNDUSERDATA(ps_cps, struct mbe_renametsdlg_cp *);

	LRESULT res;
	BOOL dlgres = FALSE;

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)lparam);

			/*
			 * Set keyboard focus to the edit control
			 * containing the new name.
			 */
			SendMessage(
				p_hwnd,
				WM_NEXTDLGCTL,
				(WPARAM)MBE_DLGWND(RenameTSDLG_EDIT_NewName),
				TRUE
			);

			ZeroMemory((void *)lparam, sizeof(struct mbe_renametsdlg_cp));
			break;
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case RenameTSDLG_EDIT_NewName:
					switch (HIWORD(wparam)) {
						/*
						 * An edit control sends an "EN_CHANGE" notification
						 * via a WM_COMMAND message after its contents have changed.
						 * When that happens, we update the text inside our create-params
						 * structure.
						 */
						case EN_CHANGE:
							GetWindowText(
								MBE_DLGWND(RenameTSDLG_EDIT_NewName),
								ps_cps->maz_name,
								MBE_MAXIDLEN
							);

							break;
					}

					break;
				case RenameTSDLG_CB_RenameFile:
					res = SendDlgItemMessage(p_hwnd, RenameTSDLG_CB_RenameFile, BM_GETCHECK, 0, 0);

					ps_cps->m_renamefile = res != BST_UNCHECKED;
					break;
				case RenameTSDLG_BTN_Cancel:
				case RenameTSDLG_BTN_Apply:
					if (LOWORD(wparam) == RenameTSDLG_BTN_Apply) {
						/*
						 * Clicking "Apply" with an empty "File Name" field
						 * acts as if the user clicked the "Cancel" or close
						 * button of the dialog window.
						 */
						if (*ps_cps->maz_name == TEXT('\0')) {
							dlgres = FALSE;

							goto lbl_END;
						}

						BOOL isvalid = mbe_tsetview_renametsdlg_internal_valfname(ps_cps->maz_name);
						if (isvalid == FALSE) {
							MessageBox(
								p_hwnd,
								TEXT("File name contains invalid characters."),
								TEXT("Error"),
								MB_ICONWARNING | MB_OK
							);

							break;
						}

						dlgres = TRUE;
					}

				lbl_END:
					EndDialog(p_hwnd, dlgres);
					break;
			}

			break;
		case WM_CLOSE:
			/*
			 * Clicking the sysmenu close-button will act
			 * like a click on the "Cancel" button.
			 */
			SendMessage(
				p_hwnd,
				WM_COMMAND,
				MAKELONG(RenameTSDLG_BTN_Cancel, 0),
				0
			);

			break;
	}

	return FALSE;
}
#pragma endregion


#pragma region TILESET-CTRL
/*
 * Calculates the ratio between screen tilesize (**gl_viewtsize**), and physical
 * tilesize (**tssize**).
 */
#define MBE_TSSCALE(tssize) ((float)((gl_viewtsize / (float)(tssize))))


static int const gl_viewtsize = 32;
static TCHAR const *const glpz_tsviewwndclname = TEXT("mbe_tsview");

static void mbe_tsetview_internal_closetileset(
	struct mbe_tsetview *ps_tsetview,
	struct mbe_tset *ps_tset
);

/*
 * Creates a new, empty tileset.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t mbe_tset_internal_new(
	/*
	 * pointer to a tileset pointer which will
	 * receive the pointer to the newly-created
	 * tileset.
	 */
	struct mbe_tset **pps_tset
) {
	if (pps_tset == NULL)
		return MARBLE_EC_INTERNALPARAM;

	marble_ecode_t ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof **pps_tset,
		TRUE,
		FALSE,
		pps_tset
	);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	return MARBLE_EC_OK;
}

/*
 * Destroys a tileset.
 * The function will also deallocate all resources used by the tileset.
 * If the tileset is currently registered in a tileset view,
 * the view's tab will also be removed.
 * 
 * Returns nothing.
 */
static void __cdecl mbe_tset_internal_destroy(struct mbe_tset **pps_tset /* tileset to destroy */) {
	if (pps_tset == NULL)
		return;

	if ((*pps_tset)->m_isinit == TRUE) {
		/* Remove the tab from the tileset view container. */
		mbe_tsetview_internal_closetileset((*pps_tset)->mps_parent, *pps_tset);

		/* Delete GDI resources. */
		SelectObject((*pps_tset)->ms_res.mp_hbmpdc, (*pps_tset)->ms_res.mp_hbmpdcold);

		DeleteObject((*pps_tset)->ms_res.mp_hbmpdcbmp);
		DeleteDC((*pps_tset)->ms_res.mp_hbmpdc);
	}

	free(*pps_tset);
	*pps_tset = NULL;
}

/*
 * Get index of a tileset view in its current view
 * container.
 * 
 * Returns indes, or -1 if an error occurred.
 */
static size_t mbe_tset_internal_getindex(
	struct mbe_tset *ps_tset /* tileset to get index of */
) {
	if (ps_tset == NULL || ps_tset->m_isinit == FALSE)
		return -1;

	return marble_util_vec_find(
		ps_tset->mps_parent->mps_tsets,
		ps_tset,
		0,
		0
	);
}

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
 * Gets the nearest valid index to **index**. **index** is the index of
 * a tileset view which is about to get removed.
 * 
 * Returns index.
 */
static int mbe_tsetview_internal_getnearestpage(
	struct mbe_tsetview *ps_tsetview,
	int index
) {
	/*
	 * If the index that's about to get deleted is 0,
	 * check if there are pages loaded after it. If
	 * yes, return the next page, i.e. index 1.
	 * If not, return -1, signifying that the view
	 * container is now empty.
	 */
	if (index == 0)
		return ps_tsetview->m_nts < 2 ? -1 : 1;

	return index - 1;
}

/*
 * Sets up the internal memory DC of tileset view.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t mbe_tset_internal_setupdc(
	struct mbe_tset *ps_tset,         /* tileset */
	struct mbe_tsviewdlg_cp *ps_crps, /* create parameters */
	int nwidth,                       /* bitmap width */
	int nheight,                      /* bitmap height */
	HBITMAP p_hsrcbmp                 /* source bitmap */
) {
	if (ps_tset == NULL || ps_crps == NULL)
		return MARBLE_EC_PARAM;

	marble_ecode_t ecode = MARBLE_EC_OK;

	/* Get DC of Desktop window. */
	HDC p_hdc = GetDC(GetDesktopWindow());
	if (p_hdc == NULL)
		return MARBLE_EC_GETDC;

	/* Create DC compatible with the desktop window. */
	ps_tset->ms_res.mp_hbmpdc = CreateCompatibleDC(p_hdc);
	if (ps_tset->ms_res.mp_hbmpdc == NULL) {
		ecode = MARBLE_EC_CREATEMEMDC;

		goto lbl_END;
	}

	/* Calculate bitmap sizes. */
	ps_tset->ms_sz.m_tsize   = ps_crps->m_tsize;
	ps_tset->ms_sz.m_twidth  = nwidth / ps_crps->m_tsize;
	ps_tset->ms_sz.m_theight = nheight / ps_crps->m_tsize;
	ps_tset->ms_sz.m_pwidth  = nwidth / ps_crps->m_tsize * ps_crps->m_tsize;
	ps_tset->ms_sz.m_pheight = nheight / ps_crps->m_tsize * ps_crps->m_tsize;

	/*
	 * If the sizes we calculated equal to the physical
	 * bitmap size, the bitmap contains only full tiles.
	 * In this casse we do not have to copy it.
	 */
	if (nwidth == ps_tset->ms_sz.m_pwidth && nheight == ps_tset->ms_sz.m_pheight && p_hsrcbmp != NULL) {
		ps_tset->ms_res.mp_hbmpdcold = SelectObject(ps_tset->ms_res.mp_hbmpdc, p_hsrcbmp);

		goto lbl_END;
	}

	/* Create memory bitmap. */
	ps_tset->ms_res.mp_hbmpdcbmp = CreateCompatibleBitmap(
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
	if (ps_tset->ms_res.mp_hbmpdcbmp == NULL) {
		ecode = MARBLE_EC_CREATEMEMBITMAP;

		DeleteDC(ps_tset->ms_res.mp_hbmpdc);
		goto lbl_END;
	}

	/*
	 * Select newly-created bitmap into memory DC.
	 * The default bitmap is saved as it will be restored later
	 * when the memory device context is about to be deleted.
	 */
	ps_tset->ms_res.mp_hbmpdcold = SelectObject(ps_tset->ms_res.mp_hbmpdc, ps_tset->ms_res.mp_hbmpdcbmp);

	if (p_hsrcbmp != NULL) {
		HBITMAP p_hbmpold;
		HDC p_hdctmp;
		
		p_hdctmp = CreateCompatibleDC(p_hdc);
		if (p_hdctmp == NULL) {
			SelectObject(ps_tset->ms_res.mp_hbmpdc, ps_tset->ms_res.mp_hbmpdcold);
			
			DeleteObject(ps_tset->ms_res.mp_hbmpdcbmp);
			DeleteDC(ps_tset->ms_res.mp_hbmpdc);

			ecode = MARBLE_EC_CREATEMEMDC;
			goto lbl_END;
		}
		p_hbmpold = SelectObject(p_hdctmp, p_hsrcbmp);

		/*
		 * Copy-over the valid portion of the
		 * source bitmap.
		 */
		BitBlt(
			ps_tset->ms_res.mp_hbmpdc,
			0,
			0,
			ps_tset->ms_sz.m_pwidth,
			ps_tset->ms_sz.m_pheight,
			p_hdctmp,
			0,
			0,
			SRCCOPY
		);

		SelectObject(p_hdctmp, p_hbmpold);
		DeleteObject(p_hsrcbmp);
		DeleteDC(p_hdctmp);
	}

lbl_END:
	/* Release device context of desktop window. */
	ReleaseDC(GetDesktopWindow(), p_hdc);
	return ecode;
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
	if (ps_tset->ms_sz.m_pheight * MBE_TSSCALE(ps_tset->ms_sz.m_tsize) > nheight) {
		int const maxscr = max(ps_tset->ms_sz.m_pheight - (int)(nheight / MBE_TSSCALE(ps_tset->ms_sz.m_tsize)), 0); 

		ps_tset->ms_scr.ms_yscr = (SCROLLINFO){
			.cbSize = sizeof ps_tset->ms_scr.ms_yscr,
			.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS,
			.nMin   = 0,
			.nMax   = ps_tset->ms_sz.m_pheight - 1,
			.nPage  = (int)(nheight / MBE_TSSCALE(ps_tset->ms_sz.m_tsize)),
			.nPos   = min(ps_tset->ms_scr.ms_yscr.nPos, maxscr)
		};

		isyvisible = TRUE;
		SetScrollInfo(ps_tset->mp_hwnd, SB_VERT, &ps_tset->ms_scr.ms_yscr, TRUE);
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
		ps_tset->ms_scr.ms_yscr.fMask = SIF_POS;
		ps_tset->ms_scr.ms_yscr.nPos  = 0;
	}
	/* Submit new scrollbar info. */
	SetScrollInfo(ps_tset->mp_hwnd, SB_VERT, &ps_tset->ms_scr.ms_yscr, TRUE);

	/* Update horizontal scrollbar. */
	if (ps_tset->ms_sz.m_pwidth * MBE_TSSCALE(ps_tset->ms_sz.m_tsize) > nwidth) {
		int const maxscr = max(ps_tset->ms_sz.m_pwidth - (int)(nwidth / MBE_TSSCALE(ps_tset->ms_sz.m_tsize)), 0);

		ps_tset->ms_scr.ms_xscr = (SCROLLINFO){
			.cbSize = sizeof ps_tset->ms_scr.ms_xscr,
			.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS,
			.nMin   = 0,
			.nMax   = ps_tset->ms_sz.m_pwidth - 1,
			.nPage  = (int)(nwidth / MBE_TSSCALE(ps_tset->ms_sz.m_tsize)),
			.nPos   = min(ps_tset->ms_scr.ms_xscr.nPos, maxscr)
		};

		isxvisible = TRUE;
	} else {
		ps_tset->ms_scr.ms_xscr.fMask = SIF_POS;
		ps_tset->ms_scr.ms_xscr.nPos  = 0;
	}
	/* Submit new scrollbar info. */
	SetScrollInfo(ps_tset->mp_hwnd, SB_HORZ, &ps_tset->ms_scr.ms_xscr, TRUE);

	/* Update visible states of scrollbars. */
	ShowScrollBar(ps_tset->mp_hwnd, SB_HORZ, ps_tset->ms_scr.m_xscrv = isxvisible);
	ShowScrollBar(ps_tset->mp_hwnd, SB_VERT, ps_tset->ms_scr.m_yscrv = isyvisible);
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
#define MBE_ISUPSCRMSG(msg)   (msg == SB_PAGEUP || msg == SB_LINEUP)
#define MBE_ISDOWNSCRMSG(msg) (msg == SB_PAGEDOWN || msg == SB_LINEDOWN)

	int newpos;

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
	float fac = 1.0f;
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
		? &ps_tset->ms_scr.ms_xscr
		: &ps_tset->ms_scr.ms_yscr
	;

	/*
	 * If the current scrollbar is not visible, do not
	 * allow scrolling of the window by other means such
	 * as the mouse-wheel or even the keyboard.
	 */
	if (msg == WM_HSCROLL && ps_tset->ms_scr.m_xscrv == FALSE || msg == WM_VSCROLL && ps_tset->ms_scr.m_yscrv == FALSE)
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
			newpos = ps_scrinfo->nPos + (int)((float)gl_viewtsize / MBE_TSSCALE(ps_tset->ms_sz.m_tsize) * fac);

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
		ps_tset->mp_hwnd,
		msg == WM_HSCROLL
		? SB_HORZ
		: SB_VERT
		, ps_scrinfo,
		TRUE
	);

	InvalidateRect(ps_tset->mp_hwnd, NULL, TRUE);
}

/*
 * Calculates the bounding rectangle of the tile at (x, y),
 * in map coordinates, and writes the result into **ps_dest**.
 * 
 * Returns nothing.
 */
static void mbe_tsetview_internal_calctrect(
	int x,        /* x-coordinate of tile, in tile units */
	int y,        /* y-coordinate of tile, in tile units */
	RECT *ps_dest /* destination rectangle */
) {
	*ps_dest = (RECT){
		x       * gl_viewtsize,
		y       * gl_viewtsize,
		(x + 1) * gl_viewtsize,
		(y + 1) * gl_viewtsize
	};
}

/*
 * Calculates the smallest rectangle that completely encloses the diagonal
 * corners of the tile rectangles of the current selection and the incoming 
 * selection (destx, desty), i.e. the current selection rectangle.
 * The result gets written to **ps_dest**.
 * 
 * Returns nothing.
 */
static void mbe_tsetview_internal_calcboundrect(
	struct mbe_tset *ps_tset, /* tileset view (with current selection coordinates) */
	int newx,                 /* x-coordinate of incoming selection */
	int newy,                 /* y-coordinate of incoming selection */
	RECT *ps_dest             /* destination rectangle */
) {
	/*
	 * Calculate the pixel coordinates of the bounding rectangles of the
	 * currently selected tile and the incoming selected tile, relative
	 * to (0,0) of the tileset bitmap.
	 */
	RECT s_srect, s_drect;
	
	/* Calculate destination rectangle. */
	mbe_tsetview_internal_calctrect(newx, newy, &s_drect);

	/*
	 * If the SHIFT key is not held down (i.e. user does not want a
	 * rectangle-select), set the new selection rectangle to the one
	 * just calculated, and move on.
	 */
	if (gls_editorapp.ms_flags.mf_isshift == FALSE) {
		*ps_dest = s_drect;

		return;
	}

	/*
	 * If the user held down SHIFT, the user wants rectangle-select,
	 * so we have to calculate our source rectangle as well.
	 */
	mbe_tsetview_internal_calctrect(
		ps_tset->ms_sel.m_xindex,
		ps_tset->ms_sel.m_yindex,
		&s_srect
	);

	/*
	 * Write the result to **ps_dest**.
	 * 
	 * The idea is that we only have to know two (diagonal) points to
	 * be able to sort-of "interpolate" the other two.
	 * 
	 * Let
	 *     D              C
	 *      +------------+
	 *      |            |
	 *      |            |
	 *      +------------+ 
	 *     A              B
	 * 
	 * The individual points can then be determined with
	 *     A = { X(D), Y(B) }
	 *     B = { X(C), Y(A) }
	 *     C = { X(B), Y(D) }
	 *     D = { X(A), Y(C) }
	 * 
	 * The formula underneath will always calculate the upper-left and the bottom-right
	 * corner of the bounding rectangle, relative to (0,0) of the tileset bitmap.
	 */
	*ps_dest = (RECT){
		newx > ps_tset->ms_sel.m_xindex ? s_srect.left   : s_drect.left,
		newy > ps_tset->ms_sel.m_yindex ? s_srect.top    : s_drect.top,
		newx > ps_tset->ms_sel.m_xindex ? s_drect.right  : s_srect.right,
		newy > ps_tset->ms_sel.m_yindex ? s_drect.bottom : s_srect.bottom
	};
}

/*
 * Handles changing the current tile selection of
 * a tileset view.
 * 
 * Returns whether the message was handled
 * by this function or not.
 */
static BOOL mbe_tsetview_internal_handleselection(
	UINT msg,                /* message identifier */
	LPARAM lparam,           /* wndproc lparam */
	struct mbe_tset *ps_tset /* tileset view */
) {
	if (ps_tset == NULL)
		return FALSE;

	int reqx, reqy;
	RECT s_rnew;

	/* Calculate the indices of the tile the user clicked on. */
	reqx = (int)(ps_tset->ms_scr.ms_xscr.nPos * MBE_TSSCALE(ps_tset->ms_sz.m_tsize) + GET_X_LPARAM(lparam)) / gl_viewtsize;
	reqy = (int)(ps_tset->ms_scr.ms_yscr.nPos * MBE_TSSCALE(ps_tset->ms_sz.m_tsize) + GET_Y_LPARAM(lparam)) / gl_viewtsize;

	/*
	 * If requested selection is out of bounds,
	 * do not change the selection and return.
	 */
	if (mbe_tsetview_internal_isseloob(ps_tset, reqx, reqy) == TRUE)
		return FALSE;

	/* Calculate new selection bounding rectangle. */
	mbe_tsetview_internal_calcboundrect(ps_tset, reqx, reqy, &s_rnew);

	/*
	 * If the selection has not changed, i.e. the current and
	 * the new bounding rectangle of the selections do not
	 * differ, ignore the message and move on.
	 */
	if (memcmp(&s_rnew, &ps_tset->ms_sel.ms_rsel, sizeof s_rnew) == 0)
		return FALSE;

	/*
	 * Only update the origin of the current selection
	 * if the user did not make a rectangle-select via
	 * SHIFT + LMB.
	 */
	if (gls_editorapp.ms_flags.mf_isshift == FALSE) {
		ps_tset->ms_sel.m_xindex = reqx;
		ps_tset->ms_sel.m_yindex = reqy;
	}

	/* Update selection rectangle. */
	ps_tset->ms_sel.ms_rsel = s_rnew;

	InvalidateRect(ps_tset->mp_hwnd, NULL, TRUE);
	return TRUE;
}

/*
 * Handles the WM_CONTEXTMENU.
 * 
 * Returns nothing.
 */
static void mbe_tsetview_internal_handlecontextmenu(
	struct mbe_tset *ps_tset, /* tileset */
	WPARAM wparam,            /* WM_CONTEXTMENU wparam */
	LPARAM lparam             /* WM_CONTEXTMENU lparam */
) {
	if (ps_tset == NULL || ps_tset->m_isinit == FALSE || (HWND)wparam != ps_tset->mp_hwnd)
		return;

	HMENU p_hmenu, p_hdispmenu;

	/* Load menu from a resource. */
	p_hmenu = LoadMenu(gls_editorapp.mp_hinst, MAKEINTRESOURCE(MBE_TSetView_ContextMenu));
	if (p_hmenu == NULL)
		return;

	/*
	 * Get the sub-menu of the first menu item,
	 * which is going to be the context menu we
	 * will present to the user.
	 */
	p_hdispmenu = GetSubMenu(p_hmenu, 0);
	if (p_hdispmenu == NULL)
		goto lbl_END;

	/*
	 * Show context menu.
	 * 
	 * "TrackPopupMenu()" returns as soon as the user selected a
	 * menu item or the menu has been closed.
	 */
	TrackPopupMenu(
		p_hdispmenu,
		TPM_LEFTALIGN | TPM_LEFTBUTTON,
		GET_X_LPARAM(lparam),
		GET_Y_LPARAM(lparam),
		0,
		ps_tset->mp_hwnd,
		NULL
	);

lbl_END:
	DestroyMenu(p_hmenu);
}

/*
 * Closes a specific tileset view,
 * i.e. removes the tab.
 * 
 * Returns nothing.
 */
static void mbe_tsetview_internal_closetileset(
	struct mbe_tsetview *ps_tsetview, /* tileset view container */
	struct mbe_tset *ps_tset          /* tileset view to close */
) {
	if (ps_tsetview == NULL || ps_tsetview->m_isinit == FALSE || ps_tset == NULL || ps_tsetview->m_isinit == FALSE)
		return;

	size_t index = marble_util_vec_find(
		ps_tsetview->mps_tsets,
		ps_tset,
		0,
		0
	);
	if (index == (size_t)(-1))
		return;

	int newindex = mbe_tsetview_internal_getnearestpage(
		ps_tsetview,
		(int)index
	);
	/*
	 * BUGFIX (2022-08-31):
	 *   When the windows get destroyed at the end of the app's lifetime,
	 *   and we try change the selection to something else, it generates a
	 *   mysterious second WM_DESTROY for the window being currently selected,
	 *   causing memory to be freed twice.
	 *   Until the cause of this was found, we simply use a global flag to determine
	 *   *why* we call this function.
	 *   Possible scenarios are:
	 *     (1) because the user decided to close the tileset
	 *     (2) the user clicked the "Close" button of the main window
	 * 
	 * Only change selection if the user actively decided to close the tileset
	 * view.
	 */
	if (newindex != -1 && gls_editorapp.ms_flags.mf_isdest == FALSE) {
		TabCtrl_SetCurSel(ps_tsetview->mp_hwnd, newindex);
		
		/*
		 * As "TCM_SETCURSEL" does not send a WM_NOTIFY + TCN_SELCHANGE notification
		 * to the tileset view container's parent, we have to manually send it (which
		 * triggers hide-old show-new mechanism).
		 */
		NMHDR s_hdr = {
			.hwndFrom = ps_tsetview->mp_hwnd,
			.code     = TCN_SELCHANGE
		};
		SendMessage(
			GetParent(ps_tsetview->mp_hwnd),
			WM_NOTIFY,
			0,
			(LPARAM)&s_hdr
		);
	}

	/* Remove the tab. */
	TabCtrl_DeleteItem(ps_tsetview->mp_hwnd, index);
	--ps_tsetview->m_nts;
	marble_util_vec_erase(ps_tsetview->mps_tsets, index, FALSE);

	/*
	 * If the tileset view container is now empty,
	 * hide it.
	 */
	if (ps_tsetview->m_nts == 0) {
		UpdateWindow(ps_tsetview->mp_hwnd);

		ShowWindow(ps_tsetview->mp_hwnd, SW_HIDE);
	}
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
	INT_PTR ret;
	int orix, oriy, oldmode;
	TCITEM s_item;
	struct mbe_renametsdlg_cp s_crps;
	size_t index;

	switch (msg) {
		case WM_CREATE: MBE_SETUDATA(); return FALSE;
		case WM_SIZE:
			mbe_tset_internal_updatescrollbarinfo(
				ps_udata,
				GET_X_LPARAM(lparam),
				GET_Y_LPARAM(lparam)
			);

			return FALSE;
		case WM_ACTIVATE:
			if (wparam != WA_INACTIVE) {
				SetFocus(p_hwnd);

				return FALSE;
			}

			return FALSE;
		case WM_CONTEXTMENU:
			mbe_tsetview_internal_handlecontextmenu(
				ps_udata,
				wparam,
				lparam
			);

			return FALSE;
		case WM_KEYDOWN:
		case WM_HSCROLL:
		case WM_VSCROLL:
		case WM_MOUSEWHEEL:
			mbe_tsetview_internal_handlescrolling(
				msg,
				(LONG)wparam,
				ps_udata
			);

			return FALSE;
		case WM_LBUTTONDOWN:
			mbe_tsetview_internal_handleselection(
				msg,
				lparam,
				ps_udata
			);

			return FALSE;
		case WM_PAINT:
			p_hdc = BeginPaint(p_hwnd, &s_ps);

			if (ps_udata->m_isinit == FALSE)
				goto lbl_ENDPAINT;

			/* Get current dimensions of tileset view. */
			GetClientRect(p_hwnd, &s_rect);

			/* Draw tileset bitmap. */
			if (ps_udata->ms_sz.m_tsize == gl_viewtsize) {
				/*
				 * If the screen tilesize is the same as the physical
				 * tilesize, simply run a bitblit (faster).
				 */
				BitBlt(
					p_hdc,
					s_rect.left,
					s_rect.top,
					s_rect.right - s_rect.left,
					s_rect.bottom - s_rect.top,
					ps_udata->ms_res.mp_hbmpdc,
					ps_udata->ms_scr.ms_xscr.nPos,
					ps_udata->ms_scr.ms_yscr.nPos,
					SRCCOPY
				);
			} else {
				/*
				 * If the DC obtained by "BeginPaint()" gets modified by the user,
				 * the user has to restore it to its original state before calling
				 * "EndPaint()".
				 * We have to track all changes we make to **p_hdc** and manually
				 * revert them later.
				 */
				oldmode = SetStretchBltMode(p_hdc, COLORONCOLOR);

				/*
				 * Draw the currently visible portion of the tileset tile by
				 * tile. This avoids interpolation inaccuracies that a single
				 * "StretchBlt()" would have if the magnification is very high.
				 */
				for (int x = 0, xmpos = ps_udata->ms_scr.ms_xscr.nPos; x < s_rect.right; x += gl_viewtsize, xmpos += ps_udata->ms_sz.m_tsize)
					for (int y = 0, ympos = ps_udata->ms_scr.ms_yscr.nPos; y < s_rect.bottom; y += gl_viewtsize, ympos += ps_udata->ms_sz.m_tsize)
						StretchBlt(
							p_hdc,
							x,
							y,
							gl_viewtsize,
							gl_viewtsize,
							ps_udata->ms_res.mp_hbmpdc,
							xmpos,
							ympos,
							ps_udata->ms_sz.m_tsize,
							ps_udata->ms_sz.m_tsize,
							SRCCOPY
						);

				/* Restore old blit-mode. */
				SetStretchBltMode(p_hdc, oldmode);
			}

			p_hpold = SelectObject(p_hdc, gls_editorapp.ms_res.mp_hpgrid);
			/*
			 * Set the BkMode to transparent, otherwise the blank spaces between
			 * the dots of the grid lines will be rendered white. 
			 */
			oldmode = SetBkMode(p_hdc, TRANSPARENT);

			/*
			 * Calculate the offset of the topmost visible tile's
			 * top-left border from the top-left border of the
			 * tileset view. This is used as an offset factor for
			 * drawing the grid.
			 */
			orix = -(int)(ps_udata->ms_scr.ms_xscr.nPos * MBE_TSSCALE(ps_udata->ms_sz.m_tsize)) % gl_viewtsize;
			oriy = -(int)(ps_udata->ms_scr.ms_yscr.nPos * MBE_TSSCALE(ps_udata->ms_sz.m_tsize)) % gl_viewtsize;

			/* Draw vertical grid lines. */
			for (int x = orix; x < s_rect.right; x += gl_viewtsize) {
				MoveToEx(p_hdc, x, oriy, NULL);

				LineTo(p_hdc, x, s_rect.bottom);
			}

			/* Draw horizontal grid lines. */
			for (int y = oriy; y < s_rect.bottom; y += gl_viewtsize) {
				MoveToEx(p_hdc, orix, y, NULL);

				LineTo(p_hdc, s_rect.right, y);
			}

			/* Restore previous mode. */
			SetBkMode(p_hdc, oldmode);

			/* Draw selection rectangle. */
			SelectPen(p_hdc, gls_editorapp.ms_res.mp_hpsel);
			/*
			 * By default, the rectangle drawn by "Rectangle()" gets
			 * outlined with the current pen, and filled with the
			 * current brush. Because we cannot create a transparent 
			 * brush directly, we have to get a predefined object (NULL_BRUSH)
			 * that basically acts as an invisible brush, allowing us to
			 * draw rectangles with no fill color using "Rectangle()". 
			 */
			p_hbrold = SelectBrush(p_hdc, GetStockObject(NULL_BRUSH));

			Rectangle(
				p_hdc,
				ps_udata->ms_sel.ms_rsel.left   - (int)(ps_udata->ms_scr.ms_xscr.nPos * MBE_TSSCALE(ps_udata->ms_sz.m_tsize)) + 1,
				ps_udata->ms_sel.ms_rsel.top    - (int)(ps_udata->ms_scr.ms_yscr.nPos * MBE_TSSCALE(ps_udata->ms_sz.m_tsize)) + 1,
				ps_udata->ms_sel.ms_rsel.right  - (int)(ps_udata->ms_scr.ms_xscr.nPos * MBE_TSSCALE(ps_udata->ms_sz.m_tsize)) + 1,
				ps_udata->ms_sel.ms_rsel.bottom - (int)(ps_udata->ms_scr.ms_yscr.nPos * MBE_TSSCALE(ps_udata->ms_sz.m_tsize)) + 1
			);

			SelectPen(p_hdc, p_hpold);
			SelectBrush(p_hdc, p_hbrold);

		lbl_ENDPAINT:
			EndPaint(p_hwnd, &s_ps);
			return FALSE;
		case WM_COMMAND:
			switch (wparam) {
				case MBE_TSetMenu_Reload:
					break;
				case MBE_TSetMenu_Rename:
					ret = DialogBoxParam(
						gls_editorapp.mp_hinst,
						MAKEINTRESOURCE(MBE_DLG_RenameTS),
						p_hwnd,
						(DLGPROC)&mbe_tsetview_renametsdlg_dlgproc,
						(LPARAM)&s_crps
					);
					if (ret == FALSE)
						break;

					/* Rename tab of tileset view. */
					s_item = (TCITEM){
						.mask    = TCIF_TEXT,
						.pszText = s_crps.maz_name
					};
					index = mbe_tset_internal_getindex(ps_udata);
					if (index == -1)
						break;

					/* Issue rename command. */
					TabCtrl_SetItem(
						ps_udata->mps_parent->mp_hwnd,
						index,
						&s_item
					);

					/*
					 * Rename the file on disk if the user
					 * wishes to do so.
					 */
					if (s_crps.m_renamefile == TRUE) {
					
					}
					
					break;
				case MBE_TSetMenu_Close:
					DestroyWindow(p_hwnd);

					break;
			}

			return FALSE;
		case WM_DESTROY:
			mbe_tset_internal_destroy(&ps_udata);

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
	if (TabCtrl_InsertItem(ps_parent->mp_hwnd, ps_parent->m_nts, &s_item) == -1)
		return MARBLE_EC_UNKNOWN;

	/* Get current size of tab view. */
	mbe_tsetview_internal_getrect(ps_parent, s_parentsize);

	/* Create window of tileset view. */
	ps_tset->mp_hwnd = CreateWindowEx(
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
	if (ps_tset->mp_hwnd == NULL) {
		TabCtrl_DeleteItem(
			ps_parent->mp_hwnd,
			ps_parent->m_nts
		);

		return MARBLE_EC_CREATEWND;
	}

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

	/*
	 * Send a fake WM_LBUTTONDOWN that simulates a mouse
	 * click on bitmap position (0,0), essentially placing
	 * the selection on the tile in the top-left corner.
	 */
	SendMessage(ps_tset->mp_hwnd, WM_LBUTTONDOWN, 0, 0);
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
	/*ecode = mbe_tset_internal_setupdc(ps_tset, &ps_crps->_base);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;*/

	/* Initialize scrollbars. */
	mbe_tset_internal_updatescrollbarinfo(
		ps_tset,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top
	);
	/* Initialize selection. */
	mbe_tsetview_internal_initselection(ps_tset);

	/* Update init state. */
	ps_tset->m_isinit = TRUE;

lbl_END:
	if (ecode != MARBLE_EC_OK)
		if (ps_tset->mp_hwnd != NULL)
			DestroyWindow(ps_tset->mp_hwnd);

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
	HBITMAP p_hbmp;
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

	/* Get info about the loaded bitmap. */
	GetObject(p_hbmp, sizeof s_info, &s_info);

	/* Calculate bitmap size and set-up a memory DC. */
	ecode = mbe_tset_internal_setupdc(
		ps_tset,
		&ps_crps->_base,
		s_info.bmiHeader.biWidth,
		s_info.bmiHeader.biHeight,
		p_hbmp
	);
	if (ecode != MARBLE_EC_OK) {
		DeleteObject(p_hbmp);

		return ecode;
	}

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
		MBE_MAXIDLEN,
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
		goto lbl_END;

	/* Create tileset bitmap from source bitmap. */
	ecode = mbe_tsetview_internal_createtsbmpfromsrc(
		ps_parent,
		ps_tset,
		ps_crps
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* Initialize scrollbars. */
	mbe_tset_internal_updatescrollbarinfo(
		ps_tset,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top
	);

	/* Initialize selection. */
	mbe_tsetview_internal_initselection(ps_tset);

	/* Update init state. */
	ps_tset->m_isinit   = TRUE;
	ps_tset->mps_parent = ps_parent;

lbl_END:
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

	ecode = marble_util_vec_create(
		0,
		NULL,
		&ps_tsetview->mps_tsets
	);

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

		ZeroMemory(ps_tsetview, sizeof *ps_tsetview);
	}

	return ecode;
}

void mbe_tsetview_uninit(struct mbe_tsetview *ps_tsetview) {
	if (ps_tsetview == NULL || ps_tsetview->m_isinit == FALSE)
		return;

	/* Free all resources used by all tilesets. */
	marble_util_vec_destroy(&ps_tsetview->mps_tsets);

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

	struct mbe_tset *ps_curts = marble_util_vec_get(ps_tsetview->mps_tsets, ps_tsetview->m_curtsi);
	if (ps_curts == NULL)
		return;

	SetWindowPos(
		ps_curts->mp_hwnd,
		NULL,
		s_parentsize.left,
		s_parentsize.top,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top,
		SWP_NOACTIVATE
	);
}

marble_ecode_t mbe_tsetview_newtsdlg(struct mbe_tsetview *ps_tsetview) {
	if (ps_tsetview == NULL || ps_tsetview->m_isinit == FALSE)
		return MARBLE_EC_PARAM;

	marble_ecode_t ecode = MARBLE_EC_OK;

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
		struct mbe_tset *ps_tset;
		ecode = mbe_tset_internal_new(&ps_tset);
		if (ecode != MARBLE_EC_OK)
			return ecode;

		ecode = mbe_tsetview_internal_createemptyts(
			ps_tsetview,
			&s_crps,
			ps_tset
		);
		if (ecode != MARBLE_EC_OK) {
			mbe_tset_internal_destroy(&ps_tset);

			return ecode;
		}

		ShowWindow(ps_tsetview->mp_hwnd, SW_SHOW);
		mbe_tsetview_setpage(ps_tsetview, ps_tsetview->m_curtsi);
	}

	return MARBLE_EC_OK;
}

marble_ecode_t mbe_tsetview_bmptsdlg(struct mbe_tsetview *ps_tsetview) {
	if (ps_tsetview == NULL)
		return MARBLE_EC_PARAM;

	marble_ecode_t ecode = MARBLE_EC_OK;

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
		struct mbe_tset *ps_tset;
		ecode = mbe_tset_internal_new(&ps_tset);
		if (ecode != MARBLE_EC_OK)
			return ecode;

		ecode = mbe_tsetview_internal_createtsfrombmp(
			ps_tsetview,
			&s_crps,
			ps_tset
		);
		if (ecode != MARBLE_EC_OK)
			goto lbl_END;

		ecode = marble_util_vec_pushback(ps_tsetview->mps_tsets, ps_tset);
		if (ecode != MARBLE_EC_OK)
			goto lbl_END;

		/* Update tileset count of container. */
		++ps_tsetview->m_nts;

		/*
		 * Show the tileset view container if it isn't
		 * already visible. 
		 */
		ShowWindow(ps_tsetview->mp_hwnd, SW_SHOW);
		/*
		 * By default, when a page gets added to a tab view,
		 * the page will be changed to the newly-created one.
		 * We do not want that as it may throw off the user.
		 * 
		 * To evade this problem, after creating the tab, we
		 * just change the page back to the tab that was selected
		 * just before the new tileset was created.
		 */
		mbe_tsetview_setpage(ps_tsetview, ps_tsetview->m_curtsi);

	lbl_END:
		if (ecode != MARBLE_EC_OK) {
			DestroyWindow(ps_tset->mp_hwnd);

			return ecode;
		}
	}

	return MARBLE_EC_OK;
}

void mbe_tsetview_setpage(
	struct mbe_tsetview *ps_tsetview, 
	int index
) {
	if (ps_tsetview == NULL || ps_tsetview->m_isinit == FALSE)
		return;

	/*
	 * Get currently visible page and
	 * the page that's about to become
	 * visible.
	 */
	struct mbe_tset *ps_oldts = marble_util_vec_get(ps_tsetview->mps_tsets, ps_tsetview->m_curtsi);
	struct mbe_tset *ps_newts = marble_util_vec_get(ps_tsetview->mps_tsets, index);
	if (ps_oldts == NULL || ps_newts == NULL)
		return;

	/* Hide it. */
	UpdateWindow(ps_oldts->mp_hwnd);
	ShowWindow(ps_oldts->mp_hwnd, SW_HIDE);

	/* Update selection and show the new page. */
	ps_tsetview->m_curtsi = index;

	/*
	 * When the tileset view container gets resized,
	 * only the currently visible page will resize with
	 * the control.
	 * When the page gets changed, the now visible page
	 * may have wrong dimensions, so we query the size
	 * of the view container and resize the page accordingly. 
	 */
	RECT s_parentsize;
	mbe_tsetview_internal_getrect(ps_tsetview, &s_parentsize);

	SetWindowPos(
		ps_newts->mp_hwnd,
		NULL,
		s_parentsize.left,
		s_parentsize.top,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top,
		SWP_NOACTIVATE
	);
	UpdateWindow(ps_newts->mp_hwnd);
	ShowWindow(ps_newts->mp_hwnd, SW_SHOW);
}


