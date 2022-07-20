#pragma once

#ifdef MARBLE_DLLEXP 
	#define MARBLE_API extern __declspec(dllexport)
#else
	#define MARBLE_API extern __declspec(dllimport)
#endif


#define MARBLE_VERSION ((DWORD)(0)) /* 0 = indev */

#define Marble_DefAssetMan   ((Marble_AssetManager *)(-1))
#define Marble_DefRenderer   ((Marble_Renderer *)(-1))
#define Marble_DefWindow     ((Marble_Window *)(-1))
#define Marble_DefLayerStack ((Marble_LayerStack *)(-1))


