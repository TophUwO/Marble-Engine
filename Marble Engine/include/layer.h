#pragma once

#include <api.h>
#include <event.h>


enum Marble_LayerHandlerType {
	Marble_LayerHandlerType_Unknown,

	Marble_LayerHandlerType_OnPush,
	Marble_LayerHandlerType_OnPop,
	Marble_LayerHandlerType_OnUpdate,
	Marble_LayerHandlerType_OnEvent
};


typedef struct Marble_Layer Marble_Layer;

MARBLE_API int           Marble_Layer_Create(Marble_Layer **ptrpLayer, _Bool blIsEnabled, DWORD dwSizeOfUserdata);
MARBLE_API int           Marble_Layer_Push(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API Marble_Layer *Marble_Layer_Pop(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API void inline  *Marble_Layer_GetUserdata(Marble_Layer *sLayer);
MARBLE_API void inline  *Marble_Layer_GetHandler(Marble_Layer *sLayer, int iHandlerType);
MARBLE_API void inline   Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled);
MARBLE_API void inline  *Marble_Layer_SetHandler(Marble_Layer *sLayer, int iHandlerType, void *fnptrHandler);
MARBLE_API void inline  *Marble_Layer_SetUserdata(Marble_Layer *sLayer, void *ptrUserdata);

