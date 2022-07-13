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

static void Marble_LayerStack_Internal_DestroyLayer(Marble_Layer **ptrpLayer) {
	(*ptrpLayer)->sCallbacks.OnPop(*ptrpLayer);

	free(*ptrpLayer);
	*ptrpLayer = NULL;
}


int Marble_LayerStack_Create(Marble_LayerStack **ptrpLayerstack) {
	if (ptrpLayerstack) {
		int iErrorCode = Marble_ErrorCode_Ok;
		if (iErrorCode = Marble_System_AllocateMemory(ptrpLayerstack, sizeof(**ptrpLayerstack), FALSE, TRUE)) {
			*ptrpLayerstack = NULL;

			return iErrorCode;
		}

		if (iErrorCode = Marble_Util_Vector_Create(&(*ptrpLayerstack)->sLayerStack, 32, (void (*)(void **))&Marble_LayerStack_Internal_DestroyLayer)) {
			free(*ptrpLayerstack);
			*ptrpLayerstack = NULL;

			return iErrorCode;
		}
		(*ptrpLayerstack)->stLastLayer = 0;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_Parameter;
}

void Marble_LayerStack_Destroy(Marble_LayerStack **ptrpLayerstack) {
	if (ptrpLayerstack && *ptrpLayerstack) {
		Marble_Util_Vector_Destroy(&(*ptrpLayerstack)->sLayerStack);

		free(*ptrpLayerstack);
		*ptrpLayerstack = NULL;
	}
}


int Marble_Layer_Create(Marble_Layer **ptrpLayer, _Bool blIsEnabled) { MARBLE_ERRNO
	if (ptrpLayer) {
		if (iErrorCode = Marble_System_AllocateMemory(ptrpLayer, sizeof(**ptrpLayer), FALSE, TRUE))
			return iErrorCode;

		(*ptrpLayer)->dwLayerId   = InterlockedIncrement(&gl_dwCurrentLayerId);
		(*ptrpLayer)->blIsEnabled = blIsEnabled;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_Parameter;
}

int Marble_Layer_Push(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost) { MARBLE_ERRNO
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

	if (!iErrorCode)
		iErrorCode = sLayer->sCallbacks.OnPush(sLayer);

	return iErrorCode;
}

Marble_Layer *Marble_Layer_Pop(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost) {
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
	if (sLayer) {
		switch (iHandlerType) {
			case Marble_LayerHandlerType_OnPush:   return sLayer->sCallbacks.OnPush;
			case Marble_LayerHandlerType_OnPop:    return sLayer->sCallbacks.OnPop;
			case Marble_LayerHandlerType_OnUpdate: return sLayer->sCallbacks.OnUpdate;
			case Marble_LayerHandlerType_OnEvent:  return sLayer->sCallbacks.OnEvent;
		}

		return NULL;
	}

	return NULL;
}

void Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled) {
	if (sLayer)
		sLayer->blIsEnabled = blIsEnabled;
}

void Marble_Layer_SetCallbacks(Marble_Layer *sLayer, struct Marble_Layer_Callbacks const *sCallbacks) {
	if (sLayer) {
		memcpy(&sLayer->sCallbacks, sCallbacks, sizeof(struct Marble_Layer_Callbacks));

		Marble_Layer_Internal_FixLayerCallbacks(sLayer);
	}
}

void *Marble_Layer_SetUserdata(Marble_Layer *sLayer, void *ptrUserdata) {
	if (sLayer) {
		void *ptrOldUserdata = sLayer->ptrUserdata;

		sLayer->ptrUserdata = ptrUserdata;
		return ptrOldUserdata;
	}

	return NULL;
}


