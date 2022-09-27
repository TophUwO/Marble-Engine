#include <editor.h>


struct mbe_application gls_editorapp = { NULL };


/*
 * Frees any resources that are kept by the application instance itself,
 * such as system resources.
 * 
 * Returns nothing.
 */
static void mbe_editor_int_freeresources(void) {
	/*
	 * Delete system resources such as predefined brushes, pens,
	 * bitmaps, fonts etc.
	 */
	DeleteFont(gls_editorapp.ms_res.mp_hguifont);
	DestroyMenu(gls_editorapp.ms_res.mp_hmainmenu);

	/* Release global Direct2D resources. */
	D2DWR_SAFERELEASE(D2DWr_StrokeStyle_Release, gls_editorapp.ms_res.mp_grstroke);
	D2DWR_SAFERELEASE(D2DWr_Factory_Release, gls_editorapp.ms_res.mp_d2dfac);
}

/*
 * Loads system resources. These resources may be used by other
 * windows of the editor application.
 * 
 * Returns nothing.
 */
static marble_ecode_t mbe_editor_int_loadresources(
	_In_ HWND p_hparent /* parent window handle */
) { MB_ERRNO
	if (p_hparent == NULL)
		return MARBLE_EC_INTERNALPARAM;

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
	if (gls_editorapp.ms_res.mp_hguifont == NULL) {
		ecode = MARBLE_EC_LOADRESOURCE;

		goto lbl_END;
	}

	/* Create Direct2D and WIC resources. */
#if (defined _DEBUG) || (defined MB_DEVBUILD)
	D2D1_FACTORY_OPTIONS const s_opts = {
		.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION
	}, *ps_opts = &s_opts;
#else
	D2D1_FACTORY_OPTIONS const *ps_opts = NULL;
#endif
	hres = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_MULTI_THREADED,
		&IID_ID2D1Factory,
		ps_opts,
		&gls_editorapp.ms_res.mp_d2dfac
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_CREATED2DFAC;

		goto lbl_END;
	}

	/* Grid-line stroke properties */
	D2D1_STROKE_STYLE_PROPERTIES const s_strprops = {
		.dashStyle  = D2D1_DASH_STYLE_CUSTOM,
		.dashCap    = D2D1_CAP_STYLE_SQUARE,
		.dashOffset = 0.0f
	};
	float const a_dashes[2] = { 2.0f, 4.0f };

	/* Create stroke style. */
	hres = D2DWr_Factory_CreateStrokeStyle(
		gls_editorapp.ms_res.mp_d2dfac,
		&s_strprops,
		a_dashes,
		2,
		&gls_editorapp.ms_res.mp_grstroke
	);
	if (hres != S_OK) {
		ecode = MARBLE_EC_CREATED2DSTROKESTYLE;

		goto lbl_END;
	}

lbl_END:
	if (ecode != MARBLE_EC_OK)
		mbe_editor_int_freeresources();

	return MARBLE_EC_OK;
}

/*
 * Loads menu bar of main editor window.
 * 
 * Returns nothing.
 */
static marble_ecode_t mbe_editor_int_loadmenu(HWND p_hwnd) {
	/* Load menu. */
	gls_editorapp.ms_res.mp_hmainmenu = LoadMenu(gls_editorapp.mp_hinst, MAKEINTRESOURCE(MBE_MainWnd_Menubar));
	if (gls_editorapp.ms_res.mp_hmainmenu == NULL)
		return MARBLE_EC_LOADRESOURCE;

	/* Associate menu with main window. */
	SetMenu(p_hwnd, gls_editorapp.ms_res.mp_hmainmenu);

	return MARBLE_EC_OK;
}

/*
 * Window procedure for main editor window.
 */
static LRESULT CALLBACK mbe_editor_int_wndproc(
	HWND p_hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	NMHDR *ps_nmhdr;
	int cursel;
	struct mbe_wndsize s_wndsize;
	POINT s_pt;

	switch (msg) {
		case WM_CREATE:
			/*
			 * If resource creation fails, return -1 which will cause
			 * "CreateWindow[Ex]()" to fail, destroying the window
			 * beforehand.
			 */
			if (mbe_editor_int_loadmenu(p_hwnd) + mbe_editor_int_loadresources(p_hwnd) != MARBLE_EC_OK) {
				MB_LOG_FATAL("Could not load one or more critical application resources.", 0);

				return -1;
			}
			
			return 0;
		case WM_SIZE:
			/* Resize tileset view. */
			if (gls_editorapp.mps_tsview == NULL)
				return 0;

			mbe_base_getwindowpos(gls_editorapp.mps_tsview->mp_hwndtab, &s_pt);

			s_wndsize = (struct mbe_wndsize){
				.m_xpos   = 0,
				.m_ypos   = 0,
				.m_width  = 200,
				.m_height = GET_Y_LPARAM(lparam)
			};
			mbe_tabview_resize(gls_editorapp.mps_tsview, &s_wndsize);

			/* Resize level view. */
			if (gls_editorapp.mps_lvlview == NULL)
				return 0;

			mbe_base_getwindowpos(gls_editorapp.mps_lvlview->mp_hwndtab, &s_pt);

			s_wndsize = (struct mbe_wndsize){
				.m_xpos   = s_pt.x,
				.m_ypos   = 0,
				.m_width  = GET_X_LPARAM(lparam) - s_pt.x,
				.m_height = GET_Y_LPARAM(lparam)
			};
			mbe_tabview_resize(gls_editorapp.mps_lvlview, &s_wndsize);

			return 0;
		case WM_COMMAND:
			switch (wparam) {
				case MBE_FileNew_Level:
					mbe_levelview_newlvlbydlg(gls_editorapp.mps_lvlview);

					break;
				case MBE_Menubar_File_Import_BmpTS:
					mbe_tsetview_imptsfromimg(gls_editorapp.mps_tsview);

					break;
				case MBE_MainMenu_File_Close:
					PostMessage(p_hwnd, WM_CLOSE, 0, 0);

					break;
			}

			return 0;
		case WM_NOTIFY:
			ps_nmhdr = (NMHDR *)lparam;

			switch (ps_nmhdr->code) {
				case TCN_SELCHANGE:
					cursel = TabCtrl_GetCurSel(ps_nmhdr->hwndFrom);
					
					switch (ps_nmhdr->idFrom) {
						case MBE_COMPID_LVLVIEW:
							mbe_tabview_changepage(gls_editorapp.mps_lvlview, cursel);

							break;
					}

					break;
			}

			return 0;
		case WM_CLOSE:
			gls_editorapp.ms_flags.mf_isdest = TRUE;

			DestroyWindow(p_hwnd);
			return 0;
		case WM_DESTROY:
			mbe_editor_int_freeresources();

			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(p_hwnd, msg, wparam, lparam);
}

static marble_ecode_t mbe_editor_int_createmainwnd(HINSTANCE p_hinst) {
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
		.lpfnWndProc   = (WNDPROC)&mbe_editor_int_wndproc
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


marble_ecode_t mbe_editor_init(
	HINSTANCE p_hinst,
	LPSTR pz_cmdline
) { MB_ERRNO
	extern BOOL MB_CALLBACK mbe_levelview_int_ontviewcreate(_Inout_ struct mbe_tabview *, _In_opt_ void *);
	extern BOOL MB_CALLBACK mbe_levelview_int_ontviewresize(_Inout_ struct mbe_tabview *, int, int);
	extern BOOL MB_CALLBACK mbe_levelview_int_ontviewdestroy(_Inout_ struct mbe_tabview *);

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
	if (InitCommonControlsEx(&s_ctrls) == FALSE)
		return MARBLE_EC_REGWNDCLASS;

	mbe_tabview_inl_regwndclass();

	/* Create main window. */
	marble_ecode_t res = mbe_editor_int_createmainwnd(p_hinst);
	if (res != MARBLE_EC_OK)
		return res;

	/* Get dimensions of main window client area. */
	RECT s_viewrect;
	GetClientRect(gls_editorapp.mp_hwnd, &s_viewrect);

	/* Initialize tileset view. */
	struct mbe_wndsize s_size = {
		.m_xpos   = 0,
		.m_ypos   = 0,
		.m_width  = 200,
		.m_height = s_viewrect.bottom
	};
	struct mbe_tabview_callbacks s_cbs = {
		.mpfn_oncreate  = NULL,
		.mpfn_ondestroy = NULL
	};
	res = mbe_tabview_create(
		gls_editorapp.mp_hwnd,
		&s_size,
		&s_cbs,
		NULL,
		MBE_COMPID_TSETVIEW,
		&gls_editorapp.mps_tsview
	);
	if (res != MARBLE_EC_OK)
		return res;

	/* Initialize level view. */
	s_size = (struct mbe_wndsize){
		.m_xpos   = 200,
		.m_ypos   = 0,
		.m_width  = s_viewrect.right - 200,
		.m_height = s_viewrect.bottom
	};
	s_cbs = (struct mbe_tabview_callbacks){
		.mpfn_oncreate  = &mbe_levelview_int_ontviewcreate,
		.mpfn_onresize  = &mbe_levelview_int_ontviewresize,
		.mpfn_ondestroy = &mbe_levelview_int_ontviewdestroy
	};
	res = mbe_tabview_create(
		gls_editorapp.mp_hwnd,
		&s_size, &s_cbs,
		NULL,
		MBE_COMPID_LVLVIEW,
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
	mbe_tabview_destroy(&gls_editorapp.mps_tsview);
	mbe_tabview_destroy(&gls_editorapp.mps_lvlview);
	marble_log_uninit();

#if (defined _DEBUG)
	_CrtDumpMemoryLeaks();
#endif
	return MARBLE_EC_OK;
}


