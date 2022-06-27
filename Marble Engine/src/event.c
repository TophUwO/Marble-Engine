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
	{ Marble_EventType_Keyboard_KeyRepeated, TEXT("Ev_KeyboardKeyRepeated"), Marble_EventCategory_Input | Marble_EventCategory_Keyboard    }
};
static const DWORD gl_dwNumberOfEventTypes = sizeof(gl_sEventTable) / sizeof(*gl_sEventTable);


int Marble_Event_ConstructEvent(void *ptrEvent, Marble_EventType eEventType, void *ptrData) {
	switch (eEventType) {
		case Marble_EventType_Keyboard_KeyReleased:
		case Marble_EventType_Keyboard_KeyPressed:
			*(Marble_KeyPressedEvent *)ptrEvent = (Marble_KeyPressedEvent){
				.dwKeyCode  = ((Marble_KeyPressedData *)ptrData)->dwKeyCode,
				.blIsSysKey = ((Marble_KeyPressedData *)ptrData)->blIsSysKey
			};

			break;
		case Marble_EventType_Window_Resized:
			*(Marble_WindowResizedEvent *)ptrEvent = (Marble_WindowResizedEvent){
				.dwWidth  = ((Marble_WindowResizedData *)ptrData)->dwWidth,
				.dwHeight = ((Marble_WindowResizedData *)ptrData)->dwHeight
			};

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


