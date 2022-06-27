#include <api.h>


BOOLEAN WINAPI DllMain(HINSTANCE hiDllHandle, DWORD dwReason, LPVOID lpvReserved) {
    BOOLEAN blIsSuccess = TRUE;
    
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hiDllHandle);

            break;

        case DLL_PROCESS_DETACH:

            break;
    }

    return blIsSuccess;
}


