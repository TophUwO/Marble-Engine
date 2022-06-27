#pragma once

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#ifdef _DEBUG
	#include <crtdbg.h>
#endif

#include <d2dwr.h>

#include <api.h>
#include <error.h>
#include <event.h>
#include <window.h>
#include <layer.h>
#include <util.h>


enum Marble_Internal_AppState {
	Marble_AppState_Init,
	Marble_AppState_Running,
	Marble_AppState_Shutdown,
	Marble_AppState_ForcedShutdown
};


extern struct Marble_Application {
	HINSTANCE hiInstance;
	PSTR      astrCommandLine;
	int       dwAppState;

	struct Marble_Window *sMainWindow;
	struct Marble_LayerStack {
		struct Marble_Util_Vector *sLayerStack;

		size_t stLastLayer;
	} sLayers;
} gl_sApplication;


extern int Marble_System_Internal_OnEvent(void *ptrEvent);

extern int                       Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData);
extern TCHAR const *const        Marble_Event_GetEventTypeName(Marble_EventType eEventType);
extern Marble_EventType   inline Marble_Event_GetMouseEventType(UINT udwMessage);


