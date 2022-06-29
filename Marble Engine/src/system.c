#include <application.h>


struct Marble_Application gl_sApplication = { NULL };


__declspec(noreturn) static void Marble_System_Internal_Cleanup(_Bool blIsForced, int iRetCode) {
	gl_sApplication.dwAppState = 
		blIsForced 
			? Marble_AppState_ForcedShutdown 
			: Marble_AppState_Shutdown
		;

	Marble_Window_Destroy(&gl_sApplication.sMainWindow);
	Marble_Renderer_Uninitialize();
	Marble_LayerStack_Destroy();

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	exit(iRetCode);
}

static int Marble_System_Internal_CreateDebugConsole(void) {
	if (AllocConsole()) {
		FILE *fpTmp = NULL;
		freopen_s(&fpTmp, "CONOUT$", "w", stdout);

		printf("init: debug console\n");
		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_CreateDebugConsole;
}


int Marble_System_Internal_OnEvent(void *ptrEvent) {
	Marble_GenericEvent *sEvent = (Marble_GenericEvent *)ptrEvent;

	if (sEvent->eType == Marble_EventType_Mouse_MouseMoved) {
		Marble_MouseMovedEvent *sMovedEvent = (Marble_MouseMovedEvent *)ptrEvent;

		printf("[EVENT '%S']: %i (%i / %i)\n", 
			Marble_Event_GetEventTypeName(sEvent->eType), 
			sEvent->eType,
			sMovedEvent->sPos.x,
			sMovedEvent->sPos.y
		);
	} else
		printf("[EVENT '%S']: %i\n", Marble_Event_GetEventTypeName(sEvent->eType), sEvent->eType);

	return Marble_ErrorCode_Ok;
}

MARBLE_API int Marble_System_InitializeApplication(HINSTANCE hiInstance, PSTR astrCommandLine) {
#ifdef _DEBUG
	Marble_System_Internal_CreateDebugConsole();
#endif
	printf("init: high-precision clock\n");
	if (!QueryPerformanceFrequency(&gl_sApplication.uPerfFreq)) {
		MessageBox(NULL, TEXT("Could not initialize high-precision clock.\n"), TEXT("Fatal Error"), MB_ICONERROR | MB_OK);

		Marble_System_Internal_Cleanup(TRUE, Marble_ErrorCode_InitHighPrecClock);
	}

	printf("init: application\n");
	gl_sApplication.dwAppState      = Marble_AppState_Init;
	gl_sApplication.hiInstance      = hiInstance;
	gl_sApplication.astrCommandLine = astrCommandLine;

	printf("init: main window\n");
	int iErrorCode = Marble_ErrorCode_Ok;
	if (iErrorCode = Marble_Window_Create(&gl_sApplication.sMainWindow, TEXT("Marble Engine Sandbox"), 512, 512, TRUE)) {
		if (gl_sApplication.sMainWindow)
			Marble_Window_Destroy(&gl_sApplication.sMainWindow);

		MessageBox(NULL, TEXT("Could not create main window.\n"), TEXT("Fatal Error"), MB_ICONERROR | MB_OK);

		exit(iErrorCode);
	}

	printf("init: renderer\n");
	if (iErrorCode = Marble_Renderer_Initialize(Marble_RendererAPI_Direct2D, gl_sApplication.sMainWindow->hwWindow)) {
		MessageBox(NULL, TEXT("Could not initialize renderer."), TEXT("Fatal Error"), MB_ICONERROR | MB_OK);

		Marble_System_Internal_Cleanup(TRUE, iErrorCode);
	}

	printf("init: layer stack\n");
	if (iErrorCode = Marble_LayerStack_Initialize())
		Marble_System_Internal_Cleanup(TRUE, iErrorCode);

	return Marble_ErrorCode_Ok;
}

MARBLE_API int Marble_System_RunApplication(void) {
	gl_sApplication.dwAppState = Marble_AppState_Running;

	MSG sMessage = { 0 };
	while (TRUE) {
		Marble_Util_Clock sTimer;
		Marble_Util_Clock_Start(&sTimer);

		while (PeekMessage(&sMessage, NULL, 0, 0, PM_REMOVE)) {
			if (sMessage.message == WM_QUIT)
				goto CLEANUP;

			TranslateMessage(&sMessage);
			DispatchMessage(&sMessage);
		}

		D2DWr_DeviceContext_BeginDraw(gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext);
		D2D1_COLOR_F sCol = { 1.0f, 0.3f, 0.3f, 1.0f };
		D2DWr_DeviceContext_Clear(gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext, &sCol);

		D2DWr_DeviceContext_EndDraw(gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext, NULL, NULL);
		Marble_Renderer_Present();

		Marble_Util_Clock_Stop(&sTimer);
		printf("Time: %g ms\n", Marble_Util_Clock_AsMilliseconds(&sTimer));
	}

CLEANUP:
	Marble_System_Internal_Cleanup(FALSE, 0);
}


