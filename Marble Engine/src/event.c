#include <event.h>
#include <error.h>


/* Table representing additional event data; mainly used for filtering. */
static struct marble_event_eventinfo { enum marble_evtype m_type; enum marble_evcat m_cat; char const *const pz_name; } const glsa_evinfotable[] = {
	{ MARBLE_EVTYPE_UNKNOWN, MARBLE_EVCAT_NONE, "Ev_Unknown" },

	{ MARBLE_EVTYPE_WNDCLOSED,     MARBLE_EVCAT_WINDOW,                        "Ev_WindowClosed"        },
	{ MARBLE_EVTYPE_WNDDMINIMIZED, MARBLE_EVCAT_WINDOW,                        "Ev_WindowMinimzed"      },
	{ MARBLE_EVTYPE_WNDMAXIMIZED,  MARBLE_EVCAT_WINDOW,                        "Ev_WindowMaximized"     },
	{ MARBLE_EVTYPE_WNDRESIZED,    MARBLE_EVCAT_WINDOW,                        "Ev_WindowResized"       },
	{ MARBLE_EVTYPE_WNDGOTFOCUS,   MARBLE_EVCAT_WINDOW,                        "Ev_WindowGotFocus"      },
	{ MARBLE_EVTYPE_WNDLOSTFOCUS,  MARBLE_EVCAT_WINDOW,                        "Ev_WindowLostFocus"     },

	{ MARBLE_EVTYPE_KBPRESSED,     MARBLE_EVCAT_INPUT | MARBLE_EVCAT_KEYBOARD, "Ev_KeyboardKeyPressed"  }, 
	{ MARBLE_EVTYPE_KBRELEASED,    MARBLE_EVCAT_INPUT | MARBLE_EVCAT_KEYBOARD, "Ev_KeyboardKeyReleased" },
	{ MARBLE_EVTYPE_KBREPEATED,    MARBLE_EVCAT_INPUT | MARBLE_EVCAT_KEYBOARD, "Ev_KeyboardKeyRepeated" },

	{ MARBLE_EVTYPE_MBTNPRESSED,  MARBLE_EVCAT_INPUT | MARBLE_EVCAT_MOUSE,     "Ev_MouseButtonPressed"  },
	{ MARBLE_EVTYPE_MBTNRELEASED, MARBLE_EVCAT_INPUT | MARBLE_EVCAT_MOUSE,     "Ev_MouseButtonReleased" },
	{ MARBLE_EVTYPE_MMOVED,       MARBLE_EVCAT_INPUT | MARBLE_EVCAT_MOUSE,     "Ev_MouseMoved"          }
};


/*
 * Gets event type name.
 * 
 * This function is only used for debugging purposes.
 * 
 * Returns event type name.
 */
static char const *const marble_event_internal_getevtypename(
	_In_range_(MARBLE_EVTYPE_UNKNOWN, __MARBLE_NUMEVTYPES__ - 1)
	enum marble_evtype type
) {
	if (type >= __MARBLE_NUMEVTYPES__ || type < 0)
		return marble_event_internal_getevtypename(MARBLE_EVTYPE_UNKNOWN);

	return glsa_evinfotable[type].pz_name;
}


void marble_event_construct(
	_Out_    void *p_evptr,
	_In_range_(MARBLE_EVTYPE_UNKNOWN, __MARBLE_NUMEVTYPES__ - 1)
	         enum marble_evtype type,
	_In_opt_ void const *p_evdata
) {
	if (p_evptr == NULL)
		return;

	/* Initialize the entire block with 0 first. */
	memset(
		p_evptr,
		0,
		sizeof(struct marble_event)
	);

	/*
	 * Copy-over event-specific data.
	 * 
	 * As this function is always going to be called from within
	 * the engine, there is no way the data given can be invalid (i.e.
	 * incorrect struct data passed for a particular event type).
	 * Therefore, just copying over by casting the pointer
	 * is of no great danger.
	 */
	switch (type) {
		case MARBLE_EVTYPE_KBPRESSED:
		case MARBLE_EVTYPE_KBRELEASED:
			if (p_evdata == NULL)
				return;

			((struct marble_keyboardevent *)p_evptr)->_data = *(struct marble_keyboardevent_data *)p_evdata;

			break;
		case MARBLE_EVTYPE_WNDRESIZED:
			if (p_evdata == NULL)
				return;

			((struct marble_windowevent *)p_evptr)->_data._resized = *(struct marble_windowevent_resizeddata *)p_evdata;

			break;
		case MARBLE_EVTYPE_MBTNPRESSED:
		case MARBLE_EVTYPE_MBTNRELEASED:
		case MARBLE_EVTYPE_MMOVED:
			if (p_evdata == NULL)
				return;

			((struct marble_mouseevent *)p_evptr)->_data = *(struct marble_mouseevent_data *)p_evdata;

			break;
	}

	*(struct marble_event *)p_evptr = (struct marble_event){
		.m_type      = type,
		.m_cat       = glsa_evinfotable[type].m_cat,
		.m_ishandled = false
	};
	QueryPerformanceCounter((LARGE_INTEGER *)&((struct marble_event *)p_evptr)->m_timestamp);
}

enum marble_evtype marble_event_getmouseevent(uint32_t msgid) {
	switch (msgid) {
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN: return MARBLE_EVTYPE_MBTNPRESSED;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:   return MARBLE_EVTYPE_MBTNRELEASED;
		case WM_MOUSEMOVE:   return MARBLE_EVTYPE_MMOVED;
	}

	return MARBLE_EVTYPE_UNKNOWN;
}


