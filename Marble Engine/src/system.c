#include <internal.h>


struct MarbleMain_Internal_Application gl_sApplication = { NULL };


static int __MarbleMain_System_Internal_DummyOnInit__(void) { return Marble_ErrorCode_Ok; }
static int __MarbleMain_System_Internal_DummyOnUpdate__(void) { return Marble_ErrorCode_Ok; }
static int __MarbleMain_System_Internal_DummyOnCleanup__(void) { return Marble_ErrorCode_Ok; }

static int MarbleMain_System_Internal_Cleanup(_Bool blIsForced) {
	gl_sApplication.dwAppState = 
		blIsForced 
			? MarbleMain_AppState_ForcedShutdown 
			: MarbleMain_AppState_Shutdown
		;

	return gl_sApplication.onCleanup();
}


int MarbleMain_System_CreateDebugConsole(void) {
	if (AllocConsole()) {
		FILE *fpTmp = NULL;
		freopen_s(&fpTmp, "CONOUT$", "w", stdout);

		printf("init: debug console\n");
		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_CreateDebugConsole;
}

int MarbleMain_System_InitializeApplication(HINSTANCE hiInstance, PSTR astrCommandLine) {
	printf("init: application\n");

	gl_sApplication.dwAppState      = MarbleMain_AppState_Init;
	gl_sApplication.hiInstance      = hiInstance;
	gl_sApplication.astrCommandLine = astrCommandLine;

	return Marble_ErrorCode_Ok;
}

int MarbleMain_System_RegisterHandlers(int (*onInit)(void), int (*onUpdate)(void), int (*onCleanup)(void)) {
	gl_sApplication.onInit    = onInit   ? onInit   : &__MarbleMain_System_Internal_DummyOnInit__;
	gl_sApplication.onUpdate  = onUpdate ? onUpdate : &__MarbleMain_System_Internal_DummyOnUpdate__;
	gl_sApplication.onCleanup = onUpdate ? onCleanup : &__MarbleMain_System_Internal_DummyOnCleanup__;

	return Marble_ErrorCode_Ok;
}

int MarbleMain_System_RunApplication(void) {
	gl_sApplication.onInit();

	gl_sApplication.dwAppState = MarbleMain_AppState_Running;


	MSG sMessage = { 0 };
	while (TRUE) {
		while (PeekMessage(&sMessage, NULL, 0, 0, PM_REMOVE)) {
			if (sMessage.message == WM_QUIT)
				goto CLEANUP;

			TranslateMessage(&sMessage);
			DispatchMessage(&sMessage);
		}

		gl_sApplication.onUpdate();
	}

CLEANUP:
	return MarbleMain_System_Internal_Cleanup(sMessage.message == WM_QUIT);
}


