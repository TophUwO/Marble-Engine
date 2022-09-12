#include <editor.h>


struct mbe_application gls_editorapp = { NULL };


/*
 * Loads system resources. These resources may be used by other
 * windows of the editor application.
 * 
 * Returns nothing.
 */
static marble_ecode_t mbe_editor_internal_loadresources(void) { MB_ERRNO
	HRESULT hres;

	gls_editorapp.ms_res.mp_hguifont = CreateFont(
		14,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		FALSE,
		0,
		0,
		0,
		CLEARTYPE_QUALITY,
		FF_DONTCARE,
		TEXT("MS Shell Dlg")
	);

	gls_editorapp.ms_res.mp_hbrwhite = CreateSolidBrush(RGB(255, 255, 255));
	gls_editorapp.ms_res.mp_hbrblack = CreateSolidBrush(RGB(0, 0, 0));
	gls_editorapp.ms_res.mp_hpsel    = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	gls_editorapp.ms_res.mp_hpgrid   = CreatePen(PS_DOT, 1, RGB(0, 0, 0));

	/* Create Direct2D and WIC resources. */
#if (defined _DEBUG) || (defined MB_DEVBUILD)
	D2D1_FACTORY_OPTIONS s_opts = { .debugLevel = D2D1_DEBUG_LEVEL_INFORMATION }, *ps_opts = &s_opts;
#else
	D2D1_FACTORY_OPTIONS *ps_opts = NULL;
#endif
	hres = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_MULTI_THREADED,
		&IID_ID2D1Factory,
		ps_opts,
		&gls_editorapp.ms_res.mps_d2dfac
	);
	if (hres != S_OK)
		ecode = MARBLE_EC_CREATED2DFAC;

	/* Grid-line stroke properties */
	D2D1_STROKE_STYLE_PROPERTIES const s_strprops = {
		.dashStyle  = D2D1_DASH_STYLE_DASH,
		.dashCap    = D2D1_CAP_STYLE_SQUARE,
		.dashOffset = 0.0f
	};

	/* Create stroke style. */
	hres = D2DWr_Factory_CreateStrokeStyle(
		gls_editorapp.ms_res.mps_d2dfac,
		&s_strprops,
		NULL,
		0,
		&gls_editorapp.ms_res.mp_grstroke
	);
	if (hres != S_OK)
		ecode = MARBLE_EC_CREATED2DSTROKESTYLE;

	// TODO: BUG: currently, if any of these resources fail to create, the already created resources are not released.
	return ecode;
}

/*
 * Loads menu bar of main editor window.
 * 
 * Returns nothing.
 */
static void mbe_editor_internal_loadmenu(HWND p_hwnd) {
	/* Load menu. */
	HMENU p_hmenubar = LoadMenu(gls_editorapp.mp_hinst, MAKEINTRESOURCE(MBE_MainWnd_Menubar));
	if (p_hmenubar == NULL)
		return;

	/* Associate menu with main window. */
	SetMenu(p_hwnd, p_hmenubar);
}

/*
 * Window procedure for main editor window.
 */
static LRESULT CALLBACK mbe_editor_internal_wndproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	NMHDR *ps_nmhdr;
	int cursel;
	struct mbe_tsetview *ps_tsview;
	struct mbe_wndsize s_wndsize;
	POINT s_pt;

	switch (msg) {
		case WM_CREATE:
			mbe_editor_internal_loadmenu(p_hwnd);
			mbe_editor_internal_loadresources();
			
			return FALSE;
		case WM_SIZE:
			/* Resize tileset view. */
			mbe_tsetview_resize(
				&gls_editorapp.ms_tsview,
				GET_X_LPARAM(lparam),
				GET_Y_LPARAM(lparam)
			);

			/* Resize level view. */
			if (gls_editorapp.mps_lvlview == NULL)
				return FALSE;

			mbe_base_getwindowpos(gls_editorapp.mps_lvlview->mp_hwndtab, &s_pt);

			s_wndsize = (struct mbe_wndsize){
				.m_xpos   = s_pt.x,
				.m_ypos   = 0,
				.m_width  = GET_X_LPARAM(lparam) - s_pt.x,
				.m_height = GET_Y_LPARAM(lparam)
			};
			mbe_tabview_resize(gls_editorapp.mps_lvlview, &s_wndsize);

			return FALSE;
		case WM_COMMAND:
			switch (wparam) {
				case MBE_FileNew_Tileset:
					mbe_tsetview_newtsdlg(&gls_editorapp.ms_tsview);

					break;
				case MBE_Menubar_File_Import_BmpTS:
					mbe_tsetview_bmptsdlg(&gls_editorapp.ms_tsview);

					break;
				case MBE_FileNew_Level:
					mbe_levelview_newlvlbydlg(gls_editorapp.mps_lvlview);

					break;
				case MBE_MainMenu_File_Close:
					PostMessage(p_hwnd, WM_CLOSE, 0, 0);

					break;
			}

			return FALSE;
		case WM_NOTIFY:
			ps_nmhdr = (NMHDR *)lparam;

			switch (ps_nmhdr->code) {
				case TCN_SELCHANGE:
					cursel    = TabCtrl_GetCurSel(ps_nmhdr->hwndFrom);
					ps_tsview = (struct mbe_tsetview *)GetWindowLongPtr(ps_nmhdr->hwndFrom, GWLP_USERDATA);

					mbe_tsetview_setpage(ps_tsview, cursel);
					break;
			}

			return FALSE;
		case WM_CLOSE:
			gls_editorapp.ms_flags.mf_isdest = TRUE;

			DestroyWindow(p_hwnd);
			return FALSE;
		case WM_DESTROY: PostQuitMessage(0); return FALSE;
	}

	return DefWindowProc(p_hwnd, msg, wparam, lparam);
}

static marble_ecode_t mbe_editor_internal_createmainwnd(HINSTANCE p_hinst) {
	static TCHAR const *const glpz_wndclassname = TEXT("mbe_mainwnd");

	/* Register editor window class. */
	WNDCLASSEX s_class = {
		.cbSize        = sizeof s_class,
		.hInstance     = p_hinst,
		.lpszClassName = glpz_wndclassname,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.hIcon         = LoadIcon(NULL, IDI_APPLICATION),
		.hIconSm       = LoadIcon(NULL, IDI_APPLICATION),
		.lpfnWndProc   = (WNDPROC)&mbe_editor_internal_wndproc
	};
	if (RegisterClassEx(&s_class) == false)
		return MARBLE_EC_REGWNDCLASS;

	/* Create main window. */
	gls_editorapp.mp_hinst = p_hinst;
	gls_editorapp.mp_hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		glpz_wndclassname,
		TEXT("Marble Editor"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1200,
		700,
		NULL,
		NULL,
		p_hinst,
		NULL
	);
	if (gls_editorapp.mp_hwnd == NULL)
		return MARBLE_EC_CREATEWND;

	return MARBLE_EC_OK;
}

/*
 * Frees any resources that are kept by the application instance itself,
 * such as system resources.
 * 
 * Returns nothing.
 */
static void mbe_editor_internal_freeresources(void) {
	/*
	 * Delete system resources such as predefined brushes, pens,
	 * bitmaps, fonts etc.
	 */
	DeleteObject(gls_editorapp.ms_res.mp_hbrwhite);
	DeleteObject(gls_editorapp.ms_res.mp_hbrblack);
	DeleteObject(gls_editorapp.ms_res.mp_hpsel);
	DeleteObject(gls_editorapp.ms_res.mp_hpgrid);
	DeleteObject(gls_editorapp.ms_res.mp_hguifont);

	D2DWr_Factory_Release(gls_editorapp.ms_res.mps_d2dfac);
}


marble_ecode_t mbe_editor_init(
	HINSTANCE p_hinst,
	LPSTR pz_cmdline
) { MB_ERRNO
	static BOOL MB_CALLBACK mbe_levelview_int_ontviewcreate(_Inout_ struct mbe_tabview *, _In_opt_ void *);
	static BOOL MB_CALLBACK mbe_levelview_int_ontviewdestroy(_Inout_ struct mbe_tabview *);

	/* Init HPC. */
	marble_util_clock_init();

	ecode = marble_log_init("editorlog.txt");
	if (ecode != MARBLE_EC_OK)
		return ecode;

	/* Initialize common controls. */
	INITCOMMONCONTROLSEX s_ctrls = {
		.dwSize = sizeof s_ctrls,
		.dwICC  = ICC_TAB_CLASSES | ICC_WIN95_CLASSES
	};
	if (InitCommonControlsEx(&s_ctrls) == false)
		return MARBLE_EC_REGWNDCLASS;

	mbe_tabview_inl_regwndclass();

	/* Create main window. */
	marble_ecode_t res = mbe_editor_internal_createmainwnd(p_hinst);
	if (res != MARBLE_EC_OK)
		return res;

	/* Initialize tileset view container. */
	res = mbe_tsetview_init(gls_editorapp.mp_hwnd, &gls_editorapp.ms_tsview);
	if (res != MARBLE_EC_OK)
		return res;

	/* Initialize level view. */
	RECT s_viewrect;
	GetClientRect(gls_editorapp.mp_hwnd, &s_viewrect);

	struct mbe_wndsize const s_size = {
		.m_xpos   = 200,
		.m_ypos   = 0,
		.m_width  = s_viewrect.right - 200,
		.m_height = s_viewrect.bottom
	};
	struct mbe_tabview_callbacks const s_cbs = {
		.mpfn_oncreate  = &mbe_levelview_int_ontviewcreate,
		.mpfn_ondestroy = &mbe_levelview_int_ontviewdestroy
	};
	res = mbe_tabview_create(
		gls_editorapp.mp_hwnd,
		&s_size, &s_cbs,
		NULL,
		&gls_editorapp.mps_lvlview
	);
	if (res != MARBLE_EC_OK)
		return res;

	/* Show main window. */
	UpdateWindow(gls_editorapp.mp_hwnd);
	ShowWindow(gls_editorapp.mp_hwnd, SW_SHOWNORMAL);

	return MARBLE_EC_OK;
}

marble_ecode_t mbe_editor_run(void) {
	MB_LOG_PLAIN("-------------------------------------------------------------------------------", 0);

	MSG s_msg;
	while (GetMessage(&s_msg, NULL, 0, 0) > 0) {
		TranslateMessage(&s_msg);

		/* Track state of modifier keys. */
		switch (s_msg.message) {
			case WM_KEYDOWN:
				switch (s_msg.wParam) {
					case VK_SHIFT:
						gls_editorapp.ms_flags.mf_isshift = TRUE;
						
						break;
				}

				break;
			case WM_KEYUP:
				switch (s_msg.wParam) {
					case VK_SHIFT:
						gls_editorapp.ms_flags.mf_isshift = FALSE;
						
						break;
				}

				break;
		}

		/* Pass the message to the correct window procedure. */
		DispatchMessage(&s_msg);
	}

	MB_LOG_PLAIN("-------------------------------------------------------------------------------", 0);
	marble_log_info(NULL, "Application shutdown ...");

	/* Free resources and exit. */
	mbe_tsetview_uninit(&gls_editorapp.ms_tsview);
	mbe_tabview_destroy(&gls_editorapp.mps_lvlview);
	marble_log_uninit();

	mbe_editor_internal_freeresources();
#if (defined _DEBUG)
	_CrtDumpMemoryLeaks();
#endif
	return MARBLE_EC_OK;
}


