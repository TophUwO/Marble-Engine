#pragma once

#include <editor.h>
#include <tsetview.h>


#pragma region CREATENEWTS-DLG
/*
 * Structure used for "Create new tileset ..." dialog.
 * Logical tileset will be created based on these
 * parameters.
 */
struct mbeditor_tset_emptytsdlg_createparams {
	TCHAR maz_name[MBE_MAXTSNAME]; /* tileset name */
	TCHAR maz_cmt[MBE_MAXCMT];     /* comment/description */

	int  m_width;   /* width, in tiles */
	int  m_height;  /* height, in tiles */
	int  m_tsize;   /* tile size, in pixels */
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
		COLORREF m_col; /* as integer */
	};
} ms_crps;


/*
 * Table representing the settings for the
 * spin controls. 
 */
struct { int m_id, m_buddy; SHORT m_pos, m_min, m_max; size_t m_size, m_off; } const glas_udopts[] = {
	{ EmptyTSDlg_SPIN_Width,  EmptyTSDlg_EDIT_Width,  1, 1, 1024, sizeof(int),  offsetof(struct mbeditor_tset_emptytsdlg_createparams, m_width)  },
	{ EmptyTSDlg_SPIN_Height, EmptyTSDlg_EDIT_Height, 1, 1, 1024, sizeof(int),  offsetof(struct mbeditor_tset_emptytsdlg_createparams, m_height) },
	{ EmptyTSDlg_SPIN_TSize,  EmptyTSDlg_EDIT_TSize,  8, 1, 128,  sizeof(int),  offsetof(struct mbeditor_tset_emptytsdlg_createparams, m_tsize)  },
	{ EmptyTSDlg_SPIN_Red,    EmptyTSDlg_EDIT_Red,    0, 0, 255,  sizeof(BYTE), offsetof(struct mbeditor_tset_emptytsdlg_createparams, m_r)      },
	{ EmptyTSDlg_SPIN_Green,  EmptyTSDlg_EDIT_Green,  0, 0, 255,  sizeof(BYTE), offsetof(struct mbeditor_tset_emptytsdlg_createparams, m_g)      },
	{ EmptyTSDlg_SPIN_Blue,   EmptyTSDlg_EDIT_Blue,   0, 0, 255,  sizeof(BYTE), offsetof(struct mbeditor_tset_emptytsdlg_createparams, m_b)      }
};
size_t const gl_nudopts = ARRAYSIZE(glas_udopts);

/*
 * Table representing maximum character counts
 * for all text fields of the dialog.
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
 * of imput, that is, max. number of characters, range,
 * etc.
 * 
 * Returns nothing.
 */
static void mbeditor_tsetview_emptytsdlg_setupcontrols(
	HWND p_hwnd /* dialog window */
) {
	/* Set the edit control limits. */
	for (size_t i = 0; i < gl_neclimits; i++)
		SendMessage(
			GetDlgItem(p_hwnd, glas_eclimits[i].m_id),
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
		HWND const p_hdlgitem = GetDlgItem(p_hwnd, glas_udopts[i].m_id);

		SendMessage(p_hdlgitem, UDM_SETPOS, 0, (LPARAM)glas_udopts[i].m_pos);
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
static void mbeditor_tsetview_emptytsdlg_setcoltransparent(
	HWND p_hwnd,                                           /* dialog window */
	struct mbeditor_tset_emptytsdlg_createparams *ps_crps, /* create params */
	BOOL istrans                                           /* new transparent setting */
) {
	/* Update internal state. */
	ps_crps->m_istrans = istrans;

	/* Disable necessary controls. */
	for (size_t i = 0; i < gl_nudopts; i++) {
		if (i < 3)
			continue;

		EnableWindow(GetDlgItem(p_hwnd, glas_udopts[i].m_id), !ps_crps->m_istrans);
		EnableWindow(GetDlgItem(p_hwnd, glas_udopts[i].m_buddy), !ps_crps->m_istrans);
	}
	EnableWindow(GetDlgItem(p_hwnd, EmptyTSDlg_BTN_ChooseCol), !ps_crps->m_istrans);

	/* Update checkbox state. */
	CheckDlgButton(p_hwnd, EmptyTSDlg_CB_Transparent, (UINT)ps_crps->m_istrans);
}

/*
 * Sets dialog fields to specific values.
 * 
 * Returns nothing.
 */
static void mbeditor_tsetview_emptytsdlg_setfields(
	HWND p_hwnd,                                          /* dialog window */
	struct mbeditor_tset_emptytsdlg_createparams *ps_crps /* create parameters */
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
			GetDlgItem(p_hwnd, as_strreps[i].m_id),
			as_strreps[i].pz_str
		);

	/*
	 * Solely setting the up-down control position is enough, as the up-down controls
	 * all have a "buddy" registered which will be automatically notified of the
	 * position change and, if needed, will subsequently update its caption.
	 */
	SendMessage(GetDlgItem(p_hwnd, EmptyTSDlg_SPIN_Width), UDM_SETPOS, 0, (LPARAM)ps_crps->m_width);
	SendMessage(GetDlgItem(p_hwnd, EmptyTSDlg_SPIN_Height), UDM_SETPOS, 0, (LPARAM)ps_crps->m_height);
	SendMessage(GetDlgItem(p_hwnd, EmptyTSDlg_SPIN_TSize), UDM_SETPOS, 0, (LPARAM)ps_crps->m_tsize);
	SendMessage(GetDlgItem(p_hwnd, EmptyTSDlg_SPIN_Red), UDM_SETPOS, 0, (LPARAM)ps_crps->m_r);
	SendMessage(GetDlgItem(p_hwnd, EmptyTSDlg_SPIN_Green), UDM_SETPOS, 0, (LPARAM)ps_crps->m_g);
	SendMessage(GetDlgItem(p_hwnd, EmptyTSDlg_SPIN_Blue), UDM_SETPOS, 0, (LPARAM)ps_crps->m_b);

	/* Set checkbox states. */
	mbeditor_tsetview_emptytsdlg_setcoltransparent(p_hwnd, ps_crps, ps_crps->m_istrans);
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
static BOOL mbeditor_tsetview_emptytsdlg_handlecolordlg(
	struct mbeditor_tset_emptytsdlg_createparams *ps_crps /* create parameters */
) {
	static COLORREF a_custcol[16] = { 0 };

	CHOOSECOLOR s_ccol = {
		.lStructSize  = sizeof s_ccol,
		.hwndOwner    = NULL,
		.lpCustColors = (LPDWORD)a_custcol,
		.rgbResult    = RGB(255, 255, 255),
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
 * Updates the internal create parameters just before the dialog closes.
 * 
 * Returns nothing.
 */
static void mbeditor_tsetview_emptytsdlg_writebackvalues(
	HWND p_hwnd,                                          /* dialog window */
	struct mbeditor_tset_emptytsdlg_createparams *ps_crps /* destination */
) {
	GetWindowText(GetDlgItem(p_hwnd, EmptyTSDlg_EDIT_Name), ps_crps->maz_name, MBE_MAXTSNAME);
	GetWindowText(GetDlgItem(p_hwnd, EmptyTSDlg_EDIT_Comment), ps_crps->maz_cmt, MBE_MAXCMT);

	/*
	 * Only issue an update if the values are needed
	 * to create the tileset later.
	 */
	ps_crps->m_istrans = Button_GetCheck(GetDlgItem(p_hwnd, EmptyTSDlg_CB_Transparent));
	if (ps_crps->m_istrans == FALSE)
		for (size_t i = 0; i < gl_nudopts; i++) {
			UINT res = SendMessage(GetDlgItem(p_hwnd, glas_udopts[i].m_id), UDM_GETPOS, 0, 0) & 0xFFFF;

			CopyMemory((void *)((char *)ps_crps + glas_udopts[i].m_off), &res, glas_udopts[i].m_size);
		}
}

static BOOL CALLBACK mbeditor_tsetview_emptytsdlg_dlgproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	MBE_WNDUSERDATA(ps_param, struct mbeditor_tset_emptytsdlg_createparams *);

	/* Default settings for this dialog. */
	static struct mbeditor_tset_emptytsdlg_createparams const s_defcps = {
		.maz_name  = TEXT("unnamed"),
		.maz_cmt   = TEXT(""),
		
		.m_col     = RGB(0, 0, 0),
		.m_istrans = FALSE,
		.ms_aspect = {
			.m_islock = FALSE,
			.m_aspect = 1.0f
		},
		.m_width   = 32,
		.m_height  = 32,
		.m_tsize   = 16
	};

	switch (msg) {
		case WM_INITDIALOG:
			/* Copy default values into the userdata pointer. */
			CopyMemory((void *)lparam, &s_defcps, sizeof s_defcps);
			/* Set userdata. */
			SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)lparam);

			/* Initialize dialog fields. */
			mbeditor_tsetview_emptytsdlg_setupcontrols(p_hwnd);
			mbeditor_tsetview_emptytsdlg_setfields(
				p_hwnd,
				(struct mbeditor_tset_emptytsdlg_createparams *)lparam
			);
			break;
		case WM_CLOSE:
			PostMessage(
				p_hwnd,
				WM_COMMAND,
				EmptyTSDlg_BTN_Cancel,
				0
			);

			break;
		case WM_COMMAND:
			switch (wparam) {
				case EmptyTSDlg_CB_Transparent:
					mbeditor_tsetview_emptytsdlg_setcoltransparent(p_hwnd, ps_param, !ps_param->m_istrans);

					break;
				case EmptyTSDlg_CB_LockAspect:
					if ((ps_param->ms_aspect.m_islock = !ps_param->ms_aspect.m_islock) == TRUE)
						ps_param->ms_aspect.m_aspect = (float)ps_param->m_width / (float)ps_param->m_height;
					else
						ps_param->ms_aspect.m_aspect = 1.0f;

					break;
				case EmptyTSDlg_BTN_ChooseCol:
					/* Open CHOOSECOLOR dialog. */
					if (mbeditor_tsetview_emptytsdlg_handlecolordlg(ps_param) == TRUE) {
						/*
						 * Update positions of spin controls. Note that this will automatically
						 * change the captions of the corresponding edit controls accordingly.
						 */
						SendMessage(GetDlgItem(p_hwnd, EmptyTSDlg_SPIN_Red), UDM_SETPOS, 0, ps_param->m_r);
						SendMessage(GetDlgItem(p_hwnd, EmptyTSDlg_SPIN_Green), UDM_SETPOS, 0, ps_param->m_g);
						SendMessage(GetDlgItem(p_hwnd, EmptyTSDlg_SPIN_Blue), UDM_SETPOS, 0, ps_param->m_b);
					}

					break;
				case EmptyTSDlg_BTN_Reset:
					/* Copy default values into the userdata pointer. */
					CopyMemory(ps_param, &s_defcps, sizeof s_defcps);

					mbeditor_tsetview_emptytsdlg_setfields(p_hwnd, ps_param);

					break;
				case EmptyTSDlg_BTN_Ok:
				case EmptyTSDlg_BTN_Cancel:
					if (wparam == EmptyTSDlg_BTN_Ok)
						mbeditor_tsetview_emptytsdlg_writebackvalues(p_hwnd, ps_param);

					EndDialog(
						p_hwnd,
						wparam == EmptyTSDlg_BTN_Ok
					);

					break;
			}

			break;
	}

	return FALSE;
}
#pragma endregion


#pragma region TSVIEW-CTRL
static TCHAR const *const glpz_tsviewwndclname = TEXT("mbeditor_tsview");

/*
 * Window procedure for single tileset views.
 * Note that this is not for the container of the view which
 * is a tab control, but for the controls that display
 * the tiles themselves.
 */
static LRESULT CALLBACK mbeditor_tsetview_internal_wndproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	switch (msg) {

	}

	return DefWindowProc(p_hwnd, msg, wparam, lparam);
}

/*
 * Creates a logical tileset.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t mbeditor_tsetview_internal_createemptyts(
	struct mbeditor_tsetview *ps_parent,                   /* parent window */
	struct mbeditor_tset_emptytsdlg_createparams *ps_crps, /* tileset parameters */
	struct mbeditor_tset *ps_tset                          /* destination */
) {
	if (ps_parent == NULL || ps_crps == NULL || ps_tset == NULL)
		return MARBLE_EC_INTERNALPARAM;

	/* Get current size of tab view. */
	RECT s_parentsize;
	GetClientRect(ps_parent->mp_hwnd, &s_parentsize);
	TabCtrl_AdjustRect(ps_parent->mp_hwnd, FALSE, &s_parentsize);

	/* Create window of tileset view. */
	ps_tset->p_hwnd = CreateWindowEx(
		WS_EX_WINDOWEDGE | WS_EX_COMPOSITED,
		glpz_tsviewwndclname,
		NULL,
		WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPSIBLINGS,
		s_parentsize.left,
		s_parentsize.top,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top,
		ps_parent->mp_hwnd,
		NULL,
		gls_editorapp.mp_hinst,
		(LPVOID)ps_tset
	);
	if (ps_tset->p_hwnd == NULL)
		return MARBLE_EC_CREATEWND;

	/* Insert tab. */
	TCITEM s_item = { .mask = TCIF_TEXT, .pszText = ps_crps->maz_name };
	TabCtrl_InsertItem(ps_parent->mp_hwnd, ps_parent->m_nts++, &s_item);
	return MARBLE_EC_OK;
}
#pragma endregion


marble_ecode_t mbeditor_tsetview_init(
	HWND p_hwndparent,
	struct mbeditor_tsetview *ps_tsetview
) {
	if (ps_tsetview == NULL || p_hwndparent == NULL) return MARBLE_EC_PARAM;
	if (ps_tsetview->m_isinit == TRUE)               return MARBLE_EC_COMPSTATE;

	/* Register tileset view window class. */
	WNDCLASSEX s_wndclass = {
		.cbSize        = sizeof s_wndclass,
		.hInstance     = gls_editorapp.mp_hinst,
		.lpszClassName = glpz_tsviewwndclname,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.lpfnWndProc   = (WNDPROC)&mbeditor_tsetview_internal_wndproc
	};
	if (RegisterClassEx(&s_wndclass) == FALSE)
		return MARBLE_EC_REGWNDCLASS;

	/* Create tab view window. */
	ps_tsetview->mp_hwnd = CreateWindowEx(
		WS_EX_WINDOWEDGE | WS_EX_COMPOSITED,
		WC_TABCONTROL,
		NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		0,
		0,
		200,
		700,
		p_hwndparent,
		NULL,
		gls_editorapp.mp_hinst,
		ps_tsetview
	);
	if (ps_tsetview->mp_hwnd == NULL)
		return MARBLE_EC_CREATEWND;

	/* Set tab text font. */
	SendMessage(
		ps_tsetview->mp_hwnd,
		WM_SETFONT,
		(WPARAM)gls_editorapp.ms_res.mp_hguifont,
		(LPARAM)TRUE
	);

	ps_tsetview->m_isinit = TRUE;
	return MARBLE_EC_OK;
}

void mbeditor_tsetview_uninit(
	struct mbeditor_tsetview *ps_tsetview
) {
	
}

void mbeditor_tsetview_resize(
	struct mbeditor_tsetview *ps_tsetview,
	int nwidth,
	int nheight
) {
	if (ps_tsetview == NULL || ps_tsetview->mp_hwnd == NULL)
		return;

	SetWindowPos(
		ps_tsetview->mp_hwnd,
		NULL,
		0,
		0,
		200,
		nheight,
		SWP_NOACTIVATE | SWP_NOMOVE
	);

	RECT s_parentsize;
	GetClientRect(ps_tsetview->mp_hwnd, &s_parentsize);
	TabCtrl_AdjustRect(ps_tsetview->mp_hwnd, FALSE, &s_parentsize);

	for (size_t i = 0; i < ps_tsetview->m_nts; i++)
		SetWindowPos(
			ps_tsetview->mas_tsets[i].p_hwnd,
			NULL,
			s_parentsize.left,
			s_parentsize.top,
			s_parentsize.right - s_parentsize.left,
			s_parentsize.bottom - s_parentsize.top,
			SWP_NOACTIVATE
		);
}

marble_ecode_t mbeditor_tsetview_newtsdlg(
	struct mbeditor_tsetview *ps_tsetview
) {
	if (ps_tsetview == NULL)
		return MARBLE_EC_PARAM;

	/*
	 * Checl whether the maximum number of tilesets per view
	 * has been reached. If yes, output error message and return.
	 * If not, open the dialog.
	 */
	if (ps_tsetview->m_nts == MBE_MAXTSC) {
		MessageBox(
			NULL,
			TEXT("Could not create tileset.\n    The maximum number of tilesets per view has been reached."),
			TEXT("Error"),
			MB_ICONERROR | MB_OK
		);

		return MARBLE_EC_COMPSTATE;
	}

	struct mbeditor_tset_emptytsdlg_createparams s_crps;
	if (DialogBoxParam(
		gls_editorapp.mp_hinst,
		MAKEINTRESOURCE(MBE_DLG_EmptyTS),
		gls_editorapp.mp_hwnd,
		(DLGPROC)&mbeditor_tsetview_emptytsdlg_dlgproc,
		(LPARAM)&s_crps
	) != FALSE) {
		marble_ecode_t ecode = mbeditor_tsetview_internal_createemptyts(
			ps_tsetview,
			&s_crps,
			&ps_tsetview->mas_tsets[ps_tsetview->m_nts]
		);
		if (ecode != MARBLE_EC_OK)
			return ecode;

		++ps_tsetview->m_nts;
	}

	return MARBLE_EC_OK;
}


