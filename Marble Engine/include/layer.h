#pragma once

#include <api.h>
#include <event.h>


typedef struct Marble_Layer Marble_Layer;
typedef struct Marble_Layer {
	DWORD  dwLayerId;
	_Bool  blIsEnabled;
	void  *ptrUserdata;

	struct Marble_Layer_Callbacks {
		int (*onPush)(Marble_Layer *sSelf);
		int (*onPop)(Marble_Layer *sSelf);
		int (*onUpdate)(Marble_Layer *sSelf);
		int (*onEvent)(Marble_Layer *sSelf, Marble_Event *sEvent);
	} sCallbacks;
} Marble_Layer;


MARBLE_API int           Marble_Layer_Create(Marble_Layer **ptrpLayer, _Bool blIsEnabled, DWORD dwSizeOfUserdata);
MARBLE_API int           Marble_Layer_Push(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API Marble_Layer *Marble_Layer_Pop(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API void inline   Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled);


