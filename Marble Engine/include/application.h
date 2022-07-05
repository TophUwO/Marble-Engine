#pragma once

#include <layer.h>
#include <util.h>
#include <window.h>
#include <renderer.h>
#include <asset.h>
#include <atlas.h>
#include <color.h>

#define Marble_IfError(expr, equal, body) if ((expr) != equal) { body; }


enum Marble_Internal_AppState {
	Marble_AppState_Init,
	Marble_AppState_Running,
	Marble_AppState_Shutdown,
	Marble_AppState_ForcedShutdown
};


typedef struct Marble_AssetManager {
	Marble_Util_Vector *sAtlases;

	IWICImagingFactory *sWICFactory;
} Marble_AssetManager;


extern struct Marble_Application {
	HINSTANCE     hiInstance;
	PSTR          astrCommandLine;
	int           iAppState;
	LARGE_INTEGER uPerfFreq;
	LARGE_INTEGER uFTLast;

	Marble_Window       *sMainWindow;
	Marble_Renderer     *sRenderer;
	Marble_AssetManager *sAssets;
	struct Marble_LayerStack {
		Marble_Util_Vector *sLayerStack;

		size_t stLastLayer;
	} sLayers;
} gl_sApplication;

extern ULONGLONG volatile Marble_AssetManager_RequestAssetId(void);

extern int                Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData);
extern TCHAR const *const Marble_Event_GetEventTypeName(Marble_EventType eEventType);
extern Marble_EventType   Marble_Event_GetMouseEventType(UINT udwMessage);


