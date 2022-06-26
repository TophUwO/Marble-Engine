#pragma once

#ifdef MARBLEMN_DLLEXP 
	#define MARBLEMN_API extern __declspec(dllexport)
#else
	#define MARBLEMN_API extern __declspec(dllexport)
#endif


MARBLEMN_API int MarbleMain_System_CreateDebugConsole(void);
MARBLEMN_API int MarbleMain_System_InitializeApplication(HINSTANCE hiInstance, PSTR astrCommandLine);
MARBLEMN_API int MarbleMain_System_RegisterHandlers(int (*onInit)(void), int (*onUpdate)(void), int (*onCleanup)(void));
MARBLEMN_API int MarbleMain_System_RunApplication(void);


