#include <entrypoint.h>


static Marble_Layer *sGameLayer;
struct GameLayerUserdata {
	Marble_Asset *sMap1;
	Marble_Asset *sCT1;
} sUserdata;


static int GameLayer_OnPush(Marble_Layer *sLayer) {
	Marble.Asset.LoadFromFile(
		TEXT("..\\res\\map1.mba"),
		&sUserdata.sMap1
	);
	Marble.Asset.Register(sUserdata.sMap1);

	return 0;
}

static int GameLayer_OnPop(Marble_Layer *sLayer) {

	return 0;
}

static int GameLayer_OnUpdate(Marble_Layer *sLayer, float fFrameTime) {
	Marble.Asset.Render(sUserdata.sMap1, Marble_DefRenderer);

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


