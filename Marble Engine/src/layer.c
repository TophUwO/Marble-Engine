#include <application.h>


static DWORD volatile gl_dwCurrentLayerId = 1;

static int __Marble_Layer_Internal_DummyDestroy__(void **ptrpUserdata) { return Marble_ErrorCode_Ok; }
static int __Marble_Layer_Internal_DummyOnPush__(Marble_Layer *sSelf) { return Marble_ErrorCode_Ok; }
static int __Marble_Layer_Internal_DummyOnPop__(Marble_Layer *sSelf) { return Marble_ErrorCode_Ok; }
static int __Marble_Layer_Internal_DummyOnUpdate__(Marble_Layer *sSelf, float fFrameTime) { return Marble_ErrorCode_Ok; }
static int __Marble_Layer_Internal_DummyOnEvent__(Marble_Layer *sSelf, Marble_Event *sEvent) { return Marble_ErrorCode_Ok; }

static void inline Marble_Layer_Internal_FixLayerCallbacks(Marble_Layer *sLayer) {
	sLayer->sCallbacks.OnPush   = sLayer->sCallbacks.OnPush   ? sLayer->sCallbacks.OnPush   : &__Marble_Layer_Internal_DummyOnPush__;
	sLayer->sCallbacks.OnPop    = sLayer->sCallbacks.OnPop    ? sLayer->sCallbacks.OnPop    : &__Marble_Layer_Internal_DummyOnPop__;
	sLayer->sCallbacks.OnUpdate = sLayer->sCallbacks.OnUpdate ? sLayer->sCallbacks.OnUpdate : &__Marble_Layer_Internal_DummyOnUpdate__;
	sLayer->sCallbacks.OnEvent  = sLayer->sCallbacks.OnEvent  ? sLayer->sCallbacks.OnEvent  : &__Marble_Layer_Internal_DummyOnEvent__;
}


void Marble_LayerStack_Destroy(Marble_LayerStack **ptrpLayerstack) {
	if (!ptrpLayerstack)
		return;

	Marble_Util_Vector_Destroy(&(*ptrpLayerstack)->sLayerStack);

	free(*ptrpLayerstack);
	*ptrpLayerstack = NULL;
}

int Marble_LayerStack_Create(Marble_LayerStack **ptrpLayerstack) { MARBLE_ERRNO
	if (!ptrpLayerstack)
		return Marble_ErrorCode_Parameter;

	MB_IFNOK_RET_CODE(Marble_System_AllocateMemory(
		ptrpLayerstack, 
		sizeof **ptrpLayerstack, 
		FALSE, 
		TRUE
	));

	MB_IFNOK_DO_BODY(Marble_Util_Vector_Create(
		Marble_Util_VectorType_VecOfPointers, 
		0, 
		32, 
		&Marble_Layer_Destroy, 
		NULL, 
		&(*ptrpLayerstack)->sLayerStack
	), { Marble_LayerStack_Destroy(ptrpLayerstack); return iErrorCode; });

	(*ptrpLayerstack)->stLastLayer = 0;
	return Marble_ErrorCode_Ok;
}


int Marble_Layer_Create(_Bool blIsEnabled, Marble_Layer **ptrpLayer) { MARBLE_ERRNO
	if (!ptrpLayer)
		return Marble_ErrorCode_Parameter;

	MB_IFNOK_RET_CODE(Marble_System_AllocateMemory(
		ptrpLayer, 
		sizeof **ptrpLayer, 
		FALSE, 
		TRUE
	));

	(*ptrpLayer)->dwLayerId   = InterlockedIncrement(&gl_dwCurrentLayerId);
	(*ptrpLayer)->blIsEnabled = blIsEnabled;

	return Marble_ErrorCode_Ok;
}

int Marble_Layer_CreateAndPush(_Bool blIsEnabled, struct Marble_Layer_Callbacks const *sCallbacks, void *ptrUserdata, Marble_Layer **ptrpLayer, Marble_LayerStack *sLayerStack, _Bool blIsTopmost) { MARBLE_ERRNO
	MB_IFNOK_RET_CODE(Marble_Layer_Create(blIsEnabled, ptrpLayer));

	Marble_Layer_SetCallbacks(*ptrpLayer, sCallbacks);
	Marble_Layer_SetUserdata(*ptrpLayer, ptrUserdata);
	MB_IFNOK_DO_BODY(Marble_Layer_Push(sLayerStack, *ptrpLayer, blIsTopmost), {
		Marble_Layer_Destroy(ptrpLayer);

		return iErrorCode;
	});

	return Marble_ErrorCode_Ok;
}

void Marble_Layer_Destroy(Marble_Layer **ptrpLayer) {
	if (!ptrpLayer || !*ptrpLayer)
		return;

	Marble_Layer_Pop((*ptrpLayer)->sRefLayerstack, *ptrpLayer, (*ptrpLayer)->blIsTopmost);

	free(*ptrpLayer);
	*ptrpLayer = NULL;
}

int Marble_Layer_Push(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost) { MARBLE_ERRNO
	if (!sLayerStack || !sLayer)
		return Marble_ErrorCode_Parameter;
	sLayerStack = sLayerStack == Marble_DefLayerStack ? gl_sApplication.sLayers : sLayerStack;

	Marble_Layer_Internal_FixLayerCallbacks(sLayer);
	if (blIsTopmost)
		iErrorCode = Marble_Util_Vector_PushBack(sLayerStack->sLayerStack, sLayer);
	else {
		iErrorCode = Marble_Util_Vector_Insert(
			sLayerStack->sLayerStack, 
			sLayerStack->stLastLayer, 
			sLayer
		);

		if (!iErrorCode)
			++sLayerStack->stLastLayer;
	}

	if (!iErrorCode) {
		MB_IFNOK_RET_CODE(sLayer->sCallbacks.OnPush(sLayer));

		sLayer->blIsTopmost    = blIsTopmost;
		sLayer->sRefLayerstack = sLayerStack;
	}

	return iErrorCode;
}

Marble_Layer *Marble_Layer_Pop(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost) {
	if (!sLayerStack || !sLayer)
		return NULL;

	size_t stIndex = Marble_Util_Vector_Find(
		sLayerStack->sLayerStack, 
		sLayer, 
		blIsTopmost 
			? sLayerStack->stLastLayer - 1
			: 0, 
		blIsTopmost
			? sLayerStack->stLastLayer
			: 0
	);

	if (stIndex ^ (size_t)(-1)) {
		Marble_Layer *sLayer = Marble_Util_Vector_Erase(
			sLayerStack->sLayerStack,
			stIndex,
			FALSE
		);
		sLayer->sCallbacks.OnPop(sLayer);

		sLayer->blIsTopmost    = FALSE;
		sLayer->sRefLayerstack = NULL;

		if (!blIsTopmost)
			--sLayerStack->stLastLayer;

		return sLayer;
	}

	return NULL;
}

void *Marble_Layer_GetUserdata(Marble_Layer *sLayer) {
	if (sLayer)
		return sLayer->ptrUserdata;

	return NULL;
}

void *Marble_Layer_GetCallback(Marble_Layer *sLayer, int iHandlerType) {
	if (!sLayer)
		return NULL;

	switch (iHandlerType) {
		case Marble_LayerHandlerType_OnPush:   return sLayer->sCallbacks.OnPush;
		case Marble_LayerHandlerType_OnPop:    return sLayer->sCallbacks.OnPop;
		case Marble_LayerHandlerType_OnUpdate: return sLayer->sCallbacks.OnUpdate;
		case Marble_LayerHandlerType_OnEvent:  return sLayer->sCallbacks.OnEvent;
	}

	return NULL;
}

_Bool Marble_Layer_IsEnabled(Marble_Layer *sLayer) {
	return sLayer ? sLayer->blIsEnabled : FALSE;
}

void Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled) {
	if (sLayer)
		sLayer->blIsEnabled = blIsEnabled;
}

void Marble_Layer_SetCallbacks(Marble_Layer *sLayer, struct Marble_Layer_Callbacks const *sCallbacks) {
	if (!sLayer)
		return;

	memcpy(&sLayer->sCallbacks, sCallbacks, sizeof(struct Marble_Layer_Callbacks));

	Marble_Layer_Internal_FixLayerCallbacks(sLayer);
}

void *Marble_Layer_SetUserdata(Marble_Layer *sLayer, void *ptrUserdata) {
	if (!sLayer)
		return NULL;

	void *ptrOldUserdata = sLayer->ptrUserdata;

	sLayer->ptrUserdata = ptrUserdata;
	return ptrOldUserdata;
}


