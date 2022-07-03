#include <application.h>


static LRESULT CALLBACK Marble_Window_Internal_WindowProcedure(HWND hwWindow, UINT udwMessage, WPARAM wParam, LPARAM lParam) {
	extern int Marble_System_Internal_OnEvent(void *ptrEvent);

	switch (udwMessage) {
		case WM_CREATE:
			SetWindowLongPtr(hwWindow, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);

			return Marble_ErrorCode_Ok;
		case WM_SIZE:
			Marble_Renderer_Resize((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
			
			return Marble_ErrorCode_Ok;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			Marble_KeyPressedEvent sKeyDownEvent;
			Marble_KeyPressedData  sEventData = {
				.dwKeyCode     = (DWORD)wParam,
				.blIsSysKey    = udwMessage == WM_SYSKEYDOWN
			};
			Marble_Event_ConstructEvent(&sKeyDownEvent, lParam >> 30 & 1 ? Marble_EventType_Keyboard_KeyRepeated : Marble_EventType_Keyboard_KeyPressed, &sEventData);

			return Marble_System_Internal_OnEvent(&sKeyDownEvent);
		}
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			if (wParam == VK_F11) {
				if (gl_sApplication.sMainWindow->sWndData.blIsFullscreen = !gl_sApplication.sMainWindow->sWndData.blIsFullscreen) {
					SetWindowLongPtr(hwWindow, GWL_STYLE, 0);
					SetWindowLongPtr(hwWindow, GWL_EXSTYLE, 0);

					SetWindowPos(hwWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
					ShowWindow(hwWindow, SW_SHOWMAXIMIZED);
				} else {
					SetWindowLongPtr(hwWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW);
					SetWindowLongPtr(hwWindow, GWL_EXSTYLE, WS_EX_CLIENTEDGE);

					ShowWindow(hwWindow, SW_SHOWNORMAL);
					SetWindowPos(hwWindow, HWND_TOP, CW_USEDEFAULT, CW_USEDEFAULT, 512, 512, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
				}

				//printf("Application set to %s mode.\n", gl_sApplication.sMainWindow->sWndData.blIsFullscreen ? "fullscreen" : "windowed");
				return Marble_ErrorCode_Ok;
			}

			Marble_KeyReleasedEvent sKeyReleasedEvent;
			Marble_KeyPressedData sEventData = {
				.dwKeyCode  = (DWORD)wParam,
				.blIsSysKey = udwMessage == WM_SYSKEYUP
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
			Marble_Event_ConstructEvent(&sMouseEvent, Marble_Event_GetMouseEventType(udwMessage), &sEventData);

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
			PostQuitMessage(0);

			return Marble_ErrorCode_Ok;
	}

	return DefWindowProc(hwWindow, udwMessage, wParam, lParam);
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

		(*ptrpWindow)->sWndData.dwWidth        = dwWidth;
		(*ptrpWindow)->sWndData.dwHeight       = dwHeight;
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
			WS_VISIBLE | WS_OVERLAPPEDWINDOW, 
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

	UpdateWindow((*ptrpWindow)->hwWindow);
	return Marble_ErrorCode_Ok;
}

void Marble_Window_Destroy(Marble_Window **ptrpWindow) {
	if (ptrpWindow && *ptrpWindow) {
		free((*ptrpWindow)->sWndData.strTitle);
		free(*ptrpWindow);
		*ptrpWindow = NULL;
	}
}

void Marble_Window_SetVsyncEnabled(Marble_Window *sWindow, _Bool blIsEnabled) {
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

void Marble_Window_Update(float fFrameTime) {
	LARGE_INTEGER uTime;
	QueryPerformanceCounter(&uTime);

	if (uTime.QuadPart - gl_sApplication.sMainWindow->sWndData.uqwLastTitleUpdate > 0.5f * gl_sApplication.uPerfFreq.QuadPart) {
		gl_sApplication.sMainWindow->sWndData.uqwLastTitleUpdate = uTime.QuadPart;

		TCHAR caBuffer[256] = { 0 };
		_stprintf_s(caBuffer, 255, TEXT("Marble Engine Sandbox - %i FPS"), (int)(1.0f / fFrameTime));

		SetWindowText(gl_sApplication.sMainWindow->hwWindow, caBuffer);
	}
}


