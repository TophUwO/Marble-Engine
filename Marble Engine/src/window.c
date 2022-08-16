#include <application.h>


static TCHAR const *const glpz_wndclassname = TEXT("marble_window");


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
	for (size_t i = gls_app.ms_layerstack.mps_vec->m_size - 1; i && i != (size_t)(-1) && !ps_event->m_ishandled; i--) {
		struct marble_layer *ps_layer = marble_util_vec_get(gls_app.ms_layerstack.mps_vec, i);

		if (ps_layer->m_isenabled)
			(*ps_layer->ms_cbs.cb_onevent)(ps_layer->m_id, ps_event, ps_layer->mp_userdata);
	}
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
			//ps_wnddata = (struct marble_window *)GetWindowLongPtr(p_window, GWLP_USERDATA);
			ps_wnddata->ms_data.m_isminimized = wparam == SIZE_MINIMIZED;

			/* Ignore message if window initialization is not complete yet. */
			if (ps_wnddata && ps_wnddata->mp_handle && ps_wnddata->mps_renderer)
				marble_renderer_resize(
					ps_wnddata->mps_renderer,
					(UINT)LOWORD(lparam),
					(UINT)HIWORD(lparam)
				);
			
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
				if (ps_wnddata->ms_data.m_isfscreen == true) {
					/* Make window borderless. */
					SetWindowLong(p_window, GWL_STYLE, 0);

					/* Resize window so it covers the entire screen. */
					SetWindowPos(
						p_window,
						HWND_TOP,
						0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
					);

					ShowWindow(p_window, SW_SHOWMAXIMIZED);
				} else {
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

					ShowWindow(p_window, SW_SHOW);
				}

				/* Recompute drawing area coordinates. */
				marble_window_internal_computedrawingorigin(ps_wnddata);
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
			/*
			 * Only post a WM_QUIT if the window that is being destroyed
			 * is the main window of the application and the state is not
			 * a forced shutdown, in which case we will use a custom return
			 * value and do not need WM_QUIT to make us leave the main loop.
			 */
			if (ps_wnddata->ms_data.m_ismainwnd == true && gls_app.ms_state.m_id == MARBLE_APPSTATE_SHUTDOWN)
				PostQuitMessage(0);

			return 0;
	}

	return DefWindowProc(p_window, msgid, wparam, lparam);
}


_Critical_ marble_ecode_t marble_window_create(
	_In_z_          char const *const pz_title,
	_In_            uint32_t width,
	_In_            uint32_t height,
	                bool isvsync,
	                bool ismainwnd,
	_Init_(pps_wnd) struct marble_window **pps_wnd
) { MB_ERRNO
	if (pps_wnd == NULL)
		return MARBLE_EC_PARAM;

	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof **pps_wnd,
		true,
		false,
		pps_wnd
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_ERROR;

	(*pps_wnd)->ms_data.ms_ext.ms_window = (struct marble_sizei2d){ width, height };
	(*pps_wnd)->ms_data.m_isvsync            = isvsync;
	(*pps_wnd)->ms_data.m_isfscreen          = false;
	(*pps_wnd)->ms_data.m_style              = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME;

	WNDCLASSEX s_wndclassdesc = {
		.cbSize        = sizeof s_wndclassdesc,
		.hInstance     = gls_app.mp_inst,
		.lpszClassName = glpz_wndclassname,
		.lpfnWndProc   = (WNDPROC)&marble_window_internal_windowproc,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hIcon         = LoadIcon(NULL, IDI_APPLICATION),
		.hIconSm       = LoadIcon(NULL, IDI_APPLICATION),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1)
	};
	if (RegisterClassEx(&s_wndclassdesc) == false) {
		ecode = MARBLE_EC_REGWNDCLASS;

		goto lbl_ERROR;
	}

	(*pps_wnd)->mp_handle = CreateWindowEx(
		0, 
		glpz_wndclassname, 
		TEXT("(placeholder window title)"), 
		(*pps_wnd)->ms_data.m_style,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		(int)width, 
		(int)height, 
		NULL, 
		NULL, 
		gls_app.mp_inst,
		*pps_wnd
	);
	if (*pps_wnd == NULL) {
		// TODO: add support for creating non-main windows
		ecode = MARBLE_EC_CREATEMAINWND;

		goto lbl_ERROR;
	}

	RECT s_clientrect;
	GetClientRect((*pps_wnd)->mp_handle, &s_clientrect);

	(*pps_wnd)->ms_data.ms_ext.ms_client = (struct marble_sizei2d){
		(uint32_t)s_clientrect.right,
		(uint32_t)s_clientrect.bottom
	};

	(*pps_wnd)->ms_data.m_ismainwnd = gls_app.m_hasmainwnd == true
		? false
		: ismainwnd
	;
	return MARBLE_EC_OK;

lbl_ERROR:
	UnregisterClass(glpz_wndclassname, gls_app.mp_inst);
	marble_window_destroy(pps_wnd);

	return ecode;
}

void marble_window_destroy(
	_Uninit_(pps_wnd) struct marble_window **pps_wnd
) {
	if (pps_wnd == NULL)
		return;

	free(*pps_wnd);
	*pps_wnd = NULL;
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
	 * query the value every time it presents a frame.
	 */
	if (ps_wnd != NULL)
		ps_wnd->ms_data.m_isvsync = isenabled;
}

void marble_window_update(
	_In_ struct marble_window *ps_wnd,
	     float fFrameTime
) {
	LARGE_INTEGER u_time;
	QueryPerformanceCounter(&u_time);

	/* Set window text to show current FPS. */
	if (u_time.QuadPart - ps_wnd->ms_data.m_lastupdate > 0.5f * gl_pfreq) {
		ps_wnd->ms_data.m_lastupdate = u_time.QuadPart;

		TCHAR az_buffer[256] = { 0 };
		_stprintf_s(
			az_buffer,
			255,
			TEXT("Marble Engine Sandbox - %i FPS"),
			(int)(1.0f / fFrameTime)
		);

		SetWindowText(ps_wnd->mp_handle, az_buffer);
	}
}

void marble_window_resize(
	_In_ struct marble_window *ps_wnd,
	     uint32_t width,
	     uint32_t height,
	     uint32_t tsize
) {
	if (ps_wnd == NULL || width * height * tsize == 0)
		return;

	/* Get window info and window's monitor size. */
	MONITORINFO s_moninfo = { .cbSize = sizeof s_moninfo };
	HMONITOR p_monitor = MonitorFromWindow(
		ps_wnd->mp_handle,
		MONITOR_DEFAULTTOPRIMARY
	);
	if (p_monitor == NULL || GetMonitorInfo(p_monitor, &s_moninfo) == false)
		return;

	/* Compute total window size based on requested render target size. */
	RECT s_wndrect = { 0, 0, (LONG)(width * tsize), (LONG)(height * tsize) };
	AdjustWindowRect(&s_wndrect, ps_wnd->ms_data.m_style, false);

	/* Is our desired size too large for our display device? */
	float const scr_w = (float)abs(s_moninfo.rcWork.right - s_moninfo.rcWork.left);
	float const wnd_w = (float)abs(s_wndrect.right - s_wndrect.left);
	float const scr_h = (float)abs(s_moninfo.rcWork.bottom - s_moninfo.rcWork.top);
	float const wnd_h = (float)abs(s_wndrect.bottom - s_wndrect.top);
	if (wnd_w > scr_w || wnd_h > scr_h) {
		/* Get scale factor by which to scale tile sizes. */
		float const scale = min(scr_w / wnd_w, scr_h / wnd_h);

		/* Calculate new window size. */
		s_wndrect = (RECT){
			0,
			0,
			(int)(width * scale) * tsize,
			(int)(height * scale) * tsize
		};
		AdjustWindowRect(&s_wndrect, ps_wnd->ms_data.m_style, false);

		/* Calculate window and render area size. */
		ps_wnd->ms_data.ms_ext.m_tsize = tsize;

		ps_wnd->ms_data.ms_ext.ms_client = (struct marble_sizei2d){
			(SHORT)(width * scale) * tsize,
			(SHORT)(height * scale) * tsize
		};
		ps_wnd->ms_data.ms_ext.ms_window = (struct marble_sizei2d){
			abs(s_wndrect.right - s_wndrect.left),
			abs(s_wndrect.bottom - s_wndrect.top)
		};
	}

	/* Resize window and renderer. */
	MoveWindow(
		ps_wnd->mp_handle,
		0, 0,
		ps_wnd->ms_data.ms_ext.ms_window.m_width,
		ps_wnd->ms_data.ms_ext.ms_window.m_height,
		true
	);
	marble_window_internal_computedrawingorigin(ps_wnd);
}


