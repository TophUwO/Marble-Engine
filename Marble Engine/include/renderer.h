#pragma once

#include <dxgi1_2.h>
#include <d3d11.h>
#include <d2dwr.h>
#include <dwritewr.h>


enum Marble_RendererAPI {
	Marble_RendererAPI_Direct2D = 1
};

typedef enum Marble_FontWeight {
    Marble_FontWeight_Thin       = 100,
    Marble_FontWeight_ExtraLight = 200,
    Marble_FontWeight_UltraLight = 200,
    Marble_FontWeight_Light      = 300,
    Marble_FontWeight_SemiLight  = 350,
    Marble_FontWeight_Normal     = 400,
    Marble_FontWeight_Regular    = 400,
    Marble_FontWeight_Medium     = 500,
    Marble_FontWeight_DemiBold   = 600,
    Marble_FontWeight_SemiBold   = 600,
    Marble_FontWeight_Bold       = 700,
    Marble_FontWeight_ExtraBold  = 800,
    Marble_FontWeight_UltraBold  = 800,
    Marble_FontWeight_Black      = 900,
    Marble_FontWeight_Heavy      = 900,
    Marble_FontWeight_ExtraBlack = 950,
    Marble_FontWeight_UltraBlack = 950
} Marble_FontWeight;

typedef enum Marble_FontStyle {
    Marble_FontStyle_Normal,
    Marble_FontStyle_Oblique,
    Marble_FontStyle_Italic
} Marble_FontStyle;


typedef struct Marble_Direct2DRenderer {
	ID2D1Factory1      *sD2DFactory;
	ID2D1DeviceContext *sD2DDevContext;
	IDXGISwapChain1    *sDXGISwapchain;
	ID2D1Device        *sD2DDevice;
	ID2D1Bitmap1       *sD2DBitmap;
	IDWriteFactory     *sDWriteFactory;
} Marble_Direct2DRenderer;

typedef struct Marble_TextFormat {
    int iRendererAPI;

    union {
        IDWriteTextFormat *sDWriteTextFormat;
    };
} Marble_TextFormat;

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
extern void Marble_Renderer_Resize(Marble_Renderer **ptrpRenderer, UINT uiNewWidth, UINT uiNewHeight);
extern void Marble_Renderer_DrawText(Marble_Renderer *sRenderer, WCHAR const *wstrText, Marble_TextFormat *sFormat, int iXPos, int iYPos, ID2D1Brush *sBrush);
extern int  Marble_Renderer_CreateTextFormat(Marble_Renderer *sRenderer, WCHAR const *wstrFamily, float fSize, Marble_FontWeight eWeight, Marble_FontStyle eStyle, Marble_TextFormat **ptrpTextFormat);
extern void Marble_Renderer_DestroyTextFormat(Marble_TextFormat **ptrpTextFormat);


