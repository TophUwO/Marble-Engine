#include <event.h>


static struct Marble_Internal_EventEntry { Marble_EventType eType; Marble_EventCategory eCategory; } const gl_sEventTable[] = {
	{ Marble_EventType_Window_Closed,        Marble_EventCategory_Application                              },
	{ Marble_EventType_Window_Minimized,     Marble_EventCategory_Application                              },
	{ Marble_EventType_Window_Maximized,     Marble_EventCategory_Application                              },
	{ Marble_EventType_Window_Resized,       Marble_EventCategory_Application                              },
	{ Marble_EventType_Window_FocusObtained, Marble_EventCategory_Application | Marble_EventCategory_Input },
	{ Marble_EventType_Window_FocusLost,     Marble_EventCategory_Application | Marble_EventCategory_Input },

	{ Marble_EventType_Keyboard_KeyPressed,  Marble_EventCategory_Input | Marble_EventCategory_Keyboard    }, 
	{ Marble_EventType_Keyboard_KeyReleased, Marble_EventCategory_Input | Marble_EventCategory_Keyboard    },
	{ Marble_EventType_Keyboard_KeyRepeated, Marble_EventCategory_Input | Marble_EventCategory_Keyboard    }
};


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


