#pragma once

#include <d2dwr.h>
#include <dxgi1_2.h>
#include <d3d11.h>

#include <util.h>
#include <layer.h>


#define Marble_IfError(expr, equal, body) if ((expr) != equal) { body; }


enum Marble_RendererAPI {
	Marble_RendererAPI_Direct2D = 1
};


typedef struct Marble_Util_Vector {
	void   **ptrpData;
	size_t   stSize;
	size_t   stCapacity;
	size_t   stStartCapacity;

	void (*onDestroy)(void **ptrObject);
} Marble_Util_Vector;

typedef struct Marble_Util_Clock {
	LARGE_INTEGER uStartTime;
	LARGE_INTEGER uStopTime;
} Marble_Util_Clock;

typedef struct Marble_Util_FileStream {
	FILE        *flpFilePointer;
	struct stat  sInfo;
	int          iPermissions;
} Marble_Util_FileStream;


typedef struct Marble_AssetManager {
	Marble_Util_Vector *sAtlases;
} Marble_AssetManager;

typedef struct Marble_Asset {
	ULONGLONG uqwGlobalAssetId;
	int       iAssetType;
} Marble_Asset;

typedef struct Marble_Atlas {
	Marble_Asset;
	int iAtlasType;
} Marble_Atlas;

typedef struct Marble_ColorAtlas {
	Marble_Atlas;
	Marble_Util_Vector *sColorTable;
} Marble_ColorAtlas;

typedef struct Marble_Layer {
	DWORD  dwLayerId;
	_Bool  blIsEnabled;
	void  *ptrUserdata;

	struct Marble_Layer_Callbacks sCallbacks;
} Marble_Layer;

typedef struct Marble_Direct2DRenderer {
	ID2D1Factory1      *sD2DFactory;
	ID2D1DeviceContext *sD2DDevContext;
	IDXGISwapChain1    *sDXGISwapchain;
	ID3D11Device       *sD3D11Device;
	IDXGIDevice        *sDXGIDevice;
	ID2D1Device        *sD2DDevice;   
	IDXGIAdapter       *sDXGIAdapter;
	IDXGIFactory2      *sDXGIFactory;
	IDXGISurface       *sDXGIBackbuffer;
	ID2D1Bitmap1       *sBitmap;
} Marble_Direct2DRenderer;

typedef struct Marble_Renderer {
	int iActiveRendererAPI;

	union {
		Marble_Direct2DRenderer sD2DRenderer;
	};
} Marble_Renderer;


extern int                Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData);
extern TCHAR const *const Marble_Event_GetEventTypeName(Marble_EventType eEventType);
extern Marble_EventType   Marble_Event_GetMouseEventType(UINT udwMessage);

extern int  Marble_Renderer_Initialize(DWORD dwActiveAPI, HWND hwRenderWindow);
extern void Marble_Renderer_Uninitialize(void);
extern void Marble_Renderer_BeginDraw(void);
extern void Marble_Renderer_EndDraw(void);
extern int  Marble_Renderer_Present(void);


