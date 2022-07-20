#include <entrypoint.h>


static Marble_Layer *sGameLayer;
struct GameLayerUserdata {
	int unused;
} sUserdata;


static int GameLayer_OnPush(Marble_Layer *sLayer) {
	Marble_Asset *sAtlas = NULL;
	Marble.Asset.CreateAndLoadFromFileExplicit(
		Marble_AssetType_ColorTable,
		TEXT("..\\res\\mba_ctab1.mbasset"),
		NULL,
		&sAtlas
	);
	Marble.Asset.Register(Marble_DefAssetMan, sAtlas);

	return Marble_ErrorCode_Ok;
}

static int GameLayer_OnPop(Marble_Layer *sLayer) {

	return Marble_ErrorCode_Ok;
}

static int GameLayer_OnUpdate(Marble_Layer *sLayer, float fFrameTime) {

	return Marble_ErrorCode_Ok;
}

static int GameLayer_OnEvent(Marble_Layer *sLayer, Marble_Event *sEvent) {

	return Marble_ErrorCode_Ok;
}


int Marble_Application_OnUserInit(void) {
	struct Marble_Layer_Callbacks sCallbacks = {
		.OnPush   = &GameLayer_OnPush,
		.OnPop    = &GameLayer_OnPop,
		.OnUpdate = &GameLayer_OnUpdate,
		.OnEvent  = &GameLayer_OnEvent
	};
	Marble.Layer.CreateAndPush(TRUE, &sCallbacks, &sUserdata, &sGameLayer, Marble_DefLayerStack, FALSE);

	Marble.Window.setSize(Marble_DefWindow, 32, 32, 32);
	return Marble_ErrorCode_Ok;
}


