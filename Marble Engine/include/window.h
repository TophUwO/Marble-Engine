#pragma once

#include <api.h>
#include <error.h>


typedef struct Marble_Window {
	HWND hwWindow;

	struct Marble_WindowData {
		TCHAR *strTitle;
		DWORD  dwWidth;
		DWORD  dwHeight;
		_Bool  blIsVSync;
	} sWndData;
} Marble_Window;


extern int Marble_Window_Create(Marble_Window **ptrpWindow, TCHAR *strTitle, DWORD dwWidth, DWORD dwHeight, _Bool blIsVSync);
extern int Marble_Window_Destroy(Marble_Window **ptrpWindow);

extern int Marble_Window_OnUpdate(Marble_Window *sWindow);


