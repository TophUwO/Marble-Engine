#include <application.h>


static LRESULT CALLBACK Marble_Window_Internal_WindowProcedure(HWND hwWindow, UINT uiMessage, WPARAM wParam, LPARAM lParam) {
	extern int Marble_System_Internal_OnEvent(void *ptrEvent);

	struct Marble_Window *sWindowData = NULL;

	switch (uiMessage) {
		case WM_CREATE:
			SetWindowLongPtr(hwWindow, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);

			return Marble_ErrorCode_Ok;
		case WM_SIZE: {
			sWindowData = (Marble_Window *)GetWindowLongPtr(hwWindow, GWLP_USERDATA);
			sWindowData->sWndData.blIsMinimized = wParam == SIZE_MINIMIZED;

			/* Ignore message if window initialization is not complete yet. */
			if (sWindowData && sWindowData->hwWindow && sWindowData->sRefRenderer)
				Marble_Renderer_Resize(sWindowData->sRefRenderer, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
			
			return Marble_ErrorCode_Ok;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			Marble_KeyPressedEvent sKeyDownEvent;
			Marble_KeyPressedData  sEventData = {
				.dwKeyCode     = (DWORD)wParam,
				.blIsSysKey    = uiMessage == WM_SYSKEYDOWN
			};
			Marble_Event_ConstructEvent(&sKeyDownEvent, lParam >> 30 & 1 ? Marble_EventType_Keyboard_KeyRepeated : Marble_EventType_Keyboard_KeyPressed, &sEventData);

			return Marble_System_Internal_OnEvent(&sKeyDownEvent);
		}
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			if (wParam == VK_F11) {
				sWindowData = (Marble_Window *)GetWindowLongPtr(hwWindow, GWLP_USERDATA);

				if (sWindowData->sWndData.blIsFullscreen = !sWindowData->sWndData.blIsFullscreen) {
					SetWindowLongPtr(hwWindow, GWL_STYLE, 0);
					SetWindowLongPtr(hwWindow, GWL_EXSTYLE, 0);

					SetWindowPos(hwWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
					ShowWindow(hwWindow, SW_SHOWMAXIMIZED);
				} else {
					SetWindowLongPtr(hwWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW);
					SetWindowLongPtr(hwWindow, GWL_EXSTYLE, 0);

					ShowWindow(hwWindow, SW_SHOWNORMAL);
					SetWindowPos(
						hwWindow, 
						HWND_TOP, 
						0, 
						0, 
						sWindowData->sWndData.sWindowSize.cx, 
						sWindowData->sWndData.sWindowSize.cy, 
						SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED
					);
				}

				return Marble_ErrorCode_Ok;
			}

			Marble_KeyReleasedEvent sKeyReleasedEvent;
			Marble_KeyPressedData sEventData = {
				.dwKeyCode  = (DWORD)wParam,
				.blIsSysKey = uiMessage == WM_SYSKEYUP
			};
			Marble_Event_ConstructEvent(&sKeyReleasedEvent, Marble_EventType_Keyboard_KeyReleased, &sEventData);

			return Marble_System_Internal_OnEvent(&sKeyReleasedEvent);
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE: {
			Marble_MouseEvent sMouseEvent;
			Marble_MouseData sEventData = {
				.dwButtonCode = (DWORD)wParam,
				.sPos = { 
					.x = GET_X_LPARAM(lParam), 
					.y = GET_Y_LPARAM(lParam)
				}
			};
			Marble_Event_ConstructEvent(&sMouseEvent, Marble_Event_GetMouseEventType(uiMessage), &sEventData);

			return Marble_System_Internal_OnEvent(&sMouseEvent);
		}
		case WM_CLOSE: {
			Marble_WindowClosedEvent sWindowClosedEvent;
			Marble_Event_ConstructEvent(&sWindowClosedEvent, Marble_EventType_Window_Closed, NULL);

			Marble_System_Internal_OnEvent(&sWindowClosedEvent);

			DestroyWindow(hwWindow);
			return Marble_ErrorCode_Ok;
		}
		case WM_DESTROY:
			if (hwWindow == gl_sApplication.sMainWindow->hwWindow)
				PostQuitMessage(0);

			return Marble_ErrorCode_Ok;
	}

	return DefWindowProc(hwWindow, uiMessage, wParam, lParam);
}


int Marble_Window_Create(Marble_Window **ptrpWindow, TCHAR *strTitle, DWORD dwWidth, DWORD dwHeight, _Bool blIsVSync) {
	static TCHAR const *const gl_sWindowClassName = TEXT("Marble_Window");

	if (*ptrpWindow = malloc(sizeof(**ptrpWindow))) {
		(*ptrpWindow)->hwWindow = NULL;

		size_t stTitleLen = _tcslen(strTitle);
		if ((*ptrpWindow)->sWndData.strTitle = calloc(1, sizeof(*strTitle) * (stTitleLen + 1)))
			_tcscpy_s((*ptrpWindow)->sWndData.strTitle, stTitleLen + 1, strTitle);
		else 
			return Marble_ErrorCode_MemoryAllocation;

		(*ptrpWindow)->sWndData.sWindowSize    = (SIZE){ (SHORT)dwWidth, (SHORT)dwHeight };
		(*ptrpWindow)->sWndData.blIsVSync      = TRUE;
		(*ptrpWindow)->sWndData.blIsFullscreen = FALSE;
	} else 
		return Marble_ErrorCode_MemoryAllocation;

	WNDCLASSEX sWindowClass = {
		.cbSize        = sizeof(sWindowClass),
		.hInstance     = gl_sApplication.hiInstance,
		.lpszClassName = gl_sWindowClassName,
		.lpfnWndProc   = (WNDPROC)&Marble_Window_Internal_WindowProcedure,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hIcon         = LoadIcon(NULL, IDI_APPLICATION),
		.hIconSm       = LoadIcon(NULL, IDI_APPLICATION),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1)
	};
	if (!RegisterClassEx(&sWindowClass))
		return Marble_ErrorCode_RegisterWindowClass;

	if (!((*ptrpWindow)->hwWindow = 
		CreateWindowEx(
			0, 
			gl_sWindowClassName, 
			strTitle, 
			WS_OVERLAPPEDWINDOW, 
			CW_USEDEFAULT, 
			CW_USEDEFAULT, 
			(int)dwWidth, 
			(int)dwHeight, 
			NULL, 
			NULL, 
			gl_sApplication.hiInstance, 
			(LPVOID)*ptrpWindow
		))) {
		UnregisterClass(gl_sWindowClassName, gl_sApplication.hiInstance);

		return Marble_ErrorCode_CreateWindow;
	}

	RECT sClientSize = { 0 };
	GetClientRect((*ptrpWindow)->hwWindow, &sClientSize);
	(*ptrpWindow)->sWndData.sClientSize = (SIZE){ (SHORT)sClientSize.right, (SHORT)sClientSize.right };

	return Marble_ErrorCode_Ok;
}

void Marble_Window_Destroy(Marble_Window **ptrpWindow) {
	if (ptrpWindow && *ptrpWindow) {
		free((*ptrpWindow)->sWndData.strTitle);
		free(*ptrpWindow);
		*ptrpWindow = NULL;
	}
}

void Marble_Window_SetVSyncEnabled(Marble_Window *sWindow, _Bool blIsEnabled) {
	if (sWindow) {
		sWindow->sWndData.blIsVSync = blIsEnabled;
	}
}

void Marble_Window_SetFullscreen(Marble_Window *sWindow, _Bool blIsFullscreen) {
	if (sWindow) {
		sWindow->sWndData.blIsFullscreen = blIsFullscreen;

		PostMessage(sWindow->hwWindow, WM_KEYDOWN, (WPARAM)VK_F11, 0);
	}
}

void Marble_Window_Update(Marble_Window *sWindow, float fFrameTime) {
	LARGE_INTEGER uTime;
	QueryPerformanceCounter(&uTime);

	if (uTime.QuadPart - sWindow->sWndData.uqwLastTitleUpdate > 0.5f * gl_sApplication.uPerfFreq.QuadPart) {
		sWindow->sWndData.uqwLastTitleUpdate = uTime.QuadPart;

		TCHAR caBuffer[256] = { 0 };
		_stprintf_s(caBuffer, 255, TEXT("Marble Engine Sandbox - %i FPS"), (int)(1.0f / fFrameTime));

		SetWindowText(sWindow->hwWindow, caBuffer);
	}
}

void Marble_Window_SetSize(Marble_Window *sWindow, int iWidthInTiles, int iHeightInTiles, int iTileSize) {
	if (!sWindow || !iWidthInTiles || !iHeightInTiles || !iTileSize)
		return;

	/* Get window's monitor size */
	SIZE        sEndSize  = { 0 };
	MONITORINFO sMonInfo  = { .cbSize = sizeof(sMonInfo) };
	HMONITOR    hmMonitor = MonitorFromWindow(gl_sApplication.sMainWindow->hwWindow, MONITOR_DEFAULTTOPRIMARY);
	GetMonitorInfo(hmMonitor, &sMonInfo);

	/* Can desired resolution be displayed on current monitor? */
	RECT sWindowRect = { 0, 0, iWidthInTiles * iTileSize, iHeightInTiles * iTileSize };
	AdjustWindowRectEx(&sWindowRect, WS_OVERLAPPEDWINDOW, FALSE, 0);

	/* Is our desired size too large for our display device? */
	int const iScreenWidth  = abs(sMonInfo.rcWork.right - sMonInfo.rcWork.left);
	int const iWindowWidth  = abs(sWindowRect.right - sWindowRect.left);
	int const iScreenHeight = abs(sMonInfo.rcWork.bottom - sMonInfo.rcWork.top);
	int const iWindowHeight = abs(sWindowRect.bottom - sWindowRect.top);
	if (iWindowWidth > iScreenWidth || iWindowHeight > iScreenHeight) {
		/* Get scale factor by which to scale tile sizes */
		float const fScale = min(iScreenWidth / (float)iWindowWidth, iScreenHeight / (float)iWindowHeight);

		/* Calculate new window size */
		sWindowRect = (RECT){ 0, 0, (int)(iWidthInTiles * fScale) * iTileSize, (int)(iHeightInTiles * fScale) * iTileSize };
		AdjustWindowRectEx(&sWindowRect, WS_OVERLAPPEDWINDOW, FALSE, 0);

		sWindow->sWndData.sClientSize = (SIZE){ (SHORT)(iWidthInTiles * fScale), (SHORT)(iHeightInTiles * fScale) };
		sWindow->sWndData.sWindowSize = (SIZE){ abs(sWindowRect.right - sWindowRect.left), abs(sWindowRect.bottom - sWindowRect.top) };
	}

	/* Resize window and renderer */
	MoveWindow(sWindow->hwWindow, 0, 0, sWindow->sWndData.sWindowSize.cx, sWindow->sWndData.sWindowSize.cy, TRUE);
}


