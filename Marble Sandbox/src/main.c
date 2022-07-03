#include <entrypoint.h>


Marble_Layer    *sGameLayer;
Marble_Renderer *sRenderer;
struct GameLayerUserdata {
	int unused;
} sUserdata;


static int GameLayer_OnPush(Marble_Layer *sLayer) {
	struct Marble_Atlas_CreateParams const sAtlasCP = {
		.iAtlasType = Marble_AtlasType_ColorAtlas
	};
	Marble_Asset *sAtlas = NULL;

	Marble_Asset_Create(Marble_AssetType_Atlas, &sAtlas, &sAtlasCP);
	Marble_ColorAtlas_LoadFromFile((Marble_ColorAtlas *)sAtlas, TEXT("..\\res\\test.mbasset"));
	Marble_AssetManager_RegisterAsset(sAtlas);

	return Marble_ErrorCode_Ok;
}

static int GameLayer_OnPop(Marble_Layer *sLayer) {

	return Marble_ErrorCode_Ok;
}

static int GameLayer_OnUpdate(Marble_Layer *sLayer, float fFrameTime) {
	Marble_Renderer_Clear(sRenderer, 1.0f, 0.0f, 1.0f, 1.0f);

	return Marble_ErrorCode_Ok;
}

static int GameLayer_OnEvent(Marble_Layer *sLayer, Marble_Event *sEvent) {

	return Marble_ErrorCode_Ok;
}


int Marble_System_OnUserInit(void) {
	Marble_Layer_Create(&sGameLayer, TRUE);
	Marble_Layer_SetUserdata(sGameLayer, &sUserdata);
	struct Marble_Layer_Callbacks sCallbacks = {
		.OnPush   = &GameLayer_OnPush,
		.OnPop    = &GameLayer_OnPop,
		.OnUpdate = &GameLayer_OnUpdate,
		.OnEvent  = &GameLayer_OnEvent
	};
	Marble_Layer_SetCallbacks(sGameLayer, &sCallbacks);
	Marble_Layer_Push(sGameLayer, FALSE);

	Marble_Application_GetRenderer(&sRenderer);

	return Marble_ErrorCode_Ok;
}


