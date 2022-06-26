#include <mbmn_internal.h>


//struct MarbleMain_Internal_Direct2DRenderer gl_sDirect2DRenderer = { NULL };
//
//
//static LRESULT WINAPI MarbleMain_Renderer_Internal_RenderWndProc(HWND hwWindow, UINT udwMessage, WPARAM wParam, LPARAM lParam) {
//	switch (udwMessage) {
//		case WM_CLOSE:
//			DestroyWindow(hwWindow);
//
//			return 0;
//		case WM_DESTROY:
//			PostQuitMessage(Marble_ErrorCode_Ok);
//
//			return 0;
//	}
//
//	return DefWindowProc(hwWindow, udwMessage, wParam, lParam);
//}
//
//static int MarbleMain_Renderer_Internal_CreateRenderWindow(DWORD dwWidth, DWORD dwHeight, TCHAR const *strTitle) {
//	static TCHAR const *const gl_strRenderWndClassName = TEXT("MarbleMain_RenderWindow");
//
//	WNDCLASSEX sWindowProps = {
//		.cbSize        = sizeof(sWindowProps),
//		.hInstance     = gl_sApplication.hiInstance,
//		.lpszClassName = gl_strRenderWndClassName,
//		.hbrBackground = NULL,
//		.lpfnWndProc   = (WNDPROC)&MarbleMain_Renderer_Internal_RenderWndProc
//	};
//	if (!RegisterClassEx(&sWindowProps))
//		return Marble_ErrorCode_RegisterWindowClass;
//
//	if (!(gl_sDirect2DRenderer.hwRenderWindow = CreateWindowEx(WS_EX_CLIENTEDGE, gl_strRenderWndClassName, strTitle, WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, dwWidth, dwHeight, NULL, NULL, gl_sApplication.hiInstance, NULL))) {
//		UnregisterClass(gl_strRenderWndClassName, gl_sApplication.hiInstance);
//
//		return Marble_ErrorCode_CreateRenderWindow;
//	}
//	UpdateWindow(gl_sDirect2DRenderer.hwRenderWindow);
//
//	gl_sDirect2DRenderer.dwWidth  = dwWidth;
//	gl_sDirect2DRenderer.dwHeight = dwHeight;
//	return Marble_ErrorCode_Ok;
//}
//
//
//int MarbleMain_Renderer_Initialize(void) {
//	int dwErrorCode = Marble_ErrorCode_Ok;
//	if (dwErrorCode = MarbleMain_Renderer_Internal_CreateRenderWindow(1024, 1024, TEXT("Marble Sandbox")))
//		return dwErrorCode;
//
//	if (D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, &gl_sDirect2DRenderer.sFactory))
//		return Marble_ErrorCode_CreateDirect2DFactory;
//
//	if (MbWrD2D_Factory_CreateHwndRenderTarget(gl_sDirect2DRenderer.sFactory, ))
//}


