#pragma once

#include <marble.h>


INT APIENTRY WinMain(_In_ HINSTANCE hiInstance, _In_opt_ HINSTANCE hiPrevInstance, _In_ PSTR astrCommandLine, _In_ int dwShowCommand) {
	MARBLE_API int Marble_System_InitializeApplication(HINSTANCE hiInstance, PSTR astrCommandLine, int (*OnUserInit)(void));
	MARBLE_API int Marble_System_RunApplication();
	extern     int Marble_System_OnUserInit(void);

	int iErrorCode = Marble_ErrorCode_Ok;
	if (iErrorCode = Marble_System_InitializeApplication(hiInstance, astrCommandLine, &Marble_System_OnUserInit))
		return iErrorCode;

	return Marble_System_RunApplication();
}


