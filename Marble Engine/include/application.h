#pragma once

#include <layer.h>
#include <util.h>
#include <window.h>
#include <renderer.h>
#include <asset.h>
#include <atlas.h>
#include <color.h>

#define Marble_IfError(expr, equal, body) if ((expr) != equal) { body; }

#define MARBLE_WM_START (WM_USER)
#define MARBLE_WM_FATAL (MARBLE_WM_START + 1)


enum Marble_Internal_AppState {
	Marble_AppState_Init,
	Marble_AppState_Running,
	Marble_AppState_Shutdown,
	Marble_AppState_ForcedShutdown
};


extern struct Marble_Application {
	HINSTANCE     hiInstance;
	PSTR          astrCommandLine;
	LARGE_INTEGER uPerfFreq;
	LARGE_INTEGER uFTLast;
	HANDLE        htMainThread;
	struct Marble_Application_AppState {
		int   iState;
		_Bool blIsFatal;
		int   iParameter;
	} sAppState;

	Marble_Window       *sMainWindow;
	Marble_Renderer     *sRenderer;
	Marble_AssetManager *sAssets;
	Marble_LayerStack   *sLayers;
} gl_sApplication;

extern ULONGLONG volatile Marble_AssetManager_RequestAssetId(void);

extern int                Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData);
extern TCHAR const *const Marble_Event_GetEventTypeName(Marble_EventType eEventType);
extern Marble_EventType   Marble_Event_GetMouseEventType(UINT udwMessage);

extern int  Marble_System_Internal_CreateDebugConsole(void);
extern int  Marble_System_Cleanup(int iRetCode);
extern void Marble_System_InitiateShutdown(int iRetCode);
extern void Marble_System_RaiseFatalError(int iErrorCode);
extern void Marble_System_SetAppState(_Bool blIsFatal, int iParameter, int iAppState);
extern void Marble_System_ClearAppState(void);
extern int  Marble_System_AllocateMemory(void **ptrpMemoryPointer, size_t stSize, _Bool blNeedZeroed);


