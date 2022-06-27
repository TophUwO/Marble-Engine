#include <internal.h>


struct Marble_Application gl_sApplication = { NULL };


__declspec(noreturn) static void Marble_System_Internal_Cleanup(_Bool blIsForced, int iRetCode) {
	gl_sApplication.dwAppState = 
		blIsForced 
			? Marble_AppState_ForcedShutdown 
			: Marble_AppState_Shutdown
		;

	Marble_Window_Destroy(&gl_sApplication.sMainWindow);
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

	printf("Event: %i\n", sEvent->eType);
	return Marble_ErrorCode_Ok;
}

void destroy(int **x) {
	free(*x);
}
void print(Marble_Util_Vector *sVector) {
	printf("s: %i / c: %i\n", sVector->stSize, sVector->stCapacity);

	for (int i = 0; i < sVector->stSize; i++)
		printf("%i ", *(int *)sVector->ptrpData[i]);

	printf("\n");
}

MARBLE_API int Marble_System_InitializeApplication(HINSTANCE hiInstance, PSTR astrCommandLine) {
#ifdef _DEBUG
	Marble_System_Internal_CreateDebugConsole();
#endif

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

	printf("init: layer stack\n");
	if (iErrorCode = Marble_LayerStack_Initialize())
		Marble_System_Internal_Cleanup(TRUE, iErrorCode);

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
	Marble_System_Internal_Cleanup(sMessage.message == WM_QUIT, 0);
}


