#pragma once

#ifdef MARBLE_DLLEXP 
	#define MARBLE_API extern __declspec(dllexport)
#else
	#define MARBLE_API extern __declspec(dllimport)
#endif


#define MARBLE_VERSION ((DWORD)(0)) /* 0 = indev */


