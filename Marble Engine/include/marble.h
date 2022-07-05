#pragma once

#include <error.h>
#include <event.h>
#include <util.h>


enum Marble_AssetType {
	Marble_AssetType_Unknown,

	Marble_AssetType_Atlas,
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
typedef struct Marble_ColorAtlas   Marble_ColorAtlas;
typedef struct Marble_AssetManager Marble_AssetManager;


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

/// <summary>
/// Resizes window to the given width and height. If the dimensions are inadequate (e.g. too large 
/// for current monitor), the functions calculates adequate dimensions that at least closely fit
/// the original aspect ratio of the given dimensions.
/// </summary>
/// <param name="sWindow"> window handle previously obtained by Marble_Application_GetMainWindow() </param>
/// <param name="iWidthInTiles"> width in tiles (width_in_pixels = iWidthInTiles * iTileSize) </param>
/// <param name="iHeightInTiles">height in tiles (height_in_pixels = iHeightInTiles * iTileSize) </param>
/// <param name="iTileSize">tile size in pixels</param>
/// <returns>nothing</returns>
MARBLE_API void Marble_Window_SetSize(Marble_Window *sWindow, int iWidthInTiles, int iHeightInTiles, int iTileSize);
MARBLE_API void Marble_Window_SetFullscreen(Marble_Window *sWindow, _Bool blIsEnabled);
MARBLE_API void Marble_Window_SetVSyncEnabled(Marble_Window *sWindow, _Bool blIsEnabled);

MARBLE_API int        Marble_ColorAtlas_LoadFromFile(Marble_ColorAtlas *sAtlas, TCHAR const *strPath);
MARBLE_API int inline Marble_ColorAtlas_GetColorByIndex(Marble_ColorAtlas *sAtlas, size_t stIndex, void *ptrColor);

MARBLE_API int        Marble_Image_LoadFromFile(Marble_Asset *sImage, TCHAR const *strPath);

MARBLE_API int           Marble_Layer_Create(Marble_Layer **ptrpLayer, _Bool blIsEnabled);
MARBLE_API int           Marble_Layer_Push(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API Marble_Layer *Marble_Layer_Pop(Marble_Layer *sLayer, _Bool blIsTopmost);
MARBLE_API void inline  *Marble_Layer_GetUserdata(Marble_Layer *sLayer);
MARBLE_API void inline  *Marble_Layer_GetCallback(Marble_Layer *sLayer, int iHandlerType);
MARBLE_API void inline   Marble_Layer_SetEnabled(Marble_Layer *sLayer, _Bool blIsEnabled);
MARBLE_API void inline   Marble_Layer_SetCallbacks(Marble_Layer *sLayer, struct Marble_Layer_Callbacks const *sCallbacks);
MARBLE_API void inline  *Marble_Layer_SetUserdata(Marble_Layer *sLayer, void *ptrUserdata);

MARBLE_API int  Marble_Asset_Create(int iAssetType, Marble_Asset **ptrpAsset, void const *ptrCreateParams);
MARBLE_API void Marble_Asset_Destroy(Marble_Asset **ptrpAsset);
MARBLE_API int  Marble_Asset_GetType(Marble_Asset *sAsset);

MARBLE_API int  Marble_AssetManager_RegisterAsset(Marble_Asset *sAsset);
MARBLE_API int  Marble_AssetManager_UnregisterAsset(Marble_Asset *sAsset, _Bool blDoFree);


