#pragma once

#include <marble.h>


typedef enum Marble_LayerHandlerTypes {
	Marble_LayerHandlerType_Unknown,

	Marble_LayerHandlerType_OnPush,
	Marble_LayerHandlerType_OnPop,
	Marble_LayerHandlerType_OnUpdate,
	Marble_LayerHandlerType_OnEvent
} Marble_LayerHandlerTypes;


typedef struct Marble_LayerStack {
	Marble_Util_Vector *sLayerStack;

	size_t stLastLayer;
} Marble_LayerStack;

typedef struct Marble_Layer {
	DWORD  dwLayerId;
	_Bool  blIsEnabled;
	_Bool  blIsTopmost;
	void  *ptrUserdata;

	Marble_LayerStack *sRefLayerstack;
	struct Marble_Layer_Callbacks sCallbacks;
} Marble_Layer;


extern int           Marble_Layer_Create(_Bool blIsEnabled, Marble_Layer **ptrpLayer);
extern int           Marble_Layer_CreateAndPush(_Bool blIsEnabled, struct Marble_Layer_Callbacks const *sCallbacks, void *ptrUserdata, Marble_Layer **ptrpLayer, Marble_LayerStack *sLayerStack, _Bool blIsTopmost);
extern void          Marble_Layer_Destroy(Marble_Layer **ptrpLayer);
extern int           Marble_Layer_Push(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost);
extern Marble_Layer *Marble_Layer_Pop(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost);
extern void inline  *Marble_Layer_GetUserdata(Marble_Layer *sLayer);
extern void inline  *Marble_Layer_GetCallback(Marble_Layer *sLayer, int iHandlerType);
extern _Bool inline  Marble_Layer_IsEnabled(Marble_Layer *sLayer);
extern void inline   Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled);
extern void inline   Marble_Layer_SetCallbacks(Marble_Layer *sLayer, struct Marble_Layer_Callbacks const *sCallbacks);
extern void inline  *Marble_Layer_SetUserdata(Marble_Layer *sLayer, void *ptrUserdata);


