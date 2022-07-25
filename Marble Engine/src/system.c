#include <application.h>


int Marble_System_Cleanup(int iRetCode) {
	extern void Marble_LayerStack_Destroy(Marble_LayerStack **ptrpLayerstack);
	extern void Marble_AssetManager_Destroy(void);

	Marble_Window_Destroy(&gl_sApplication.sMainWindow);
	Marble_Renderer_Destroy(&gl_sApplication.sRenderer);
	Marble_LayerStack_Destroy(&gl_sApplication.sLayers);
	Marble_AssetManager_Destroy();
	CoUninitialize();

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return iRetCode;
}

static BOOL WINAPI Marble_System_Console_CtrlHandler(DWORD dwCtrlType) {
	switch (dwCtrlType) {
		case CTRL_CLOSE_EVENT:
			PostThreadMessage(GetThreadId(gl_sApplication.htMainThread), WM_QUIT, 0, 0);

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

int Marble_System_AllocateMemory(void **ptrpMemoryPointer, size_t stSize, _Bool blNeedZeroed, _Bool blIsFatalOnFailure) {
	if (!ptrpMemoryPointer || !stSize)
		return Marble_ErrorCode_InternalParameter;

	if (blNeedZeroed)
		*ptrpMemoryPointer = calloc(1, stSize);
	else
		*ptrpMemoryPointer = malloc(stSize);

	if (!*ptrpMemoryPointer) {
		if (blIsFatalOnFailure)
			Marble_System_RaiseFatalError(Marble_ErrorCode_MemoryAllocation);

		return Marble_ErrorCode_MemoryAllocation;
	}

	printf("Allocator: Allocated memory of %i bytes, starting at address: 0x%p.\n", 
		(int)stSize,
		*ptrpMemoryPointer
	);

	return Marble_ErrorCode_Ok;
}


#pragma region User API
/* 
	* Wrapper functions for those functions that use some parameters 
	* only internally but are of no use to the user.
*/
static void Marble_Asset_Internal_Destroy_Impl(Marble_Asset **ptrpAsset) {
	Marble_Asset_Destroy(ptrpAsset, FALSE);
}

static int Marble_Asset_Internal_Create_Impl(int iAssetType, CHAR const *strID, void const *ptrCreateParams, Marble_Asset **ptrpAsset) {
	return Marble_Asset_Create(
		iAssetType,
		strID,
		ptrCreateParams,
		ptrpAsset,
		NULL
	);
}

static int Marble_Asset_Internal_LoadFromFile_Impl(TCHAR const *strPath, Marble_Asset **ptrpAsset) {
	return Marble_Asset_LoadFromFile(
		strPath,
		ptrpAsset,
		NULL
	);
}


struct Marble_UserAPI Marble = {
	.Renderer = {
		.Clear = &Marble_Renderer_Clear
	},

	.Window = {
		.setSize       = &Marble_Window_SetSize,
		.setFullscreen = &Marble_Window_SetFullscreen,
		.setVSync      = &Marble_Window_SetVSyncEnabled
	},

	.Layer = {
		.Create        = &Marble_Layer_Create,
		.CreateAndPush = &Marble_Layer_CreateAndPush,
		.Destroy       = &Marble_Layer_Destroy,
		.Push          = &Marble_Layer_Push,
		.Pop           = &Marble_Layer_Pop,
		.getUserdata   = &Marble_Layer_GetUserdata,
		.getCallback   = &Marble_Layer_GetCallback,
		.isEnabled     = &Marble_Layer_IsEnabled,
		.setEnabled    = &Marble_Layer_SetEnabled,
		.setCallbacks  = &Marble_Layer_SetCallbacks,
		.setUserdata   = &Marble_Layer_SetUserdata
	},

	.Asset = {
		.Create       = &Marble_Asset_Internal_Create_Impl,
		.LoadFromFile = &Marble_Asset_Internal_LoadFromFile_Impl,
		.Destroy      = &Marble_Asset_Internal_Destroy_Impl,
		.getType      = &Marble_Asset_GetType,
		.getID        = &Marble_Asset_GetId,
		.Register     = &Marble_Asset_Register,
		.Unregister   = &Marble_Asset_Unregister,
		.Obtain       = &Marble_Asset_Obtain,
		.Release      = &Marble_Asset_Release
	}
};
#pragma endregion


