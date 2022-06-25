#pragma once

#ifdef MARBLEMN_DLLEXP 
	#define MARBLEMN_API extern __declspec(dllexport)
#else
	#define MARBLEMN_API extern __declspec(dllexport)
#endif


MARBLEMN_API int MarbleMain_System_CreateDebugConsole(void);


