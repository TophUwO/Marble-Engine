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


MARBLE_API struct Marble_UserAPI {
	struct {
		void (*const Clear)(Marble_Renderer *sRenderer, float fRed, float fGreen, float fBlue, float fAlpha);
	} Renderer;

	struct {
		void (*const setSize)(Marble_Window *sWindow, int iWidthInTiles, int iHeightInTiles, int iTileSize);
		void (*const setFullscreen)(Marble_Window *sWindow, _Bool blIsEnabled);
		void (*const setVSync)(Marble_Window *sWindow, _Bool blIsEnabled);
	} Window;

	struct {
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

	struct {
		int   (*const Create)(Marble_Asset **ptrpAsset);
		int   (*const CreateExplicit)(int iAssetType, void const *ptrCreateParams, Marble_Asset **ptrpAsset);
		int   (*const CreateAndLoadFromFile)(TCHAR const *strPath, Marble_Asset **ptrpAsset);
		int   (*const CreateAndLoadFromFileExplicit)(int iAssetType, TCHAR const *strPath, void const *ptrCreateParams, Marble_Asset **ptrpAsset);
		int   (*const LoadFromFile)(Marble_Asset *sAsset, TCHAR const *strPath);
		void  (*const Destroy)(Marble_Asset **ptrpAsset);
		int   (*const getType)(Marble_Asset *sAsset);
		CHAR *(*const getId)(Marble_Asset *sAsset);
		int   (*const Register)(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset);
		int   (*const Unregister)(Marble_AssetManager *sAssetManager, Marble_Asset *sAsset, _Bool blDoFree);
	} Asset;
} Marble;


