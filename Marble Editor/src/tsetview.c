#pragma once

#include <editor.h>
#include <tsetview.h>


static const TCHAR *const glapz_n0thru255[] = {
	TEXT("0"),
	TEXT("1"),   TEXT("2"),   TEXT("3"),   TEXT("4"),   TEXT("5"),   TEXT("6"),   TEXT("7"),   TEXT("8"), 
	TEXT("9"),   TEXT("10"),  TEXT("11"),  TEXT("12"),  TEXT("13"),  TEXT("14"),  TEXT("15"),  TEXT("16"), 
	TEXT("17"),  TEXT("18"),  TEXT("19"),  TEXT("20"),  TEXT("21"),  TEXT("22"),  TEXT("23"),  TEXT("24"), 
	TEXT("25"),  TEXT("26"),  TEXT("27"),  TEXT("28"),  TEXT("29"),  TEXT("30"),  TEXT("31"),  TEXT("32"), 
	TEXT("33"),  TEXT("34"),  TEXT("35"),  TEXT("36"),  TEXT("37"),  TEXT("38"),  TEXT("39"),  TEXT("40"), 
	TEXT("41"),  TEXT("42"),  TEXT("43"),  TEXT("44"),  TEXT("45"),  TEXT("46"),  TEXT("47"),  TEXT("48"), 
	TEXT("49"),  TEXT("50"),  TEXT("51"),  TEXT("52"),  TEXT("53"),  TEXT("54"),  TEXT("55"),  TEXT("56"), 
	TEXT("57"),  TEXT("58"),  TEXT("59"),  TEXT("60"),  TEXT("61"),  TEXT("62"),  TEXT("63"),  TEXT("64"), 
	TEXT("65"),  TEXT("66"),  TEXT("67"),  TEXT("68"),  TEXT("69"),  TEXT("70"),  TEXT("71"),  TEXT("72"), 
	TEXT("73"),  TEXT("74"),  TEXT("75"),  TEXT("76"),  TEXT("77"),  TEXT("78"),  TEXT("79"),  TEXT("80"), 
	TEXT("81"),  TEXT("82"),  TEXT("83"),  TEXT("84"),  TEXT("85"),  TEXT("86"),  TEXT("87"),  TEXT("88"), 
	TEXT("89"),  TEXT("90"),  TEXT("91"),  TEXT("92"),  TEXT("93"),  TEXT("94"),  TEXT("95"),  TEXT("96"), 
	TEXT("97"),  TEXT("98"),  TEXT("99"),  TEXT("100"), TEXT("101"), TEXT("102"), TEXT("103"), TEXT("104"), 
	TEXT("105"), TEXT("106"), TEXT("107"), TEXT("108"), TEXT("109"), TEXT("110"), TEXT("111"), TEXT("112"), 
	TEXT("113"), TEXT("114"), TEXT("115"), TEXT("116"), TEXT("117"), TEXT("118"), TEXT("119"), TEXT("120"), 
	TEXT("121"), TEXT("122"), TEXT("123"), TEXT("124"), TEXT("125"), TEXT("126"), TEXT("127"), TEXT("128"), 
	TEXT("129"), TEXT("130"), TEXT("131"), TEXT("132"), TEXT("133"), TEXT("134"), TEXT("135"), TEXT("136"), 
	TEXT("137"), TEXT("138"), TEXT("139"), TEXT("140"), TEXT("141"), TEXT("142"), TEXT("143"), TEXT("144"), 
	TEXT("145"), TEXT("146"), TEXT("147"), TEXT("148"), TEXT("149"), TEXT("150"), TEXT("151"), TEXT("152"), 
	TEXT("153"), TEXT("154"), TEXT("155"), TEXT("156"), TEXT("157"), TEXT("158"), TEXT("159"), TEXT("160"), 
	TEXT("161"), TEXT("162"), TEXT("163"), TEXT("164"), TEXT("165"), TEXT("166"), TEXT("167"), TEXT("168"), 
	TEXT("169"), TEXT("170"), TEXT("171"), TEXT("172"), TEXT("173"), TEXT("174"), TEXT("175"), TEXT("176"), 
	TEXT("177"), TEXT("178"), TEXT("179"), TEXT("180"), TEXT("181"), TEXT("182"), TEXT("183"), TEXT("184"), 
	TEXT("185"), TEXT("186"), TEXT("187"), TEXT("188"), TEXT("189"), TEXT("190"), TEXT("191"), TEXT("192"), 
	TEXT("193"), TEXT("194"), TEXT("195"), TEXT("196"), TEXT("197"), TEXT("198"), TEXT("199"), TEXT("200"), 
	TEXT("201"), TEXT("202"), TEXT("203"), TEXT("204"), TEXT("205"), TEXT("206"), TEXT("207"), TEXT("208"), 
	TEXT("209"), TEXT("210"), TEXT("211"), TEXT("212"), TEXT("213"), TEXT("214"), TEXT("215"), TEXT("216"), 
	TEXT("217"), TEXT("218"), TEXT("219"), TEXT("220"), TEXT("221"), TEXT("222"), TEXT("223"), TEXT("224"), 
	TEXT("225"), TEXT("226"), TEXT("227"), TEXT("228"), TEXT("229"), TEXT("230"), TEXT("231"), TEXT("232"), 
	TEXT("233"), TEXT("234"), TEXT("235"), TEXT("236"), TEXT("237"), TEXT("238"), TEXT("239"), TEXT("240"), 
	TEXT("241"), TEXT("242"), TEXT("243"), TEXT("244"), TEXT("245"), TEXT("246"), TEXT("247"), TEXT("248"), 
	TEXT("249"), TEXT("250"), TEXT("251"), TEXT("252"), TEXT("253"), TEXT("254"), TEXT("255")
};

#define MBE_RSTR(rgb) (rgb == (COLORREF)(-1) ? TEXT("") : glapz_n0thru255[GetRValue(rgb)])
#define MBE_GSTR(rgb) (rgb == (COLORREF)(-1) ? TEXT("") : glapz_n0thru255[GetGValue(rgb)])
#define MBE_BSTR(rgb) (rgb == (COLORREF)(-1) ? TEXT("") : glapz_n0thru255[GetBValue(rgb)])


/*
 * Structure used for "Create new tileset ..." dialog.
 * Logical tileset will be created based on these
 * parameters.
 */
struct mbeditor_tset_createparams {
	TCHAR maz_name[MBE_MAXTSNAME]; /* tileset name */
	TCHAR maz_cmt[MBE_MAXCMT];     /* comment/description */

	int      m_width;   /* width, in tiles */
	int      m_height;  /* height, in tiles */
	int      m_tsize;   /* tile size, in pixels */
	COLORREF m_col;     /* background color */
	BOOL     m_istrans; /* transparent background? */
};


/*
 * Sets-up edit controls to only accept a maximum number
 * of characters.
 * 
 * Returns nothing.
 */
static void mbeditor_tsetview_emptytsdlg_seteditlimits(
	HWND p_hwnd
) {
	/*
	 * Table representing maximum character counts
	 * for all text fields of the dialog.
	 */
	static struct { int m_id; size_t m_len; } const as_nlimits[] = {
		{ EmptyTSDlg_EDIT_Name,    MBE_MAXTSNAME - 1 },
		{ EmptyTSDlg_EDIT_Comment, MBE_MAXCMT - 1    },
		{ EmptyTSDlg_EDIT_Width,   MBE_MAXNSTR - 1   },
		{ EmptyTSDlg_EDIT_Height,  MBE_MAXNSTR - 1   },
		{ EmptyTSDlg_EDIT_TSize,   MBE_MAXNSTR - 1   },
		{ EmptyTSDlg_EDIT_Red,     3                 },
		{ EmptyTSDlg_EDIT_Green,   3                 },
		{ EmptyTSDlg_EDIT_Blue,    3                 }
	};
	size_t const nlimits = ARRAYSIZE(as_nlimits);

	/* Set the limits. */
	for (size_t i = 0; i < nlimits; i++)
		SendMessage(
			GetDlgItem(p_hwnd, as_nlimits[i].m_id),
			EM_SETLIMITTEXT,
			(WPARAM)as_nlimits[i].m_len,
			0
		);
}

/*
 * Sets dialog fields to specific values.
 * 
 * Returns nothing.
 */
static void mbeditor_tsetview_emptytsdlg_setfields(
	HWND p_hwnd,
	struct mbeditor_tset_createparams const *ps_cp
) {
	/*
	 * Dynamically format width, height, and tile size, according
	 * to the given values. Note that values will not be set
	 * if they are passed as -1.
	 * The same rule applies to the string fields inside
	 * **as_strreps**.
	 */
	TCHAR apz_dbufs[3][MBE_MAXNSTR] = { 0 };
	if (ps_cp->m_width != -1)  _stprintf_s(apz_dbufs[0], MBE_MAXNSTR, TEXT("%i"), ps_cp->m_width);
	if (ps_cp->m_height != -1) _stprintf_s(apz_dbufs[1], MBE_MAXNSTR, TEXT("%i"), ps_cp->m_height);
	if (ps_cp->m_tsize != -1)  _stprintf_s(apz_dbufs[2], MBE_MAXNSTR, TEXT("%i"), ps_cp->m_tsize);

	/*
	 * Table representing string fields.
	 * Will be used to fill all dialog edit controls
	 * with their respective values.
	 */
	struct { int m_id; TCHAR const *pz_str; } as_strreps[] = {
		{ EmptyTSDlg_EDIT_Name,    ps_cp->maz_name        },
		{ EmptyTSDlg_EDIT_Comment, ps_cp->maz_cmt         },
		{ EmptyTSDlg_EDIT_Width,   apz_dbufs[0]           },
		{ EmptyTSDlg_EDIT_Height,  apz_dbufs[1]           },
		{ EmptyTSDlg_EDIT_TSize,   apz_dbufs[2]           },
		{ EmptyTSDlg_EDIT_Red,     MBE_RSTR(ps_cp->m_col) },
		{ EmptyTSDlg_EDIT_Green,   MBE_GSTR(ps_cp->m_col) },
		{ EmptyTSDlg_EDIT_Blue,    MBE_BSTR(ps_cp->m_col) }
	};
	size_t const nreps = ARRAYSIZE(as_strreps);

	/*
	 * Fill string fields (edit controls). 
	 */
	for (size_t i = 0; i < nreps; i++)
		SetWindowText(
			GetDlgItem(p_hwnd, as_strreps[i].m_id),
			as_strreps[i].pz_str
		);

	/* Set checkbox. */
	CheckDlgButton(p_hwnd, EmptyTSDlg_CB_Transparent, (UINT)ps_cp->m_istrans);
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
	struct mbeditor_tset_createparams *ps_createparams
) {
	static COLORREF a_custcol[16] = { 0 };

	CHOOSECOLOR s_ccol = {
		.lStructSize  = sizeof s_ccol,
		.hwndOwner    = NULL,
		.lpCustColors = (LPDWORD)a_custcol,
		.rgbResult    = RGB(255, 255, 255),
		.Flags        = CC_FULLOPEN | CC_RGBINIT
	};
	
	/* Run system color dialog. */
	if (ChooseColor(&s_ccol) != FALSE) {
		ps_createparams->m_col = s_ccol.rgbResult;

		return TRUE;
	}

	return FALSE;
}

static BOOL CALLBACK mbeditor_tsetview_ontscreate_dlgproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	MBE_WNDUSERDATA(ps_param, struct mbeditor_tset_createparams *);

	/* Default settings for this dialog. */
	static struct mbeditor_tset_createparams const gls_defcps = {
		.maz_name  = TEXT("unnamed_tileset"),
		.maz_cmt   = TEXT(""),
		
		.m_col     = (COLORREF)(-1),
		.m_istrans = FALSE,
		.m_width   = -1,
		.m_height  = -1,
		.m_tsize   = -1
	};

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)lparam);

			mbeditor_tsetview_emptytsdlg_seteditlimits(p_hwnd);
			mbeditor_tsetview_emptytsdlg_setfields(p_hwnd, &gls_defcps);
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
				case EmptyTSDlg_BTN_ChooseCol:
					if (mbeditor_tsetview_emptytsdlg_handlecolordlg(ps_param) == TRUE) {
						struct { TCHAR const *mpz_red, *mpz_green, *mpz_blue; } const s_compstrs = {
							.mpz_red   = MBE_RSTR(ps_param->m_col),
							.mpz_green = MBE_GSTR(ps_param->m_col),
							.mpz_blue  = MBE_BSTR(ps_param->m_col)
						};

						SetWindowText(GetDlgItem(p_hwnd, EmptyTSDlg_EDIT_Red), s_compstrs.mpz_red);
						SetWindowText(GetDlgItem(p_hwnd, EmptyTSDlg_EDIT_Green), s_compstrs.mpz_green);
						SetWindowText(GetDlgItem(p_hwnd, EmptyTSDlg_EDIT_Blue), s_compstrs.mpz_blue);
					}

					break;
				case EmptyTSDlg_BTN_Reset:
					mbeditor_tsetview_emptytsdlg_setfields(p_hwnd, &gls_defcps);

					break;
				case EmptyTSDlg_BTN_Ok:
				case EmptyTSDlg_BTN_Cancel:
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


marble_ecode_t mbeditor_tsetview_init(
	HWND p_hwndparent,
	struct mbeditor_tsetview *ps_tsetview
) {
	if (ps_tsetview == NULL)
		return MARBLE_EC_PARAM;

	/* Create tab view window. */
	ps_tsetview->mp_hwnd = CreateWindowEx(
		WS_EX_WINDOWEDGE | WS_EX_COMPOSITED,
		WC_TABCONTROL,
		TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		0,
		0,
		150,
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
	SetWindowPos(
		ps_tsetview->mp_hwnd,
		NULL,
		0,
		0,
		150,
		nheight,
		SWP_NOACTIVATE | SWP_NOMOVE
	);
}

marble_ecode_t mbeditor_tsetview_newtsdlg(
	struct mbeditor_tsetview *ps_tsetview
) {
	if (ps_tsetview == NULL)
		return MARBLE_EC_PARAM;

	struct mbeditor_tset_createparams s_crparams;
	if (DialogBoxParam(
		gls_editorapp.mp_hinst,
		MAKEINTRESOURCE(MBE_DLG_EmptyTS),
		gls_editorapp.mp_hwnd,
		(DLGPROC)&mbeditor_tsetview_ontscreate_dlgproc,
		(LPARAM)&s_crparams
	) != FALSE) {
		/* TODO: Create tileset */

	}

	return MARBLE_EC_OK;
}


