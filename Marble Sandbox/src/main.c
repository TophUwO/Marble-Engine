#include <windows.h>
#include <stdio.h>

#include <marble.h>


static int __onInit__(void) {
	return 0;
}

static int __onUpdate__(void) {
	return 0;
}

static int __onCleanup__(void) {
	return 0;
}


int APIENTRY WinMain(_In_ HINSTANCE hiInstance, _In_opt_ HINSTANCE hiPrevInstance, _In_ PSTR astrCommandLine, _In_ int dwShowCommand) {
	MarbleMain_System_CreateDebugConsole();
	if (MarbleMain_System_InitializeApplication(hiInstance, astrCommandLine))
		return EXIT_FAILURE;
	MarbleMain_System_RegisterHandlers(&__onInit__, &__onUpdate__, &__onCleanup__);

	return MarbleMain_System_RunApplication();
}