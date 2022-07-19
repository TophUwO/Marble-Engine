#include <entrypoint.h>


static Marble_Layer        *sGameLayer;
static Marble_Renderer     *sRenderer;
static Marble_Window       *sWindow;
static Marble_AssetManager *sAssetManager;
static Marble_LayerStack   *sLayerStack;
struct GameLayerUserdata {
	int unused;
} sUserdata;


static int GameLayer_OnPush(Marble_Layer *sLayer) {
	Marble_Asset *sAtlas = NULL;
	Marble_Asset_CreateAndLoadFromFileExplicit(
		Marble_AssetType_ColorTable,
		TEXT("..\\res\\mba_ctab1.mbasset"),
		NULL,
		&sAtlas
	);
	Marble_Asset_Register(sAssetManager, sAtlas);

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
	Marble_Application_GetRenderer(&sRenderer); 
	Marble_Application_GetMainWindow(&sWindow);
	Marble_Application_GetAssetManager(&sAssetManager);
	Marble_Application_GetLayerStack(&sLayerStack);

	Marble_Layer_Create(TRUE, &sGameLayer);
	Marble_Layer_SetUserdata(sGameLayer, &sUserdata);
	struct Marble_Layer_Callbacks sCallbacks = {
		.OnPush   = &GameLayer_OnPush,
		.OnPop    = &GameLayer_OnPop,
		.OnUpdate = &GameLayer_OnUpdate,
		.OnEvent  = &GameLayer_OnEvent
	};
	Marble_Layer_SetCallbacks(sGameLayer, &sCallbacks);
	Marble_Layer_Push(sLayerStack, sGameLayer, FALSE);

	Marble_Window_SetSize(sWindow, 32, 32, 32);

	return Marble_ErrorCode_Ok;
}


