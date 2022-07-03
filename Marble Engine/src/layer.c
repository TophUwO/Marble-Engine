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


int Marble_Layer_Create(Marble_Layer **ptrpLayer, _Bool blIsEnabled) {
	if (*ptrpLayer = calloc(1, sizeof(**ptrpLayer))) {
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
		iErrorCode = sLayer->sCallbacks.OnPush(sLayer);

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
		sLayer->sCallbacks.OnPop(sLayer);

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


