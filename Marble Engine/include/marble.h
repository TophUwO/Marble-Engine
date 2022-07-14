#pragma once

#include <error.h>
#include <event.h>
#include <util.h>


enum Marble_AssetType {
	Marble_AssetType_Unknown,

	Marble_AssetType_Atlas,
	Marble_AssetType_ColorTable,
	Marble_AssetType_Image
};

enum Marble_AtlasType {
	Marble_AtlasType_Unknown,

	Marble_AtlasType_ColorAtlas
};


typedef struct Marble_Layer        Marble_Layer;
typedef struct Marble_Asset        Marble_Asset;
typedef struct Marble_Atlas        Marble_Atlas;
typedef struct Marble_Renderer     Marble_Renderer;
typedef struct Marble_Window       Marble_Window;
typedef struct Marble_AssetManager Marble_AssetManager;
typedef struct Marble_LayerStack   Marble_LayerStack;


struct Marble_Layer_Callbacks {
	int (*OnPush)(Marble_Layer *sSelf);
	int (*OnPop)(Marble_Layer *sSelf);
	int (*OnUpdate)(Marble_Layer *sSelf, float fFrameTime);
	int (*OnEvent)(Marble_Layer *sSelf, Marble_Event *sEvent);
};
struct Marble_Atlas_CreateParams { int iAtlasType; };


MARBLE_API void Marble_Renderer_Clear(Marble_Renderer *sRenderer, float fRed, float fGreen, float fBlue, float fAlpha);

MARBLE_API int  Marble_Application_GetRenderer(Marble_Renderer **ptrpRenderer);
MARBLE_API int  Marble_Application_GetMainWindow(Marble_Window **ptrpWindow);
MARBLE_API int  Marble_Application_GetAssetManager(Marble_AssetManager **ptrpAssetManager);
MARBLE_API int  Marble_Application_GetLayerStack(Marble_LayerStack **ptrpLayerStack);

MARBLE_API void Marble_Window_SetSize(Marble_Window *sWindow, int iWidthInTiles, int iHeightInTiles, int iTileSize);
MARBLE_API void Marble_Window_SetFullscreen(Marble_Window *sWindow, _Bool blIsEnabled);
MARBLE_API void Marble_Window_SetVSyncEnabled(Marble_Window *sWindow, _Bool blIsEnabled);

MARBLE_API int inline Marble_ColorTable_GetColorByIndex(Marble_Asset *sColorTable, size_t stIndex, void *ptrColor);

MARBLE_API int           Marble_Layer_Create(_Bool blIsEnabled, Marble_Layer **ptrpLayer);
MARBLE_API int           Marble_Layer_Push(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API Marble_Layer *Marble_Layer_Pop(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API void inline  *Marble_Layer_GetUserdata(Marble_Layer *sLayer);
MARBLE_API void inline  *Marble_Layer_GetCallback(Marble_Layer *sLayer, int iHandlerType);
MARBLE_API void inline   Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled);
MARBLE_API void inline   Marble_Layer_SetCallbacks(Marble_Layer *sLayer, struct Marble_Layer_Callbacks const *sCallbacks);
MARBLE_API void inline  *Marble_Layer_SetUserdata(Marble_Layer *sLayer, void *ptrUserdata);

MARBLE_API int  Marble_Asset_Create(int iAssetType, void const *ptrCreateParams, Marble_Asset **ptrpAsset);
MARBLE_API int  Marble_Asset_LoadFromFile(Marble_Asset *sAsset, TCHAR const *strPath);
MARBLE_API int  Marble_Asset_CreateAndLoadFromFile(int iAssetType, TCHAR const *strPath, void const *ptrCreateParams, Marble_Asset **ptrpAsset);
MARBLE_API void Marble_Asset_Destroy(Marble_Asset **ptrpAsset);
MARBLE_API int  Marble_Asset_GetType(Marble_Asset *sAsset);
MARBLE_API int  Marble_Asset_Register(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset);
MARBLE_API int  Marble_Asset_Unregister(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset, _Bool blDoFree);


