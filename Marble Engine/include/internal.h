#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <d2dwr.h>

#include <api.h>
#include <error.h>
#include <event.h>


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
} gl_sApplication;


extern int Marble_System_Internal_OnEvent(void *ptrEvent);


