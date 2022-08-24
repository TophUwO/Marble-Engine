#pragma once

#include <editor.h>
#include <tsetview.h>


#pragma region CREATENEWTS-DLG
/*
 * Structure used for "Create new tileset ..." dialog.
 * Logical tileset will be created based on these
 * parameters.
 */
struct mbe_dlgnewts_cp {
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
		COLORREF m_col; /* color as integer value */
	};
} ms_crps;


/*
 * Table representing the settings for the
 * spin controls. Also contains information useful
 * to simplify applying effects to various controls
 * at once.
 */
struct { int m_id, m_buddy; SHORT m_pos, m_min, m_max; size_t m_size, m_off; } const glas_udopts[] = {
	{ EmptyTSDlg_SPIN_Width,  EmptyTSDlg_EDIT_Width,  1, 1, 1024, sizeof(int),  offsetof(struct mbe_dlgnewts_cp, m_width)  },
	{ EmptyTSDlg_SPIN_Height, EmptyTSDlg_EDIT_Height, 1, 1, 1024, sizeof(int),  offsetof(struct mbe_dlgnewts_cp, m_height) },
	{ EmptyTSDlg_SPIN_TSize,  EmptyTSDlg_EDIT_TSize,  8, 1, 128,  sizeof(int),  offsetof(struct mbe_dlgnewts_cp, m_tsize)  },
	{ EmptyTSDlg_SPIN_Red,    EmptyTSDlg_EDIT_Red,    0, 0, 255,  sizeof(BYTE), offsetof(struct mbe_dlgnewts_cp, m_r)      },
	{ EmptyTSDlg_SPIN_Green,  EmptyTSDlg_EDIT_Green,  0, 0, 255,  sizeof(BYTE), offsetof(struct mbe_dlgnewts_cp, m_g)      },
	{ EmptyTSDlg_SPIN_Blue,   EmptyTSDlg_EDIT_Blue,   0, 0, 255,  sizeof(BYTE), offsetof(struct mbe_dlgnewts_cp, m_b)      }
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
 * of imput, that is, max. number of characters, range,
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
	SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Width), UDM_SETPOS, 0, (LPARAM)ps_crps->m_width);
	SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_Height), UDM_SETPOS, 0, (LPARAM)ps_crps->m_height);
	SendMessage(MBE_DLGWND(EmptyTSDlg_SPIN_TSize), UDM_SETPOS, 0, (LPARAM)ps_crps->m_tsize);
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
static void mbe_dlgnewts_writeback(
	HWND p_hwnd,                    /* dialog window */
	struct mbe_dlgnewts_cp *ps_crps /* destination */
) {
	GetWindowText(MBE_DLGWND(EmptyTSDlg_EDIT_Name), ps_crps->maz_name, MBE_MAXTSNAME);
	GetWindowText(MBE_DLGWND(EmptyTSDlg_EDIT_Comment), ps_crps->maz_cmt, MBE_MAXCMT);

	/*
	 * Only issue an update if the values are needed
	 * to create the tileset later.
	 */
	ps_crps->m_istrans = Button_GetCheck(GetDlgItem(p_hwnd, EmptyTSDlg_CB_Transparent));
	if (ps_crps->m_istrans == FALSE)
		for (size_t i = 0; i < gl_nudopts; i++) {
			UINT res = SendMessage(MBE_DLGWND(glas_udopts[i].m_id), UDM_GETPOS, 0, 0) & 0xFFFF;

			CopyMemory((void *)((char *)ps_crps + glas_udopts[i].m_off), &res, glas_udopts[i].m_size);
		}
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
						ps_param->ms_aspect.m_aspect = (float)ps_param->m_width / (float)ps_param->m_height;
					else
						ps_param->ms_aspect.m_aspect = 1.0f;

					break;
				case EmptyTSDlg_BTN_ChooseCol:
					/* Open CHOOSECOLOR dialog. */
					if (mbe_dlgnewts_handlecolordlg(ps_param) == TRUE) {
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
					 * If the user presses the "OK" button, issue a final
					 * update to the internal create-params structure.
					 */
					if (wparam == EmptyTSDlg_BTN_Ok)
						mbe_dlgnewts_writeback(p_hwnd, ps_param);

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


#pragma region TILESET-CTRL
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
 * Draws an empty grid (1 px. wide, black) on a white
 * background in the memory device context specified.
 * 
 * Returns nothing.
 */
static void mbe_tset_internal_drawgrid(struct mbe_tset *ps_tset /* tileset */) {
	HBRUSH p_hbr, p_hbrold;
	HPEN   p_hp,  p_hpold;

	/* Create required resources. */
	p_hbr = CreateSolidBrush(RGB(255, 255, 255) /* solid white */);
	p_hp  = CreatePen(PS_SOLID, 1, RGB(0, 0, 0) /* solid black */);

	/* Select resources into device context. */
	p_hbrold = SelectObject(ps_tset->ms_res.p_hbmpdc, p_hbr);
	p_hpold  = SelectObject(ps_tset->ms_res.p_hbmpdc, p_hp);

	/* Fill the entire bitmap with solid white. */
	RECT s_rect = {
		0,
		0,
		ps_tset->ms_sz.m_pwidth,
		ps_tset->ms_sz.m_pheight
	};
	FillRect(ps_tset->ms_res.p_hbmpdc, &s_rect, p_hbr);

	/* Draw vertical grid lines. */
	for (int i = 0; i < ps_tset->ms_sz.m_pwidth; i += 33) {
		MoveToEx(ps_tset->ms_res.p_hbmpdc, i, 0, NULL);

		/* Draw line. */
		LineTo(ps_tset->ms_res.p_hbmpdc, i, ps_tset->ms_sz.m_pheight);
	}

	/* Draw horizontal grid lines. */
	for (int i = 0; i < ps_tset->ms_sz.m_pheight; i += 33) {
		MoveToEx(ps_tset->ms_res.p_hbmpdc, 0, i, NULL);

		/* Draw line. */
		LineTo(ps_tset->ms_res.p_hbmpdc, ps_tset->ms_sz.m_pwidth, i);
	}

	/* Restore default pen and brush. */
	SelectObject(ps_tset->ms_res.p_hbmpdc, p_hbrold);
	SelectObject(ps_tset->ms_res.p_hbmpdc, p_hpold);

	/* Delete resources. */
	DeleteObject(p_hbr);
	DeleteObject(p_hp);
}

/*
 * Sets up the internal memory DC of tileset view.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t mbe_tset_internal_setupdc(
	struct mbe_tset *ps_tset,       /* tileset */
	struct mbe_dlgnewts_cp *ps_crps /* create parameters */
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
	ps_tset->ms_sz.m_pwidth  = ps_crps->m_width * 32 + (ps_crps->m_width + 1);
	ps_tset->ms_sz.m_pheight = ps_crps->m_height * 32 + (ps_crps->m_height + 1);

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

	/* Draw grid. */
	mbe_tset_internal_drawgrid(ps_tset);

	/* Release device context of desktop window. */
	ReleaseDC(GetDesktopWindow(), p_hdc);
	return MARBLE_EC_OK;
}

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
			.nMin   = ps_tset->s_yscr.nMin,
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
			.nMin   = ps_tset->s_xscr.nMin,
			.nMax   = ps_tset->ms_sz.m_pwidth,
			.nPage  = nwidth,
			.nPos   = min(ps_tset->s_xscr.nPos, maxscr)
		};

		isxvisible = TRUE;
		SetScrollInfo(ps_tset->p_hwnd, SB_HORZ, &ps_tset->s_xscr, TRUE);
	}

	/* Update visible states of scrollbars. */
	ShowScrollBar(ps_tset->p_hwnd, SB_VERT, isyvisible);
	ShowScrollBar(ps_tset->p_hwnd, SB_HORZ, isxvisible);
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

	switch (msg) {
		case WM_CREATE: MBE_SETUDATA(); return FALSE;
		case WM_SIZE:
			if (ps_udata->m_isinit == TRUE)
				mbe_tset_internal_updatescrollbarinfo(
					ps_udata,
					GET_X_LPARAM(lparam),
					GET_Y_LPARAM(lparam)
				);

			return TRUE;
		case WM_PAINT:
			p_hdc = BeginPaint(p_hwnd, &s_ps);

			GetClientRect(p_hwnd, &s_rect);
			BitBlt(
				p_hdc,
				s_rect.left,
				s_rect.top,
				s_rect.right,
				s_rect.bottom,
				ps_udata->ms_res.p_hbmpdc,
				0,
				0,
				SRCCOPY
			);

			EndPaint(p_hwnd, &s_ps);
			return FALSE;
	}

	return DefWindowProc(p_hwnd, msg, wparam, lparam);
}

/*
 * Creates a logical tileset.
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

	/* Insert tab item. */
	TCITEM s_item = {
		.mask = TCIF_TEXT,
		.pszText = ps_crps->maz_name
	};
	if (TabCtrl_InsertItem(ps_parent->mp_hwnd, ps_parent->m_nts++, &s_item) == -1) {
		ecode = MARBLE_EC_UNKNOWN;

		goto lbl_END;
	}

	/* Get current size of tab view. */
	RECT s_parentsize;
	mbe_tsetview_internal_getrect(ps_parent, &s_parentsize);

	/* Create window of tileset view. */
	ps_tset->p_hwnd = CreateWindowEx(
		WS_EX_WINDOWEDGE | WS_EX_COMPOSITED,
		glpz_tsviewwndclname,
		NULL,
		WS_CHILD | WS_CLIPSIBLINGS | WS_HSCROLL | WS_VSCROLL,
		s_parentsize.left,
		s_parentsize.top,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top,
		ps_parent->mp_hwnd,
		NULL,
		gls_editorapp.mp_hinst,
		(LPVOID)ps_tset
	);
	if (ps_tset->p_hwnd == NULL) {
		ecode = MARBLE_EC_CREATEWND;

		goto lbl_END;
	}

	/* Create empty bitmap. */
	ecode = mbe_tset_internal_setupdc(ps_tset, ps_crps);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* Initialize scrollbars. */
	mbe_tset_internal_updatescrollbarinfo(
		ps_tset,
		s_parentsize.right - s_parentsize.left,
		s_parentsize.bottom - s_parentsize.top
	);
	
	/* Update init state. */
	ps_tset->m_isinit = TRUE;

	UpdateWindow(ps_tset->p_hwnd);

lbl_END:
	if (ecode != MARBLE_EC_OK) {
		--ps_parent->m_nts;

		if (ps_tset->p_hwnd != NULL)
			DestroyWindow(ps_tset->p_hwnd);

		mbe_tset_destroy(ps_tset);
	}

	return ecode;
}
#pragma endregion


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
	mbe_tsetview_internal_getrect(ps_tsetview, &s_parentsize);

	for (size_t i = 0; i < ps_tsetview->m_nts; i++)
		SetWindowPos(
			ps_tsetview->mas_ts[i].p_hwnd,
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

	/*
	 * Check whether the maximum number of tilesets per view
	 * has been reached. If yes, output error message and return.
	 * If not, open the dialog.
	 */
	if (ps_tsetview->m_nts == MBE_MAXTSC) {
		MessageBox(
			NULL,
			TEXT("Could not create tileset.\nThe maximum number of tilesets per view has been reached."),
			TEXT("Error"),
			MB_ICONERROR | MB_OK
		);

		return MARBLE_EC_COMPSTATE;
	}

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
	UpdateWindow(ps_tsetview->mas_ts[index].p_hwnd);
	ShowWindow(ps_tsetview->mas_ts[index].p_hwnd, SW_SHOW);
}


