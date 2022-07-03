#pragma once

#include <marble.h>


enum Marble_LayerHandlerType {
	Marble_LayerHandlerType_Unknown,

	Marble_LayerHandlerType_OnPush,
	Marble_LayerHandlerType_OnPop,
	Marble_LayerHandlerType_OnUpdate,
	Marble_LayerHandlerType_OnEvent
};


struct Marble_Layer {
	DWORD  dwLayerId;
	_Bool  blIsEnabled;
	void  *ptrUserdata;

	struct Marble_Layer_Callbacks sCallbacks;
};


