#include <application.h>


struct Marble_Application gl_sApplication = { NULL };


#pragma region initialization functions
static void inline Marble_Application_Internal_CreateHighPrecisionClock(int *ipErrorCode) {
	if (*ipErrorCode)
		return;
		
	printf("init: high-precision clock\n");

	if (!QueryPerformanceFrequency(&gl_sApplication.uPerfFreq))
		Marble_System_RaiseFatalError(*ipErrorCode = Marble_ErrorCode_InitHighPrecClock);
}

static void inline Marble_Application_Internal_InitializeCOMForProcess(int *ipErrorCode) {
	if (*ipErrorCode)
		return;

	printf("init: component object model (COM)\n");

	int iErrorCode = Marble_ErrorCode_Ok;
	if ((iErrorCode = CoInitializeEx(NULL, COINIT_MULTITHREADED)) && iErrorCode ^ S_FALSE)
		Marble_System_RaiseFatalError(*ipErrorCode = Marble_ErrorCode_COMInit);
}

static void inline Marble_Application_Internal_InitializeAppState(int *ipErrorCode, HINSTANCE hiInstance, PSTR astrCommandLine) {
	if (*ipErrorCode)
		return;

	printf("init: application state\n");

	Marble_System_SetAppState(
		FALSE,
		0,
		Marble_AppState_Init
	);

	gl_sApplication.hiInstance      = hiInstance;
	gl_sApplication.astrCommandLine = astrCommandLine;
}

static void inline Marble_Application_Internal_CreateMainWindow(int *ipErrorCode) {
	if (*ipErrorCode)
		return;

	printf("init: main window\n");

	if (*ipErrorCode = Marble_Window_Create(&gl_sApplication.sMainWindow, TEXT("Marble Engine Sandbox"), 512, 512, TRUE))
		Marble_System_RaiseFatalError(*ipErrorCode);
}

static void inline Marble_Application_Internal_CreateRenderer(int *ipErrorCode) {
	if (*ipErrorCode)
		return;

	printf("init: renderer\n");

	if (*ipErrorCode = Marble_Renderer_Create(&gl_sApplication.sRenderer, Marble_RendererAPI_Direct2D, gl_sApplication.sMainWindow->hwWindow))
		Marble_System_RaiseFatalError(*ipErrorCode);
	else
		gl_sApplication.sMainWindow->sRefRenderer = gl_sApplication.sRenderer;
}

static void inline Marble_Application_Internal_CreateLayerStack(int *ipErrorCode) {
	extern int Marble_LayerStack_Create(Marble_LayerStack **ptrpLayerStack);

	if (*ipErrorCode)
		return;

	printf("init: layer stack\n");

	if (*ipErrorCode = Marble_LayerStack_Create(&gl_sApplication.sLayers))
		Marble_System_RaiseFatalError(*ipErrorCode);
}

static void inline Marble_Application_Internal_CreateAssetManager(int *ipErrorCode) {
	extern int Marble_AssetManager_Create(Marble_AssetManager **ptrpAssetManager);

	if (*ipErrorCode)
		return;

	printf("init: asset manager\n");

	if (*ipErrorCode = Marble_AssetManager_Create(&gl_sApplication.sAssets))
		Marble_System_RaiseFatalError(*ipErrorCode);
}

static void inline Marble_Application_Internal_RunUserInitialization(int *ipErrorCode, int (*OnUserInit)(void)) {
	if (*ipErrorCode)
		return;

	printf("init: user application\n");

	OnUserInit();
}
#pragma endregion

Marble_TextFormat *sFmt = NULL;
static int Marble_Application_Internal_UpdateAndRender(float fFrameTime) {
	Marble_Renderer_BeginDraw(gl_sApplication.sRenderer);

	Marble_Renderer_Clear(gl_sApplication.sRenderer, 0.0f, 0.0f, 0.0f, 1.0f);
	for (size_t stIndex = 0; stIndex < gl_sApplication.sLayers->sLayerStack->stSize; stIndex++) {
		Marble_Layer *sLayer = (Marble_Layer *)gl_sApplication.sLayers->sLayerStack->ptrpData[stIndex];

		if (sLayer->blIsEnabled)
			sLayer->sCallbacks.OnUpdate(sLayer, fFrameTime);
	}

	RECT sClientRect = { 0 };
	GetClientRect(gl_sApplication.sMainWindow->hwWindow, &sClientRect);

	float fRectXPos = sClientRect.right  / 2.0f - gl_sApplication.sMainWindow->sWndData.sClientSize.cx / 2.0f;
	float fRectYPos = sClientRect.bottom / 2.0f - gl_sApplication.sMainWindow->sWndData.sClientSize.cy / 2.0f;
	D2D1_RECT_F sRect = {
		fRectXPos,
		fRectYPos,
		fRectXPos + gl_sApplication.sMainWindow->sWndData.sClientSize.cx,
		fRectYPos + gl_sApplication.sMainWindow->sWndData.sClientSize.cy,
	};
	D2D1_BRUSH_PROPERTIES sBrushProps = {
		.opacity = 1.0f
	};
	D2D1_COLOR_F sColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	ID2D1SolidColorBrush *sBrush = NULL;
	D2DWr_DeviceContext_CreateSolidColorBrush(gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext, &sColor, &sBrushProps, &sBrush);
	Marble_Renderer_DrawText(
		gl_sApplication.sRenderer,
		L"Hello, world!",
		sFmt,
		100,100,
		(ID2D1Brush *)sBrush
	);
	D2DWr_DeviceContext_FillRectangle(gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext, &sRect, (ID2D1Brush *)sBrush);

	D2DWr_SolidColorBrush_Release(sBrush);

	Marble_Renderer_EndDraw(gl_sApplication.sRenderer);
	Marble_Window_Update(gl_sApplication.sMainWindow, fFrameTime);

	return Marble_Renderer_Present(&gl_sApplication.sRenderer);
}


int Marble_Application_Internal_OnEvent(void *ptrEvent) {
	Marble_Event *sEvent = (Marble_Event *)ptrEvent;

	for (size_t stIndex = gl_sApplication.sLayers->sLayerStack->stSize - 1; stIndex && stIndex ^ (size_t)(-1) && !sEvent->blIsHandled; stIndex--) {
		Marble_Layer *sLayer = gl_sApplication.sLayers->sLayerStack->ptrpData[stIndex];

		if (sLayer->blIsEnabled)
			sLayer->sCallbacks.OnEvent(sLayer, sEvent);
	}

	return Marble_ErrorCode_Ok;
}


MARBLE_API int Marble_Application_Initialize(HINSTANCE hiInstance, PSTR astrCommandLine, int (*OnUserInit)(void)) {
	gl_sApplication.htMainThread = GetCurrentThread();

#if (defined _DEBUG) || (defined MARBLE_DEVBUILD)
	Marble_System_Internal_CreateDebugConsole();
#endif

	int iErrorCode = Marble_ErrorCode_Ok;

	Marble_Application_Internal_CreateHighPrecisionClock(&iErrorCode);
	Marble_Application_Internal_InitializeCOMForProcess(&iErrorCode);
	Marble_Application_Internal_InitializeAppState(&iErrorCode, hiInstance, astrCommandLine);
	Marble_Application_Internal_CreateMainWindow(&iErrorCode);
	Marble_Application_Internal_CreateRenderer(&iErrorCode);
	Marble_Application_Internal_CreateLayerStack(&iErrorCode);
	Marble_Application_Internal_CreateAssetManager(&iErrorCode);
	Marble_Application_Internal_RunUserInitialization(&iErrorCode, OnUserInit);

	Marble_Renderer_CreateTextFormat(
		gl_sApplication.sRenderer,
		L"Times New Roman",
		36.0f,
		Marble_FontWeight_Black,
		Marble_FontStyle_Italic,
		&sFmt
	);

	/* At last, present window after user has (possibly) made some modifications. */
	if (!iErrorCode) {
		UpdateWindow(gl_sApplication.sMainWindow->hwWindow);
		ShowWindow(gl_sApplication.sMainWindow->hwWindow, SW_SHOWNORMAL);
	}

	return Marble_ErrorCode_Ok;
}

MARBLE_API int Marble_Application_Run(void) {
	gl_sApplication.sAppState.iState = Marble_AppState_Running;

	MSG sMessage = { 0 };
	while (TRUE) {
		LARGE_INTEGER uTime;
		QueryPerformanceCounter(&uTime);
		float fFrameTime = (uTime.QuadPart - gl_sApplication.uFTLast.QuadPart) / (float)gl_sApplication.uPerfFreq.QuadPart;
		gl_sApplication.uFTLast = uTime;

		while (PeekMessage(&sMessage, NULL, 0, 0, PM_REMOVE)) {
			if (sMessage.message == WM_QUIT || sMessage.message == MARBLE_WM_FATAL)
				goto CLEANUP;

			TranslateMessage(&sMessage);
			DispatchMessage(&sMessage);
		}

		if (!gl_sApplication.sMainWindow->sWndData.blIsMinimized)
			Marble_Application_Internal_UpdateAndRender(fFrameTime);
	}
	
CLEANUP:
	if (gl_sApplication.sAppState.blIsFatal == TRUE) {
		TCHAR caBuffer[1024] = { 0 };

		_stprintf_s(
			caBuffer, 
			1024, 
			TEXT(
				"Application has to abruptly quit due to the occurence of\n"
				L"a fatal error:\n\n"
				L"Code: %i\n"
				L"String: %s\n"
				L"Desc: %s\n"
			), 
			gl_sApplication.sAppState.iParameter, 
			Marble_Error_ToString(gl_sApplication.sAppState.iParameter),
			Marble_Error_ToDesc(gl_sApplication.sAppState.iParameter)
		);
		MessageBox(NULL, caBuffer, TEXT("Fatal Error"), MB_ICONERROR | MB_OK);
	}

	return Marble_System_Cleanup(gl_sApplication.sAppState.iParameter);
}


int Marble_Application_GetRenderer(Marble_Renderer **ptrpRenderer) {
	if (gl_sApplication.sRenderer && ptrpRenderer) {
		*ptrpRenderer = gl_sApplication.sRenderer;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_InitState;
}

int Marble_Application_GetMainWindow(Marble_Window **ptrpWindow) {
	if (gl_sApplication.sMainWindow && ptrpWindow) {
		*ptrpWindow = gl_sApplication.sMainWindow;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_InitState;
}

int Marble_Application_GetAssetManager(Marble_AssetManager **ptrpAssetManager) {
	if (gl_sApplication.sAssets && ptrpAssetManager) {
		*ptrpAssetManager = gl_sApplication.sAssets;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_InitState;
}

int Marble_Application_GetLayerStack(Marble_LayerStack **ptrpLayerStack) {
	if (gl_sApplication.sLayers && ptrpLayerStack) {
		*ptrpLayerStack = gl_sApplication.sLayers;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_InitState;
}


