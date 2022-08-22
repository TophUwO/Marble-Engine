#include <editor.h>


struct mbeditor_application gls_editorapp = { NULL };


static void mbeditor_internal_opendebugcon(void) {
	if (AllocConsole() == TRUE) {
		FILE *p_file;
		freopen_s(&p_file, "CONOUT$", "wb", stdout);

		printf("Hello, world");
	}
}

static marble_ecode_t mbeditor_internal_loadresources(void) {
	gls_editorapp.ms_res.mp_hguifont = CreateFont(
		16,
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
		TEXT("Segoe UI")
	);

	return MARBLE_EC_OK;
}

static void mbeditor_internal_loadmenu(void) {
	
}

static LRESULT CALLBACK mbeditor_internal_wndproc(
	_In_ HWND p_hwnd,
	_In_ UINT msg,
	     WPARAM wparam,
	     LPARAM lparam
) {
	marble_ecode_t ecode;

	switch (msg) {
		case WM_CREATE:
			mbeditor_internal_loadresources();

			ecode = mbeditor_tsetview_init(p_hwnd, &gls_editorapp.ms_tsview);
			if (ecode != MARBLE_EC_OK)
				return ecode;

			return 0;
		case WM_SIZE:
			mbeditor_tsetview_resize(
				&gls_editorapp.ms_tsview,
				GET_X_LPARAM(lparam),
				GET_Y_LPARAM(lparam)
			);

			return 0;
		case WM_CLOSE:
			DestroyWindow(p_hwnd);

			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);

			return 0;
	}

	return DefWindowProc(p_hwnd, msg, wparam, lparam);
}

static marble_ecode_t mbeditor_internal_createwnd(
	HINSTANCE p_hinst
) {
	mbeditor_internal_opendebugcon();

	/* Register editor window class. */
	WNDCLASSEX s_class = {
		.cbSize        = sizeof s_class,
		.hInstance     = p_hinst,
		.lpszClassName = TEXT("marble_editor_wnd"),
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.hIcon         = LoadIcon(NULL, IDI_APPLICATION),
		.hIconSm       = LoadIcon(NULL, IDI_APPLICATION),
		.lpfnWndProc   = (WNDPROC)&mbeditor_internal_wndproc
	};
	if (RegisterClassEx(&s_class) == false)
		return MARBLE_EC_REGWNDCLASS;

	gls_editorapp.mp_hinst = p_hinst;
	gls_editorapp.mp_hwnd  = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		TEXT("marble_editor_wnd"),
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


marble_ecode_t mbeditor_init(
	_In_   HINSTANCE p_hinst,
	_In_z_ LPSTR pz_cmdline
) {
	INITCOMMONCONTROLSEX s_ctrls = {
		.dwSize = sizeof s_ctrls,
		.dwICC  = ICC_TAB_CLASSES | ICC_WIN95_CLASSES
	};
	if (InitCommonControlsEx(&s_ctrls) == false)
		return MARBLE_EC_REGWNDCLASS;

	marble_ecode_t res = mbeditor_internal_createwnd(p_hinst);
	if (res != MARBLE_EC_OK)
		return res;

	UpdateWindow(gls_editorapp.mp_hwnd);
	ShowWindow(gls_editorapp.mp_hwnd, SW_SHOWNORMAL);

	return MARBLE_EC_OK;
}

marble_ecode_t mbeditor_run(void) {
	MSG s_msg;
	while (GetMessage(&s_msg, NULL, 0, 0) > 0) {
		TranslateMessage(&s_msg);
		DispatchMessage(&s_msg);
	}

	mbeditor_tsetview_uninit(&gls_editorapp.ms_tsview);
	return MARBLE_EC_OK;
}


