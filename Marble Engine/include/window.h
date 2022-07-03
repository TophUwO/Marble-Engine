#pragma once

#include <api.h>
#include <error.h>


typedef struct Marble_Window {
	HWND hwWindow;

	struct Marble_WindowData {
		TCHAR   *strTitle;
		DWORD    dwWidth;
		DWORD    dwHeight;
		_Bool    blIsVSync;
		_Bool    blIsFullscreen;
		uint64_t uqwLastTitleUpdate; 
	} sWndData;
} Marble_Window;


extern int         Marble_Window_Create(Marble_Window **ptrpWindow, TCHAR *strTitle, DWORD dwWidth, DWORD dwHeight, _Bool blIsVSync);
extern void        Marble_Window_Destroy(Marble_Window **ptrpWindow);
extern void inline Marble_Window_SetVsyncEnabled(Marble_Window *sWindow, _Bool blIsEnabled);
extern void inline Marble_Window_SetFullscreen(Marble_Window *sWindow, _Bool blIsFullscreen);
extern void        Marble_Window_Update(float fTimeStep);


