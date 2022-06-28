#pragma once

#include <layer.h>
#include <util.h>
#include <window.h>


enum Marble_Internal_AppState {
	Marble_AppState_Init,
	Marble_AppState_Running,
	Marble_AppState_Shutdown,
	Marble_AppState_ForcedShutdown
};


extern struct Marble_Application {
	HINSTANCE     hiInstance;
	PSTR          astrCommandLine;
	int           dwAppState;
	LARGE_INTEGER uPerfFreq;

	Marble_Window *sMainWindow;
	struct Marble_LayerStack {
		Marble_Util_Vector *sLayerStack;

		size_t stLastLayer;
	} sLayers;
} gl_sApplication;


extern int Marble_System_Internal_OnEvent(void *ptrEvent);

extern int                Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData);
extern TCHAR const *const Marble_Event_GetEventTypeName(Marble_EventType eEventType);
extern Marble_EventType   Marble_Event_GetMouseEventType(UINT udwMessage);

extern int  Marble_LayerStack_Initialize(void);
extern void Marble_LayerStack_Destroy(void);


