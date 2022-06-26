#include <internal.h>


struct Marble_Internal_Application gl_sApplication = { NULL };


static int Marble_System_Internal_Cleanup(_Bool blIsForced) {
	gl_sApplication.dwAppState = 
		blIsForced 
			? Marble_AppState_ForcedShutdown 
			: Marble_AppState_Shutdown
		;

	return Marble_ErrorCode_Ok;
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

	printf("Event: %i\n", sEvent->eType);
	return Marble_ErrorCode_Ok;
}

MARBLE_API int Marble_System_InitializeApplication(HINSTANCE hiInstance, PSTR astrCommandLine) {
	int dwErrorCode = Marble_ErrorCode_Ok;
#ifdef _DEBUG
	Marble_System_Internal_CreateDebugConsole();
#endif

	printf("init: application\n");
	gl_sApplication.dwAppState      = Marble_AppState_Init;
	gl_sApplication.hiInstance      = hiInstance;
	gl_sApplication.astrCommandLine = astrCommandLine;

	printf("init: main window\n");
	if (dwErrorCode = Marble_Window_Create(&gl_sApplication.sMainWindow, TEXT("Marble Engine Sandbox"), 512, 512, TRUE)) {
		if (gl_sApplication.sMainWindow)
			Marble_Window_Destroy(&gl_sApplication.sMainWindow);

		return dwErrorCode;
	}

	return Marble_ErrorCode_Ok;
}

MARBLE_API int Marble_System_RunApplication(void) {
	gl_sApplication.dwAppState = Marble_AppState_Running;

	MSG sMessage = { 0 };
	while (TRUE) {
		while (PeekMessage(&sMessage, NULL, 0, 0, PM_REMOVE)) {
			if (sMessage.message == WM_QUIT)
				goto CLEANUP;

			TranslateMessage(&sMessage);
			DispatchMessage(&sMessage);
		}
	}

CLEANUP:
	return Marble_System_Internal_Cleanup(sMessage.message == WM_QUIT);
}


