#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <d2dwr.h>

#include <marble.h>
#include <error.h>


enum MarbleMain_Internal_AppState {
	MarbleMain_AppState_Init,
	MarbleMain_AppState_Running,
	MarbleMain_AppState_Shutdown,
	MarbleMain_AppState_ForcedShutdown
};


extern struct MarbleMain_Internal_Application {
	HINSTANCE hiInstance;
	PSTR      astrCommandLine;
	int     (*onInit)(void);
	int     (*onUpdate)(void);
	int     (*onCleanup)(void);
	int       dwAppState;
} gl_sApplication;


