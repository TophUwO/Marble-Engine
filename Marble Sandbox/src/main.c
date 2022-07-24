#include <entrypoint.h>


static Marble_Layer *sGameLayer;
struct GameLayerUserdata {
	int unused;
} sUserdata;


static int GameLayer_OnPush(Marble_Layer *sLayer) {
	Marble_Asset *sMap1 = NULL;
	Marble_MapAsset_CreateParams sParams = { "map1", 24, 24, 1 };
	int i = Marble.Asset.CreateAndLoadFromFileExplicit(
		Marble_AssetType_Map,
		TEXT("..\\res\\map1.mba"),
		&sParams,
		&sMap1
	);
	Marble.Asset.Register(sMap1);

	return 0;
}

static int GameLayer_OnPop(Marble_Layer *sLayer) {

	return 0;
}

static int GameLayer_OnUpdate(Marble_Layer *sLayer, float fFrameTime) {

	return 0;
}

static int GameLayer_OnEvent(Marble_Layer *sLayer, Marble_Event *sEvent) {

	return 0;
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
	return 0;
}


