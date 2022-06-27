#pragma once

#ifdef MARBLE_DLLEXP 
	#define MARBLE_API extern __declspec(dllexport)
#else
	#define MARBLE_API extern __declspec(dllimport)
#endif


