#include <application.h>


int Marble_System_Cleanup(int iRetCode) {
	extern void Marble_LayerStack_Destroy(Marble_LayerStack **ptrpLayerstack);
	extern void Marble_AssetManager_Destroy(Marble_AssetManager **ptrpAssetManager);

	Marble_Window_Destroy(&gl_sApplication.sMainWindow);
	Marble_Renderer_Destroy(&gl_sApplication.sRenderer);
	Marble_AssetManager_Destroy(&gl_sApplication.sAssets);
	Marble_LayerStack_Destroy(&gl_sApplication.sLayers);
	CoUninitialize();

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return iRetCode;
}

static BOOL WINAPI Marble_System_Console_CtrlHandler(DWORD dwCtrlType) {
	switch (dwCtrlType) {
		case CTRL_CLOSE_EVENT:
			PostMessage(gl_sApplication.sMainWindow->hwWindow, WM_CLOSE, 0, 0);

			break;
	}

	return TRUE;
}

int Marble_System_Internal_CreateDebugConsole(void) {
	if (AllocConsole()) {
		FILE *fpTmp = NULL;
		freopen_s(&fpTmp, "CONOUT$", "w", stdout);

		SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Marble_System_Console_CtrlHandler, TRUE);

		printf("init: debug console\n");
		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_CreateDebugConsole;
}


void Marble_System_SetAppState(_Bool blIsFatal, int iParameter, int iAppState) {
	gl_sApplication.sAppState = (struct Marble_Application_AppState){
		.blIsFatal = blIsFatal,
		.iState    = iAppState
	};

	gl_sApplication.sAppState.iParameter = iParameter;
}

void Marble_System_InitiateShutdown(int iRetCode) {
	Marble_System_SetAppState(
		FALSE,
		iRetCode,
		Marble_AppState_Shutdown
	);

	SendMessage(gl_sApplication.sMainWindow->hwWindow, WM_CLOSE, 0, 0);
}

void Marble_System_RaiseFatalError(int iErrorCode) {
	Marble_System_SetAppState(
		TRUE,
		iErrorCode,
		Marble_AppState_ForcedShutdown
	);

	PostThreadMessage(GetCurrentThreadId(), MARBLE_WM_FATAL, 0, 0);
}

void Marble_System_ClearAppState(void) {
	gl_sApplication.sAppState = (struct Marble_Application_AppState){
		.blIsFatal  = FALSE,
		.iParameter = 0,
		.iState     = Marble_AppState_Running
	};
}

int Marble_System_AllocateMemory(void **ptrpMemoryPointer, size_t stSize, _Bool blNeedZeroed) {
	if (ptrpMemoryPointer && stSize) {
		if (blNeedZeroed)
			*ptrpMemoryPointer = calloc(1, stSize);
		else
			*ptrpMemoryPointer = malloc(stSize);

		if (!*ptrpMemoryPointer)
			Marble_System_RaiseFatalError(Marble_ErrorCode_MemoryAllocation);

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_Parameter;
}


