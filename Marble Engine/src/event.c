#include <internal.h>


static struct Marble_Internal_EventEntry { Marble_EventType eType; TCHAR const *const strName; Marble_EventCategory eCategory; } const gl_sEventTable[] = {
	{ Marble_EventType_Unknown,              TEXT("Ev_Unknown"),             Marble_EventCategory_None,                                    },

	{ Marble_EventType_Window_Closed,        TEXT("Ev_WindowClosed"),        Marble_EventCategory_Application                              },
	{ Marble_EventType_Window_Minimized,     TEXT("Ev_WindowMinimzed"),      Marble_EventCategory_Application                              },
	{ Marble_EventType_Window_Maximized,     TEXT("Ev_WindowMaximized"),     Marble_EventCategory_Application                              },
	{ Marble_EventType_Window_Resized,       TEXT("Ev_WindowResized"),       Marble_EventCategory_Application                              },
	{ Marble_EventType_Window_FocusObtained, TEXT("Ev_WindowFocusObtained"), Marble_EventCategory_Application | Marble_EventCategory_Input },
	{ Marble_EventType_Window_FocusLost,     TEXT("Ev_WindowFocusLost"),     Marble_EventCategory_Application | Marble_EventCategory_Input },

	{ Marble_EventType_Keyboard_KeyPressed,  TEXT("Ev_KeyboardKeyPressed"),  Marble_EventCategory_Input | Marble_EventCategory_Keyboard    }, 
	{ Marble_EventType_Keyboard_KeyReleased, TEXT("Ev_KeyboardKeyReleased"), Marble_EventCategory_Input | Marble_EventCategory_Keyboard    },
	{ Marble_EventType_Keyboard_KeyRepeated, TEXT("Ev_KeyboardKeyRepeated"), Marble_EventCategory_Input | Marble_EventCategory_Keyboard    },

	{ Marble_EventType_Mouse_ButtonPressed,  TEXT("Ev_MouseButtonPressed"),  Marble_EventCategory_Input | Marble_EventCategory_Mouse       },
	{ Marble_EventType_Mouse_ButtonReleased, TEXT("Ev_MouseButtonReleased"), Marble_EventCategory_Input | Marble_EventCategory_Mouse       },
	{ Marble_EventType_Mouse_MouseMoved,     TEXT("Ev_MouseButtonMoved"),    Marble_EventCategory_Input | Marble_EventCategory_Mouse       }
};
static const DWORD gl_dwNumberOfEventTypes = sizeof(gl_sEventTable) / sizeof(*gl_sEventTable);


int Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData) {
	switch (eEventType) {
		case Marble_EventType_Keyboard_KeyReleased:
		case Marble_EventType_Keyboard_KeyPressed:
			*(Marble_KeyPressedEvent *)ptrEvent = *(Marble_KeyPressedEvent *)ptrData;

			break;
		case Marble_EventType_Window_Resized:
			*(Marble_WindowResizedEvent *)ptrEvent = *(Marble_WindowResizedEvent *)ptrData;

			break;
		case Marble_EventType_Mouse_ButtonPressed:
		case Marble_EventType_Mouse_ButtonReleased:
		case Marble_EventType_Mouse_MouseMoved:
			*(Marble_MouseEvent *)ptrEvent = *(Marble_MouseEvent *)ptrData;

			break;
	}

	*(Marble_GenericEvent *)ptrEvent = (Marble_GenericEvent){
		.eType = eEventType,
		.eCategory = gl_sEventTable[eEventType].eCategory
	};
	QueryPerformanceCounter(&((Marble_GenericEvent *)ptrEvent)->uqwTime);

	return Marble_ErrorCode_Ok;
}

TCHAR const *const Marble_Event_GetEventTypeName(Marble_EventType eEventType) {
	if ((DWORD)eEventType >= gl_dwNumberOfEventTypes || eEventType < 0)
		return Marble_Event_GetEventTypeName(Marble_EventType_Unknown);

	return gl_sEventTable[eEventType].strName;
}

Marble_EventType Marble_Event_GetMouseEventType(UINT udwMessage) {
	switch (udwMessage) {
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN: return Marble_EventType_Mouse_ButtonPressed;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:   return Marble_EventType_Mouse_ButtonReleased;
		case WM_MOUSEMOVE:   return Marble_EventType_Mouse_MouseMoved;
	}

	return Marble_EventType_Unknown;
}


