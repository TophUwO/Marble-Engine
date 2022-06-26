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

}

MARBLE_API int Marble_System_InitializeApplication(HINSTANCE hiInstance, PSTR astrCommandLine) {
	Marble_System_Internal_CreateDebugConsole();
	printf("init: application\n");

	gl_sApplication.dwAppState      = Marble_AppState_Init;
	gl_sApplication.hiInstance      = hiInstance;
	gl_sApplication.astrCommandLine = astrCommandLine;

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


