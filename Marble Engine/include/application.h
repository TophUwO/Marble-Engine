#pragma once

#include <layer.h>
#include <util.h>
#include <window.h>
#include <renderer.h>
#include <asset.h>
#include <internal.h>


enum Marble_Internal_AppState {
	Marble_AppState_Init,
	Marble_AppState_Running,
	Marble_AppState_Shutdown,
	Marble_AppState_ForcedShutdown
};


extern struct Marble_Application {
	HINSTANCE     hiInstance;
	PSTR          astrCommandLine;
	int           iAppState;
	LARGE_INTEGER uPerfFreq;

	Marble_Window       *sMainWindow;
	Marble_Renderer     *sRenderer;
	Marble_AssetManager *sAssets;
	struct Marble_LayerStack {
		Marble_Util_Vector *sLayerStack;

		size_t stLastLayer;
	} sLayers;
} gl_sApplication;


