#pragma once

#include <error.h>


typedef enum Marble_EventCategory {
	Marble_EventCategory_None        = 0,

	Marble_EventCategory_Application = 1,
	Marble_EventCategory_Input       = 2,
	Marble_EventCategory_Keyboard    = 4
} Marble_EventCategory;

typedef enum Marble_EventType {
	Marble_EventType_Unknown,

	Marble_EventType_Window_Closed,
	Marble_EventType_Window_Minimized,
	Marble_EventType_Window_Maximized,
	Marble_EventType_Window_Resized,
	Marble_EventType_Window_FocusObtained,
	Marble_EventType_Window_FocusLost,

	Marble_EventType_Keyboard_KeyPressed,
	Marble_EventType_Keyboard_KeyReleased,
	Marble_EventType_Keyboard_KeyRepeated
} Marble_EventType;


typedef struct Marble_WindowResizedData { DWORD dwWidth, dwHeight; }           Marble_WindowResizedData;
typedef struct Marble_KeyPressedData    { DWORD dwKeyCode; _Bool blIsSysKey; } Marble_KeyPressedData;


typedef struct Marble_GenericEvent {
	Marble_EventCategory eCategory;
	Marble_EventType     eType;

	LARGE_INTEGER        uqwTime;
} Marble_GenericEvent, Marble_Event;

typedef struct Marble_WindowEvent {
	Marble_GenericEvent;
} Marble_WindowEvent;

typedef struct Marble_WindowResizedEvent {
	Marble_WindowEvent;
	Marble_WindowResizedData;
} Marble_WindowResizedEvent;

typedef struct Marble_KeyboardEvent {
	Marble_GenericEvent;
	Marble_KeyPressedData;
} Marble_KeyboardEvent, Marble_KeyReleasedEvent, Marble_KeyPressedEvent;


