#pragma once

#include <api.h>


INT APIENTRY WinMain(_In_ HINSTANCE hiInstance, _In_opt_ HINSTANCE hiPrevInstance, _In_ PSTR astrCommandLine, _In_ int dwShowCommand) {
	MARBLE_API int Marble_System_InitializeApplication(HINSTANCE hiInstance, PSTR astrCommandLine);
	MARBLE_API int Marble_System_RunApplication();

	if (Marble_System_InitializeApplication(hiInstance, astrCommandLine))
		return EXIT_FAILURE;

	return Marble_System_RunApplication();
}


