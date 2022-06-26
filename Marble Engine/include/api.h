#pragma once

#ifdef MARBLEMN_DLLEXP 
	#define MARBLE_API extern __declspec(dllexport)
#else
	#define MARBLE_API extern __declspec(dllimport)
#endif

#include <windows.h>


