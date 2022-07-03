#pragma once

#include <dxgi1_2.h>
#include <d3d11.h>
#include <d2dwr.h>


enum Marble_RendererAPI {
	Marble_RendererAPI_Direct2D = 1
};


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


extern int  Marble_Renderer_Create(Marble_Renderer **ptrpRenderer, DWORD dwActiveAPI, HWND hwRenderWindow);
extern void Marble_Renderer_Destroy(Marble_Renderer **ptrpRenderer);
extern void Marble_Renderer_BeginDraw(Marble_Renderer *sRenderer);
extern void Marble_Renderer_EndDraw(Marble_Renderer *sRenderer);
extern int  Marble_Renderer_Present(Marble_Renderer **ptrpRenderer);
extern void Marble_Renderer_Resize(Marble_Renderer *sRenderer, UINT uiNewWidth, UINT uiNewHeight);


