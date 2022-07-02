#include <application.h>


static DWORD volatile gl_dwCurrentLayerId = 1;

static int __Marble_Layer_Internal_DummyDestroy__(void **ptrpUserdata) { return Marble_ErrorCode_Ok; }
static int __Marble_Layer_Internal_DummyOnPush__(Marble_Layer *sSelf) { return Marble_ErrorCode_Ok; }
static int __Marble_Layer_Internal_DummyOnPop__(Marble_Layer *sSelf) { return Marble_ErrorCode_Ok; }
static int __Marble_Layer_Internal_DummyOnUpdate__(Marble_Layer *sSelf) { return Marble_ErrorCode_Ok; }
static int __Marble_Layer_Internal_DummyOnEvent__(Marble_Layer *sSelf, Marble_Event *sEvent) { return Marble_ErrorCode_Ok; }

static void inline Marble_Layer_Internal_FixLayerCallbacks(Marble_Layer *sLayer) {
	sLayer->sCallbacks.onPush   = sLayer->sCallbacks.onPush   ? sLayer->sCallbacks.onPush   : &__Marble_Layer_Internal_DummyOnPush__;
	sLayer->sCallbacks.onPop    = sLayer->sCallbacks.onPop    ? sLayer->sCallbacks.onPop    : &__Marble_Layer_Internal_DummyOnPop__;
	sLayer->sCallbacks.onUpdate = sLayer->sCallbacks.onUpdate ? sLayer->sCallbacks.onUpdate : &__Marble_Layer_Internal_DummyOnUpdate__;
	sLayer->sCallbacks.onEvent  = sLayer->sCallbacks.onEvent  ? sLayer->sCallbacks.onEvent  : &__Marble_Layer_Internal_DummyOnEvent__;
}

static void Marble_LayerStack_Internal_DestroyLayer(Marble_Layer **ptrpLayer) {
	(*ptrpLayer)->sCallbacks.onPop(*ptrpLayer);
}


int Marble_LayerStack_Initialize(void) {
	int iErrorCode = Marble_ErrorCode_Ok;
	if (iErrorCode = Marble_Util_Vector_Create(&gl_sApplication.sLayers.sLayerStack, 32, (void (*)(void **))&Marble_LayerStack_Internal_DestroyLayer))
		return iErrorCode;

	gl_sApplication.sLayers.stLastLayer = 0;
	return Marble_ErrorCode_Ok;
}

void Marble_LayerStack_Destroy(void) {
	if (gl_sApplication.sLayers.sLayerStack)
		Marble_Util_Vector_Destroy(&gl_sApplication.sLayers.sLayerStack);
}


int Marble_Layer_Create(Marble_Layer **ptrpLayer, _Bool blIsEnabled, DWORD dwSizeOfUserdata) {
	if (*ptrpLayer = calloc(1, sizeof(**ptrpLayer))) {
		if (dwSizeOfUserdata) {
			if (!((*ptrpLayer)->ptrUserdata = calloc(1, dwSizeOfUserdata))) {
				free(*ptrpLayer);

				*ptrpLayer = NULL;
				return Marble_ErrorCode_MemoryAllocation;
			}
		}

		(*ptrpLayer)->dwLayerId   = gl_dwCurrentLayerId++;
		(*ptrpLayer)->blIsEnabled = blIsEnabled;

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_CreateLayer;
}

int Marble_Layer_Push(Marble_Layer *sLayer, _Bool blIsTopmost) {
	int iErrorCode = Marble_ErrorCode_Ok;

	Marble_Layer_Internal_FixLayerCallbacks(sLayer);
	if (blIsTopmost)
		iErrorCode = Marble_Util_Vector_PushBack(gl_sApplication.sLayers.sLayerStack, sLayer);
	else {
		iErrorCode = Marble_Util_Vector_Insert(
			gl_sApplication.sLayers.sLayerStack, 
			gl_sApplication.sLayers.stLastLayer, 
			sLayer
		);

		if (!iErrorCode)
			++gl_sApplication.sLayers.stLastLayer;
	}

	if (!iErrorCode)
		iErrorCode = sLayer->sCallbacks.onPush(sLayer);

	return iErrorCode;
}

Marble_Layer *Marble_Layer_Pop(Marble_Layer *sLayer, _Bool blIsTopmost) {
	size_t stIndex = Marble_Util_Vector_Find(
		gl_sApplication.sLayers.sLayerStack, 
		sLayer, 
		blIsTopmost 
			? gl_sApplication.sLayers.stLastLayer - 1
			: 0, 
		blIsTopmost
			? gl_sApplication.sLayers.stLastLayer
			: 0
	);

	if (stIndex ^ (size_t)(-1)) {
		Marble_Layer *sLayer = Marble_Util_Vector_Erase(
			gl_sApplication.sLayers.sLayerStack,
			stIndex,
			FALSE
		);
		sLayer->sCallbacks.onPop(sLayer);

		if (!blIsTopmost)
			--gl_sApplication.sLayers.stLastLayer;

		return sLayer;
	}

	return NULL;
}

void *Marble_Layer_GetUserdata(Marble_Layer *sLayer) {
	if (sLayer)
		return sLayer->ptrUserdata;

	return NULL;
}

void *Marble_Layer_GetHandler(Marble_Layer *sLayer, int iHandlerType) {
	if (sLayer) {
		switch (iHandlerType) {
			case Marble_LayerHandlerType_OnPush:   return sLayer->sCallbacks.onPush;
			case Marble_LayerHandlerType_OnPop:    return sLayer->sCallbacks.onPop;
			case Marble_LayerHandlerType_OnUpdate: return sLayer->sCallbacks.onUpdate;
			case Marble_LayerHandlerType_OnEvent:  return sLayer->sCallbacks.onEvent;
		}
	}

	return NULL;
}

void Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled) {
	if (sLayer)
		sLayer->blIsEnabled = blIsEnabled;
}

void *Marble_Layer_SetHandler(Marble_Layer *sLayer, int iHandlerType, void *fnptrHandler) {
	if (sLayer) {
		void *fnptrRet = NULL;

		switch (iHandlerType) {
			case Marble_LayerHandlerType_OnPush:   fnptrRet = sLayer->sCallbacks.onPush; sLayer->sCallbacks.onPush = fnptrHandler; break;
			case Marble_LayerHandlerType_OnPop:    fnptrRet = sLayer->sCallbacks.onPop; sLayer->sCallbacks.onPop = fnptrHandler; break;
			case Marble_LayerHandlerType_OnUpdate: fnptrRet = sLayer->sCallbacks.onUpdate; sLayer->sCallbacks.onUpdate = fnptrHandler; break;
			case Marble_LayerHandlerType_OnEvent:  fnptrRet = sLayer->sCallbacks.onEvent; sLayer->sCallbacks.onEvent = fnptrHandler; break;
		}

		return fnptrRet;
	}

	return NULL;
}

void *Marble_Layer_SetUserdata(Marble_Layer *sLayer, void *ptrUserdata) {
	if (sLayer) {
		void *ptrOldUserdata = sLayer->ptrUserdata;

		sLayer->ptrUserdata = ptrUserdata;
		return ptrOldUserdata;
	}
}


