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

struct Marble_Layer_Callbacks {
	int (*OnPush)(Marble_Layer *sSelf);
	int (*OnPop)(Marble_Layer *sSelf);
	int (*OnUpdate)(Marble_Layer *sSelf);
	int (*OnEvent)(Marble_Layer *sSelf, Marble_Event *sEvent);
};

MARBLE_API int           Marble_Layer_Create(Marble_Layer **ptrpLayer, _Bool blIsEnabled);
MARBLE_API int           Marble_Layer_Push(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API Marble_Layer *Marble_Layer_Pop(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API void inline  *Marble_Layer_GetUserdata(Marble_Layer *sLayer);
MARBLE_API void inline  *Marble_Layer_GetCallback(Marble_Layer *sLayer, int iHandlerType);
MARBLE_API void inline   Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled);
MARBLE_API void inline   Marble_Layer_SetCallbacks(Marble_Layer *sLayer, struct Marble_Layer_Callbacks const *sCallbacks);
MARBLE_API void inline  *Marble_Layer_SetUserdata(Marble_Layer *sLayer, void *ptrUserdata);

