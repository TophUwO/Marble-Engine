#pragma once

#include <marble.h>


INT APIENTRY WinMain(_In_ HINSTANCE hiInstance, _In_opt_ HINSTANCE hiPrevInstance, _In_ PSTR astrCommandLine, _In_ int dwShowCommand) {
	MARBLE_API int Marble_Application_Initialize(HINSTANCE hiInstance, PSTR astrCommandLine, int (*OnUserInit)(void));
	MARBLE_API int Marble_Application_Run();
	extern     int Marble_Application_OnUserInit(void); /* to be defined by user application */

	Marble_Application_Initialize(hiInstance, astrCommandLine, &Marble_Application_OnUserInit);

	return Marble_Application_Run();
}


