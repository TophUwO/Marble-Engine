#include <window.h>


static LRESULT CALLBACK Marble_Window_Internal_WindowProcedure(HWND hwWindow, UINT udwMessage, WPARAM wParam, LPARAM lParam) {
	switch (udwMessage) {
		case WM_CREATE:
			SetWindowLongPtr(hwWindow, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);

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
			Marble_KeyReleasedEvent sKeyReleasedEvent;
			Marble_KeyPressedData sEventData = {
				.dwKeyCode  = (DWORD)wParam,
				.blIsSysKey = udwMessage == WM_SYSKEYUP
			};
			Marble_Event_ConstructEvent(&sKeyReleasedEvent, Marble_EventType_Keyboard_KeyReleased, &sEventData);

			return Marble_System_Internal_OnEvent(&sKeyReleasedEvent);
		}
		case WM_CLOSE: {
			Marble_WindowEvent sWindowEvent;
			Marble_Event_ConstructEvent(&sWindowEvent, Marble_EventType_Window_Closed, NULL);

			Marble_System_Internal_OnEvent(&sWindowEvent);

			DestroyWindow(hwWindow);
			return Marble_ErrorCode_Ok;
		}
		case WM_DESTROY:
			Marble_Window_Destroy(&gl_sApplication.sMainWindow);
			PostQuitMessage(0);

			return Marble_ErrorCode_Ok;
	}

	return DefWindowProc(hwWindow, udwMessage, wParam, lParam);
}


int Marble_Window_Create(Marble_Window **ptrpWindow, TCHAR *strTitle, DWORD dwWidth, DWORD dwHeight, _Bool blIsVSync) {
	static TCHAR const *const gl_sWindowClassName = TEXT("Marble_Window");

	if (*ptrpWindow = malloc(sizeof(**ptrpWindow))) {
		(*ptrpWindow)->hwWindow = NULL;

		DWORD dwTitleLen = _tcslen(strTitle);
		if ((*ptrpWindow)->sWndData.strTitle = calloc(1, sizeof(*strTitle) * (dwTitleLen + 1)))
			_tcscpy_s((*ptrpWindow)->sWndData.strTitle, dwTitleLen + 1, strTitle);
		else 
			return Marble_ErrorCode_MemoryAllocation;

		(*ptrpWindow)->sWndData.dwWidth   = dwWidth;
		(*ptrpWindow)->sWndData.dwHeight  = dwHeight;
		(*ptrpWindow)->sWndData.blIsVSync = blIsVSync;
	} else 
		return Marble_ErrorCode_MemoryAllocation;

	WNDCLASSEX sWindowClass = {
		.cbSize        = sizeof(sWindowClass),
		.hInstance     = gl_sApplication.hiInstance,
		.lpszClassName = gl_sWindowClassName,
		.lpfnWndProc   = (WNDPROC)&Marble_Window_Internal_WindowProcedure,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hIcon         = LoadIcon(NULL, IDI_APPLICATION),
		.hIconSm       = LoadIcon(NULL, IDI_APPLICATION)
	};
	if (!RegisterClassEx(&sWindowClass))
		return Marble_ErrorCode_RegisterWindowClass;

	if (!((*ptrpWindow)->hwWindow = 
		CreateWindowEx(
			WS_EX_CLIENTEDGE, 
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
		))) return Marble_ErrorCode_CreateWindow;

	UpdateWindow((*ptrpWindow)->hwWindow);
	return Marble_ErrorCode_Ok;
}

int Marble_Window_Destroy(Marble_Window **ptrpWindow) {
	free((*ptrpWindow)->sWndData.strTitle);
	free(*ptrpWindow);

	*ptrpWindow = NULL;
	return Marble_ErrorCode_Ok;
}

int Marble_Window_OnUpdate(Marble_Window *sWindow) {
	return Marble_ErrorCode_Ok;
}


