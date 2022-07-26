#pragma once

#include <event.h>


typedef enum Marble_AssetTypes {
	Marble_AssetType_Unknown = 0,

	/* Magic numbers for debug purposes only */
	Marble_AssetType_ColorTable = 2,
	Marble_AssetType_Map = 4,

	__MARBLE_NUMASSETTYPES__
} Marble_AssetType;


typedef struct Marble_Layer        Marble_Layer;
typedef struct Marble_Asset        Marble_Asset;
typedef struct Marble_Renderer     Marble_Renderer;
typedef struct Marble_Window       Marble_Window;
typedef struct Marble_LayerStack   Marble_LayerStack;


struct Marble_Layer_Callbacks {
	int (*OnPush)(Marble_Layer *sSelf);
	int (*OnPop)(Marble_Layer *sSelf);
	int (*OnUpdate)(Marble_Layer *sSelf, float fFrameTime);
	int (*OnEvent)(Marble_Layer *sSelf, Marble_Event *sEvent);
};
typedef struct Marble_MapAsset_CreateParams { 
	DWORD dwWidth; 
	DWORD dwHeight;
	DWORD dwNumOfLayers;
	int   iTileFormat;
} Marble_MapAsset_CreateParams;


MARBLE_API struct Marble_UserAPI {
	struct Renderer {
		void (*const Clear)(Marble_Renderer *sRenderer, float fRed, float fGreen, float fBlue, float fAlpha);
	} Renderer;

	struct Window {
		void (*const setSize)(Marble_Window *sWindow, int iWidthInTiles, int iHeightInTiles, int iTileSize);
		void (*const setFullscreen)(Marble_Window *sWindow, _Bool blIsEnabled);
		void (*const setVSync)(Marble_Window *sWindow, _Bool blIsEnabled);
	} Window;

	struct Layer {
		int           (*const Create)(_Bool blIsEnabled, Marble_Layer **ptrpLayer);
		int           (*const CreateAndPush)(_Bool blIsEnabled, struct Marble_Layer_Callbacks const *sCallbacks, void *ptrUserdata, Marble_Layer **ptrpLayer, Marble_LayerStack *sLayerStack, _Bool blIsTopmost);
		void          (*const Destroy)(Marble_Layer **ptrpLayer);
		int           (*const Push)(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost);
		Marble_Layer *(*const Pop)(Marble_LayerStack *sLayerStack, Marble_Layer *sLayer, _Bool blIsTopmost);
		void         *(*const getUserdata)(Marble_Layer *sLayer);
		void         *(*const getCallback)(Marble_Layer *sLayer, int iHandlerType);
		_Bool         (*const isEnabled)(Marble_Layer *sLayer);
		void          (*const setEnabled)(Marble_Layer *sLayer, _Bool blIsEnabled);
		void          (*const setCallbacks)(Marble_Layer *sLayer, struct Marble_Layer_Callbacks const *sCallbacks);
		void         *(*const setUserdata)(Marble_Layer *sLayer, void *ptrUserdata);
	} Layer;

	struct Asset {
		int   (*const Create)(int iAssetType, CHAR const *strAssetID, void const *ptrCreateParams, Marble_Asset **ptrpAsset);
		int   (*const LoadFromFile)(TCHAR const *strPath, Marble_Asset **ptrpAsset);
		void  (*const Destroy)(Marble_Asset **ptrpAsset);
		int   (*const getType)(Marble_Asset *sAsset);
		CHAR *(*const getID)(Marble_Asset *sAsset);
		int   (*const Register)(Marble_Asset *sAsset);
		int   (*const Unregister)(Marble_Asset *sAsset, _Bool blDoFree);
		int   (*const Obtain)(Marble_Asset *sAsset);
		int   (*const Release)(Marble_Asset *sAsset);
		int   (*const Render)(Marble_Asset *sAsset, Marble_Renderer *sRenderer);
	} Asset;
} Marble;


