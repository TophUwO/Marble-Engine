#pragma once

#include <api.h>
#include <event.h>


typedef struct Marble_Layer Marble_Layer;


MARBLE_API int           Marble_Layer_Create(Marble_Layer **ptrpLayer, _Bool blIsEnabled, DWORD dwSizeOfUserdata);
MARBLE_API int           Marble_Layer_Push(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API Marble_Layer *Marble_Layer_Pop(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API void inline   Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled);


