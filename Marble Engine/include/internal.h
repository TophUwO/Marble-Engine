#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#include <d2dwr.h>

#include <api.h>
#include <error.h>
#include <event.h>
#include <window.h>


enum Marble_Internal_AppState {
	Marble_AppState_Init,
	Marble_AppState_Running,
	Marble_AppState_Shutdown,
	Marble_AppState_ForcedShutdown
};


extern struct Marble_Internal_Application {
	HINSTANCE hiInstance;
	PSTR      astrCommandLine;
	int       dwAppState;

	struct Marble_Window *sMainWindow;
} gl_sApplication;


extern int Marble_System_Internal_OnEvent(void *ptrEvent);


