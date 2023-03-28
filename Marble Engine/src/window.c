#include <application.h>


static TCHAR const *const glpz_wndclassname = TEXT("marble_window");
static DWORD const        gl_wndstyle       = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME;
static TCHAR const *const glpz_deftitle     = TEXT("Marble Engine Sandbox");
static TCHAR const *const glpz_updtitle     = TEXT("Marble Engine Sandbox - %i FPS");

/* Marble Application instance */
extern struct marble_application gls_app;

/*
 * Calculate coordinates of drawing origin.
 * 
 * The drawing area will always be aligned in
 * the center of the screen.
 * 
 * Returns nothing.
 */
static void marble_window_internal_computedrawingorigin(
	/* window to calculate the drawing origin for */
	struct marble_window const *ps_window
) {
	RECT s_clientrect;
	GetClientRect(ps_window->mp_handle, &s_clientrect);

	ps_window->mps_renderer->m_orix = s_clientrect.right  / 2.0f - ps_window->ms_data.ms_ext.ms_client.m_width  / 2.0f;
	ps_window->mps_renderer->m_oriy = s_clientrect.bottom / 2.0f - ps_window->ms_data.ms_ext.ms_client.m_height / 2.0f;

    marble_log_debug(NULL, "wndsys: Recalculated drawing origin.");
}

/*
 * Generates an event and propagates it up the layerstack, having topmost layers
 * receive the event first.
 * 
 * Returns nothing.
 */
static void marble_window_internal_onevent(
	void *p_event /* event data */
) {
	struct marble_event *ps_event = (struct marble_event *)p_event;

	/*
	* Traverse layer stack top to bottom, having topmost layers
	* get the chance to handle the event first.
	*/
    size_t const len = marble_util_vec_count(gls_app.ms_layerstack.mps_vec);

	for (size_t i = len - 1; i && i != (size_t)(-1) && ps_event->m_ishandled == false; i--) {
		struct marble_layer *ps_layer = marble_util_vec_get(gls_app.ms_layerstack.mps_vec, i);

		if (ps_layer->m_isenabled)
			(*ps_layer->ms_cbs.cb_onevent)(ps_layer->m_id, ps_event, ps_layer->mp_userdata);
	}
}

/*
 * Calculates the window dimensions of a new or already existing window, based
 * on the parameters passed as **width**, **height**, and **tsize**. If the desired
 * window dimensions exceed the window's current (or primary) monitor's physical size,
 * the window size will be chosen to maximize the client area while approximately
 * maintaining the aspect ratio implicitly defined by the quotient of **width** and
 * **height**.
 * 
 * The calculated values will be written to **p_width** and **p_height**. These values
 * are then to be passed to "CreateWindow()" or "SetWindowPos()" directly.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ static marble_ecode_t marble_window_internal_calcdimensions(
	_In_opt_ HWND p_hwnd,
	         int width,
	         int height,
	         int tsize,
	         DWORD style,
	_Out_    int *p_width,
	_Out_    int *p_height
) {
	if (p_width == NULL || p_height == NULL || width * height * tsize == 0)
		return MARBLE_EC_INTERNALPARAM;

	/*
	 * (1) Get desired (physical) monitor handle.
	 * (1.1) If **p_hwnd** is not NULL, get a handle to the
	 *       monitor the window is currently placed on.
	 * (1.2) Otherwise, get a handle to the primary monitor of
	 *       the system.
	 */
	HMONITOR p_hmon = p_hwnd != NULL
		? MonitorFromWindow(p_hwnd, MONITOR_DEFAULTTOPRIMARY)
		: MonitorFromPoint((POINT){ 0, 0 }, MONITOR_DEFAULTTOPRIMARY)
	;
	/* (2) Get monitor info. */
	MONITORINFO s_moninfo = { .cbSize = sizeof s_moninfo };
	if (GetMonitorInfo(p_hmon, &s_moninfo) == false)
		return MARBLE_EC_GETMONITORINFO;

	/* 
	 * (3) Calculate hypothetical window size based on the
	 *     given dimensions.
	 */
	RECT s_clrect = { 0, 0, width * tsize, height * tsize };
	if (AdjustWindowRect(&s_clrect, style, false) == false)
		return MARBLE_EC_CALCWNDSIZE;

	/*
	 * (4) Check whether the window would be to large to be
	 *     displayed on the primary monitor.
	 */
	int const scr_w = abs(s_moninfo.rcWork.right - s_moninfo.rcWork.left);
	int const scr_h = abs(s_moninfo.rcWork.bottom - s_moninfo.rcWork.top);
	int const wnd_w = abs(s_clrect.right - s_clrect.left);
	int const wnd_h = abs(s_clrect.bottom - s_clrect.top);
	if (scr_w < wnd_w || scr_h < wnd_h) {
		/* (5) If the requested window dimensions are too large, scale it. */
		float const scale = min(scr_w / (float)wnd_w, scr_h / (float)wnd_h);

		/* (6) Request the new window size. */
		s_clrect = (RECT) { 0, 0, (LONG)(width * scale) * tsize, (LONG)(height * scale) * tsize };
		if (AdjustWindowRect(&s_clrect, style, false) == false)
			return MARBLE_EC_CALCWNDSIZE;
	}

	/* (7) Return the width and the height of the window rectangle. */
	*p_width  = s_clrect.right - s_clrect.left;
	*p_height = s_clrect.bottom - s_clrect.top;

	return MARBLE_EC_OK;
}

/*
 * Queries current window and client size and writes it into
 * the respective destination structures.
 * 
 * Returns nothing.
 */
_Success_ok_ static marble_ecode_t marble_window_internal_querydimensions(
	_In_        HWND p_hwnd,                      /* window handle */
	_Maybe_out_ struct marble_sizei2d *ps_client, /* destination client size */
	_Maybe_out_ struct marble_sizei2d *ps_window  /* destination window size */
) {
	if (p_hwnd == NULL || ps_client == NULL || ps_window == NULL)
		return MARBLE_EC_INTERNALPARAM;

	RECT s_client, s_window;
	if (GetClientRect(p_hwnd, &s_client) == false || GetWindowRect(p_hwnd, &s_window) == false)
		return MARBLE_EC_QUERYWINDOWRECT;

	*ps_client = (struct marble_sizei2d){ s_client.right - s_client.left, s_client.bottom - s_client.top };
	*ps_window = (struct marble_sizei2d){ s_window.right - s_window.left, s_window.bottom - s_window.top };

	return MARBLE_EC_OK;
}

/*
 * Retrieves the resize message reason for a given wparam of WM_SIZE.
 * 
 * Returns a string constant verbally describing the reason.
 */
static char const *const marble_window_internal_getsizereason(
    WPARAM wparam /* wparam to stringify */
) {
    switch (wparam) {
        case SIZE_MAXIMIZED: return "maximized";
        case SIZE_MINIMIZED: return "minimized";
        case SIZE_RESTORED:  return "restored";
    }

    return "<unknown_action>";
}

/*
 * Window procedure. A windows-specific mechanism to handle
 * messages and keep the application running.
 * 
 * For more information, see
 * https://docs.microsoft.com/de-de/windows/win32/winmsg/window-procedures.
 */
static LRESULT CALLBACK marble_window_internal_windowproc(
	HWND p_window, /* window handle */
	UINT msgid,    /* message id */
	WPARAM wparam, /* param1 */
	LPARAM lparam  /* param2 */
) {
	struct marble_window *ps_wnddata = (struct marble_window *)GetWindowLongPtr(
		p_window, 
		GWLP_USERDATA
	);

	switch (msgid) {
		case WM_CREATE:
			SetWindowLongPtr(
				p_window,
				GWLP_USERDATA,
				(LONG_PTR)((CREATESTRUCT *)lparam)->lpCreateParams
			);

			return 0;
		case WM_SIZE: {
			ps_wnddata->ms_data.m_isminimized = wparam == SIZE_MINIMIZED;

			/* Ignore message if window initialization is not complete yet. */
			if (ps_wnddata != NULL && ps_wnddata->mp_handle != NULL && ps_wnddata->mps_renderer != NULL) {
				marble_renderer_resize(
					ps_wnddata->mps_renderer,
					(UINT)LOWORD(lparam),
					(UINT)HIWORD(lparam)
				);

                /* Recompute drawing area coordinates. */
                marble_window_internal_computedrawingorigin(ps_wnddata);
            }
			
            marble_log_debug(NULL, "wndsys: Window %s.", marble_window_internal_getsizereason(wparam));
			return 0;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			struct marble_keyboardevent s_keydownev;
			struct marble_keyboardevent_data s_data = {
				.m_keycode  = (DWORD)wparam,
				.m_issyskey = msgid == WM_SYSKEYDOWN
			};
			marble_event_construct(
				&s_keydownev,
				lparam >> 30 & 1
					? MARBLE_EVTYPE_KBREPEATED 
					: MARBLE_EVTYPE_KBPRESSED,
				&s_data
			);

			marble_window_internal_onevent(&s_keydownev);
			return 0;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			if (wparam == VK_F11) {
				ps_wnddata->ms_data.m_isfscreen = !ps_wnddata->ms_data.m_isfscreen;

				if (ps_wnddata->ms_data.m_isfscreen) {
					/* Make window borderless. */
					SetWindowLong(p_window, GWL_STYLE, 0);

					/* Resize window so it covers the entire screen. */
					SetWindowPos(
						p_window,
						HWND_TOP,
						0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
					);

                    /* Show the window. */
					ShowWindow(p_window, SW_SHOWMAXIMIZED);
                    return 0;
				}
					
                /* Restore window styles. */
				SetWindowLong(p_window, GWL_STYLE, ps_wnddata->ms_data.m_style);

				/* Restore old size. */
				SetWindowPos(
					p_window, 
					HWND_TOP,
					0, 0,
					ps_wnddata->ms_data.ms_ext.ms_window.m_width,
					ps_wnddata->ms_data.ms_ext.ms_window.m_height,
					SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED
				);

                /* Show the window. */
				ShowWindow(p_window, SW_SHOW);
				return 0;
			}

			struct marble_keyboardevent s_keyreleasedev;
			struct marble_keyboardevent_data s_data = {
				.m_keycode  = (DWORD)wparam,
				.m_issyskey = msgid == WM_SYSKEYUP
			};
			marble_event_construct(
				&s_keyreleasedev,
				MARBLE_EVTYPE_KBRELEASED,
				&s_data
			);

			marble_window_internal_onevent(&s_keyreleasedev);
			return 0;
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE: {
			struct marble_mouseevent s_mouseev;
			struct marble_mouseevent_data s_data = {
				.m_buttoncode = (DWORD)wparam,
				.ms_pos = { 
					.m_x = GET_X_LPARAM(lparam), 
					.m_y = GET_Y_LPARAM(lparam)
				}
			};
			marble_event_construct(
				&s_mouseev,
				marble_event_getmouseevent((uint32_t)msgid),
				&s_data
			);

			marble_window_internal_onevent(&s_mouseev);
			return 0;
		}
		case WM_CLOSE: {
			struct marble_windowevent s_wndclosedev;
			marble_event_construct(
				&s_wndclosedev,
				MARBLE_EVTYPE_WNDCLOSED,
				NULL
			);

			marble_window_internal_onevent(&s_wndclosedev);
			if (wparam == 0)
				marble_application_setstate(
					false,
					MARBLE_EC_OK,
					MARBLE_APPSTATE_SHUTDOWN
				);

			DestroyWindow(p_window);
			return 0;
		}
		case WM_DESTROY:
			/* Only post a WM_QUIT if it is an orderly shutdown. */
			if (gls_app.ms_state.m_id == MARBLE_APPSTATE_SHUTDOWN)
				PostQuitMessage(0);

			return 0;
	}

	return DefWindowProc(p_window, msgid, wparam, lparam);
}

void marble_window_destroy(
	_Uninit_(pps_wnd) struct marble_window **pps_wnd
) {
	if (pps_wnd == NULL)
		return;

	if (gls_app.ms_state.m_isfatal == true)
		SendMessage((*pps_wnd)->mp_handle, WM_CLOSE, 0, 0);

	marble_renderer_destroy(&(*pps_wnd)->mps_renderer);

	free(*pps_wnd);
	*pps_wnd = NULL;
}

_Critical_ marble_ecode_t marble_window_create(
	_In_            struct marble_app_settings const *ps_settings,
	                bool isvsync,
	_Init_(pps_wnd) struct marble_window **pps_wnd
) { MB_ERRNO
	if (pps_wnd == NULL)
		return MARBLE_EC_PARAM;

	int width, height;
	ecode = marble_window_internal_calcdimensions(
		NULL,
		ps_settings->m_width,
		ps_settings->m_height,
		ps_settings->m_tilesize,
		gl_wndstyle,
		&width,
		&height
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof **pps_wnd,
		true,
		false,
		pps_wnd
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	(*pps_wnd)->ms_data.m_isfscreen = false;
	(*pps_wnd)->ms_data.m_style     = gl_wndstyle;

	(*pps_wnd)->mp_handle = CreateWindowEx(
		0, 
		glpz_wndclassname, 
        glpz_deftitle, 
		gl_wndstyle,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		width, 
		height, 
		NULL, 
		NULL, 
		gls_app.mp_inst,
		*pps_wnd
	);
	if (*pps_wnd == NULL) {
		ecode = MARBLE_EC_CREATEWND;

		goto lbl_END;
	}

	marble_window_internal_querydimensions(
		(*pps_wnd)->mp_handle,
		&(*pps_wnd)->ms_data.ms_ext.ms_client,
		&(*pps_wnd)->ms_data.ms_ext.ms_window
	);

	/* Create renderer. */
	ecode = marble_renderer_create(
		MARBLE_RENDERAPI_DIRECT2D,
		isvsync,
		(*pps_wnd)->mp_handle,
		&(*pps_wnd)->mps_renderer
	);
    marble_util_clock_start(&(*pps_wnd)->ms_data.ms_updt);

lbl_END:
	if (ecode != MARBLE_EC_OK)
		marble_window_destroy(pps_wnd);

	return ecode;
}


void marble_window_setfullscreen(
	_In_ struct marble_window *ps_window,
	     bool isenabled
) {
	if (ps_window == NULL)
		return;

	ps_window->ms_data.m_isfscreen = isenabled;

	/*
	 * Send a fake WM_KEYDOWN with keycode F11 so that
	 * the window gets resized. This is somewhat ugly, so we
	 * may change that in the future.
	 */
	SendMessage(ps_window->mp_handle, WM_KEYDOWN, (WPARAM)VK_F11, 0);
}

void marble_window_setvsync(
	_In_ struct marble_window *ps_wnd,
	     bool isenabled
) {
	/*
	 * We just have to set this boolean variable to the
	 * value we want. The renderer will automatically
	 * use the new value every time it presents a frame.
	 */
	if (ps_wnd != NULL && ps_wnd->mps_renderer != NULL && ps_wnd->mps_renderer->m_isinit == true)
		ps_wnd->mps_renderer->m_isvsync = isenabled;
}

void marble_window_update(
	_In_ struct marble_window *ps_wnd,
	     float fFrameTime
) {
	/*
     * Set window text to show current FPS. Only update
     * it when we are in windowed mode.
     */
	if (!ps_wnd->ms_data.m_isfscreen && marble_util_clock_asmsec(&ps_wnd->ms_data.ms_updt) > 1000.0) {
		marble_util_clock_start(&ps_wnd->ms_data.ms_updt);

		TCHAR az_buffer[256] = { 0 };
		_stprintf_s(
			az_buffer,
			255,
			glpz_updtitle,
			(int)(1.0f / fFrameTime)
		);

		SetWindowText(ps_wnd->mp_handle, az_buffer);
	}
}

void marble_window_resize(
	_In_ struct marble_window *ps_wnd,
	     int width,
	     int height,
	     int tsize
) {
	if (ps_wnd == NULL)
		return;

	int nwidth, nheight;
	if (marble_window_internal_calcdimensions(
		ps_wnd->mp_handle,
		width,
		height,
		tsize,
		ps_wnd->ms_data.m_style,
		&nwidth,
		&nheight
	)) return;

	marble_window_internal_querydimensions(
		ps_wnd->mp_handle,
		&ps_wnd->ms_data.ms_ext.ms_client,
		&ps_wnd->ms_data.ms_ext.ms_window
	);

	/*
	 * Resize window and renderer according
	 * to the new dimensions.
	 */
	MoveWindow(
		ps_wnd->mp_handle,
		0, 0,
		ps_wnd->ms_data.ms_ext.ms_window.m_width,
		ps_wnd->ms_data.ms_ext.ms_window.m_height,
		true
	);
	marble_window_internal_computedrawingorigin(ps_wnd);
}


#pragma region WINDOWSYS
_Critical_ marble_ecode_t marble_windowsys_init(void) {
	/* Register window class. */
	WNDCLASSEX s_wndclassdesc = {
		.cbSize        = sizeof s_wndclassdesc,
		.hInstance     = GetModuleHandle(NULL),
		.lpszClassName = glpz_wndclassname,
		.lpfnWndProc   = (WNDPROC)&marble_window_internal_windowproc,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hIcon         = LoadIcon(NULL, IDI_APPLICATION),
		.hIconSm       = LoadIcon(NULL, IDI_APPLICATION),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1)
	};
	if (RegisterClassEx(&s_wndclassdesc) == false)
		return MARBLE_EC_REGWNDCLASS;

	return MARBLE_EC_OK;
}

void marble_windowsys_uninit(void) {
	UnregisterClass(glpz_wndclassname, GetModuleHandle(NULL));
}
#pragma endregion


