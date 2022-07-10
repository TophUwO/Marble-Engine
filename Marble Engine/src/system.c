#include <application.h>


struct Marble_Application gl_sApplication = { NULL };

__declspec(noreturn) static void Marble_System_Internal_Cleanup(_Bool blIsForced, int iRetCode) {
	extern void Marble_LayerStack_Destroy(Marble_LayerStack **ptrpLayerstack);
	extern void Marble_AssetManager_Destroy(Marble_AssetManager **ptrpAssetManager);

	gl_sApplication.iAppState =  
		blIsForced 
			? Marble_AppState_ForcedShutdown 
			: Marble_AppState_Shutdown
	;

	Marble_Window_Destroy(&gl_sApplication.sMainWindow);
	Marble_Renderer_Destroy(&gl_sApplication.sRenderer);
	Marble_AssetManager_Destroy(&gl_sApplication.sAssets);
	Marble_LayerStack_Destroy(&gl_sApplication.sLayers);
	CoUninitialize();

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	ExitProcess(iRetCode);
}

static BOOL WINAPI Marble_System_Console_CtrlHandler(DWORD dwCtrlType) {
	switch (dwCtrlType) {
		case CTRL_CLOSE_EVENT:
			PostMessage(gl_sApplication.sMainWindow->hwWindow, WM_CLOSE, 0, 0);

			break;
	}

	return TRUE;
}

static int Marble_System_Internal_CreateDebugConsole(void) {
	if (AllocConsole()) {
		FILE *fpTmp = NULL;
		freopen_s(&fpTmp, "CONOUT$", "w", stdout);

		SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Marble_System_Console_CtrlHandler, TRUE);

		printf("init: debug console\n");
		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_CreateDebugConsole;
}


int Marble_System_Internal_OnEvent(void *ptrEvent) {
	Marble_Event *sEvent = (Marble_Event *)ptrEvent;

	for (size_t stIndex = gl_sApplication.sLayers->sLayerStack->stSize - 1; stIndex && stIndex ^ (size_t)(-1) && !sEvent->blIsHandled; stIndex--) {
		Marble_Layer *sLayer = gl_sApplication.sLayers->sLayerStack->ptrpData[stIndex];

		if (sLayer->blIsEnabled)
			sLayer->sCallbacks.OnEvent(sLayer, sEvent);
	}

	return Marble_ErrorCode_Ok;
}

int Marble_System_Internal_UpdateAndRender(float fFrameTime) {
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
	D2DWr_DeviceContext_FillRectangle(gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext, &sRect, (ID2D1Brush *)sBrush);

	D2DWr_SolidColorBrush_Release(sBrush);

	Marble_Renderer_EndDraw(gl_sApplication.sRenderer);
	Marble_Window_Update(gl_sApplication.sMainWindow, fFrameTime);

	return Marble_Renderer_Present(&gl_sApplication.sRenderer);
}


MARBLE_API int Marble_System_InitializeApplication(HINSTANCE hiInstance, PSTR astrCommandLine, int (*OnUserInit)(void)) {
	extern int Marble_LayerStack_Create(Marble_LayerStack **ptrpLayerStack);
	extern int Marble_AssetManager_Create(Marble_AssetManager **ptrpAssetManager);

	int iErrorCode = Marble_ErrorCode_Ok;

#if (defined _DEBUG) && (defined MARBLE_DEVBUILD)
	Marble_System_Internal_CreateDebugConsole();
#endif
	printf("init: high-precision clock\n");
	if (!QueryPerformanceFrequency(&gl_sApplication.uPerfFreq)) {
		MessageBox(NULL, TEXT("Could not initialize high-precision clock.\n"), TEXT("Fatal Error"), MB_ICONERROR | MB_OK);

		Marble_System_Internal_Cleanup(TRUE, Marble_ErrorCode_InitHighPrecClock);
	}

	printf("init: component object model (COM)\n");
	if ((iErrorCode = CoInitializeEx(NULL, COINIT_MULTITHREADED)) && iErrorCode ^ S_FALSE) {
		MessageBox(NULL, TEXT("Could not initialize COM."), TEXT("Fatal Error"), MB_ICONERROR | MB_OK);

		iErrorCode = Marble_ErrorCode_COMInit;
		Marble_System_Internal_Cleanup(TRUE, Marble_ErrorCode_InitHighPrecClock);
	}

	printf("init: application\n");
	gl_sApplication.iAppState       = Marble_AppState_Init;
	gl_sApplication.hiInstance      = hiInstance;
	gl_sApplication.astrCommandLine = astrCommandLine;

	printf("init: main window\n");
	if (iErrorCode = Marble_Window_Create(&gl_sApplication.sMainWindow, TEXT("Marble Engine Sandbox"), 512, 512, TRUE)) {
		if (gl_sApplication.sMainWindow)
			Marble_Window_Destroy(&gl_sApplication.sMainWindow);

		MessageBox(NULL, TEXT("Could not create main window.\n"), TEXT("Fatal Error"), MB_ICONERROR | MB_OK);

		exit(iErrorCode);
	}

	printf("init: renderer\n");
	if (iErrorCode = Marble_Renderer_Create(&gl_sApplication.sRenderer, Marble_RendererAPI_Direct2D, gl_sApplication.sMainWindow->hwWindow)) {
		MessageBox(NULL, TEXT("Could not initialize renderer."), TEXT("Fatal Error"), MB_ICONERROR | MB_OK);

		Marble_System_Internal_Cleanup(TRUE, iErrorCode);
	}
	gl_sApplication.sMainWindow->sRefRenderer = gl_sApplication.sRenderer;

	printf("init: layer stack\n");
	if (iErrorCode = Marble_LayerStack_Create(&gl_sApplication.sLayers))
		Marble_System_Internal_Cleanup(TRUE, iErrorCode);

	printf("init: asset manager\n");
	Marble_IfError(
		iErrorCode = Marble_AssetManager_Create(&gl_sApplication.sAssets),
		Marble_ErrorCode_Ok,
		Marble_System_Internal_Cleanup(TRUE, iErrorCode)
	);

	/* init user application */
	printf("init: user application\n");
	if (OnUserInit)
		OnUserInit();

	/* At last, present window after user has (possibly) made some modifications. */
	UpdateWindow(gl_sApplication.sMainWindow->hwWindow);
	ShowWindow(gl_sApplication.sMainWindow->hwWindow, SW_SHOWNORMAL);

	return Marble_ErrorCode_Ok;
}

MARBLE_API int Marble_System_RunApplication(void) {
	gl_sApplication.iAppState = Marble_AppState_Running;

	MSG sMessage = { 0 };
	while (TRUE) {
		LARGE_INTEGER uTime;
		QueryPerformanceCounter(&uTime);
		float fFrameTime = (uTime.QuadPart - gl_sApplication.uFTLast.QuadPart) / (float)gl_sApplication.uPerfFreq.QuadPart;
		gl_sApplication.uFTLast = uTime;

		while (PeekMessage(&sMessage, NULL, 0, 0, PM_REMOVE)) {
			if (sMessage.message == WM_QUIT)
				goto CLEANUP;

			TranslateMessage(&sMessage);
			DispatchMessage(&sMessage);
		}

		if (!gl_sApplication.sMainWindow->sWndData.blIsMinimized)
			Marble_System_Internal_UpdateAndRender(fFrameTime);
	}

CLEANUP:
	Marble_System_Internal_Cleanup(FALSE, Marble_ErrorCode_Ok);
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


void __declspec(noreturn) Marble_System_Exit(int iErrorCode) {
	exit(iErrorCode);
}

int Marble_System_AllocateMemory(void **ptrpMemoryPointer, size_t stSize, _Bool blNeedZeroed) {
	if (ptrpMemoryPointer && stSize) {
		if (blNeedZeroed)
			*ptrpMemoryPointer = calloc(1, stSize);
		else
			*ptrpMemoryPointer = malloc(stSize);

		if (!*ptrpMemoryPointer)
			Marble_System_Exit(Marble_ErrorCode_MemoryAllocation);

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_Parameter;
}


