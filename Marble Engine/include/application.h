#pragma once

#include <layer.h>
#include <util.h>
#include <window.h>
#include <renderer.h>
#include <asset.h>
#include <atlas.h>
#include <color.h>

#define Marble_IfError(expr, equal, body) if ((expr) != equal) { body; }


enum Marble_Internal_AppState {
	Marble_AppState_Init,
	Marble_AppState_Running,
	Marble_AppState_Shutdown,
	Marble_AppState_ForcedShutdown
};


extern struct Marble_Application {
	HINSTANCE     hiInstance;
	PSTR          astrCommandLine;
	int           iAppState;
	LARGE_INTEGER uPerfFreq;
	LARGE_INTEGER uFTLast;

	Marble_Window       *sMainWindow;
	Marble_Renderer     *sRenderer;
	Marble_AssetManager *sAssets;
	Marble_LayerStack   *sLayers;
} gl_sApplication;

extern ULONGLONG volatile Marble_AssetManager_RequestAssetId(void);

extern int                Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData);
extern TCHAR const *const Marble_Event_GetEventTypeName(Marble_EventType eEventType);
extern Marble_EventType   Marble_Event_GetMouseEventType(UINT udwMessage);

extern void __declspec(noreturn) Marble_System_Exit(int iErrorCode);
extern int  Marble_System_AllocateMemory(void **ptrpMemoryPointer, size_t stSize, _Bool blNeedZeroed);


