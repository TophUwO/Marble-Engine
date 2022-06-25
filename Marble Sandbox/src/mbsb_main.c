#include <windows.h>
#include <stdio.h>

#include <mbmn.h>


int APIENTRY WinMain(_In_ HINSTANCE hiInstance, _In_opt_ HINSTANCE hiPrevInstance, _In_ PSTR astrCommandLine, _In_ int dwShowCommand) {
	MarbleMain_System_CreateDebugConsole();

	printf("Hello, world!\n");

	system("pause");
	return EXIT_SUCCESS;
}