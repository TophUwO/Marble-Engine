#pragma once

#include <layer.h>
#include <util.h>
#include <window.h>
#include <renderer.h>
#include <asset.h>
#include <atlas.h>
#include <color.h>

#define Marble_IfError(expr, equal, body) if ((expr) != equal) { body; }
/*
	* Macro used to initialize error code variable. Will always be put on the
	* line as the function head.
*/
#define MARBLE_ERRNO                      int iErrorCode = Marble_ErrorCode_Ok;

/* User-defined custom window messages */
#define MARBLE_WM_START (WM_USER)
#define MARBLE_WM_FATAL (MARBLE_WM_START + 1)


enum Marble_Internal_AppState {
	Marble_AppState_Init,
	Marble_AppState_Running,
	Marble_AppState_Shutdown,
	Marble_AppState_ForcedShutdown
};


/*
	* Marble's application instance; acts as a 
	* singleton and holds the ownership to all
	* objects and components created by or submitted
	* to the engine.
*/
extern struct Marble_Application {
	HINSTANCE     hiInstance;
	PSTR          astrCommandLine;
	LARGE_INTEGER uPerfFreq;
	LARGE_INTEGER uFTLast;
	HANDLE        htMainThread;
	uint32_t      ui32HashSeed;
	struct Marble_Application_AppState {
		int   iState;
		_Bool blIsFatal;
		int   iParameter;
	} sAppState;

	Marble_Window       *sMainWindow;
	Marble_Renderer     *sRenderer;
	Marble_AssetManager *sAssets;
	Marble_LayerStack   *sLayers;
} gl_sApplication;

extern int                Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData);
extern TCHAR const *const Marble_Event_GetEventTypeName(Marble_EventType eEventType);
extern Marble_EventType   Marble_Event_GetMouseEventType(UINT udwMessage);

extern int  Marble_System_Internal_CreateDebugConsole(void);
extern int  Marble_System_Cleanup(int iRetCode);
extern void Marble_System_InitiateShutdown(int iRetCode);
extern void Marble_System_RaiseFatalError(int iErrorCode);
extern void Marble_System_SetAppState(_Bool blIsFatal, int iParameter, int iAppState);
extern void Marble_System_ClearAppState(void);

/// <summary>
/// Simple memory allocator, able to choose between malloc() and calloc() automatically (using *blNeedZeroed*) with the
/// possibility to throw a fatal error if the function should fail (as indicated by *blIsFatalOnFailure*).
/// </summary>
/// <param name="ptrpMemoryPointer"> > Pointer to receive the pointer to the allocated portion of memory </param>
/// <param name="stSize"> > Size in bytes</param>
/// <param name="blNeedZeroed"> > Should the memory block be zeroed? </param>
/// <param name="blIsFatalOnFailure"> > In case the allocation fails, should the function raise a fatal error?</param>
/// <returns>0 on success, non-zero on failure.</returns>
extern int  Marble_System_AllocateMemory(void **ptrpMemoryPointer, size_t stSize, _Bool blNeedZeroed, _Bool blIsFatalOnFailure);


