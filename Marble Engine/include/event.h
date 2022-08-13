#pragma once

#include <api.h>


/* 
 * Event Categories 
 *
 * Allow filtering for specific events
 */
enum marble_evcat {
	MARBLE_EVCAT_NONE     = 0, /* reserved */

	MARBLE_EVCAT_WINDOW   = 1, /* window events */
	/* input events (mouse, keyboard, joystick etc.) */
	MARBLE_EVCAT_INPUT    = 2,
	MARBLE_EVCAT_KEYBOARD = 4, /* keyboard events */
	MARBLE_EVCAT_MOUSE    = 8, /* mouse events */

#if (defined MB_DYNAMIC_LIBRARY)
	__MARBLE_NUMEVCATS__       /* for internal use */
#endif
};

/*
 * Event Types
 * 
 * All event types belong to one or more event categories.
 * These events are only for system events; other events,
 * like in-game events (e.g. ONUSEROPENMAINMENU etc.) are handled
 * in a different module.
 */
enum marble_evtype {
	MARBLE_EVTYPE_UNKNOWN,       /* reserved */

	MARBLE_EVTYPE_WNDCLOSED,     /* WM_CLOSE was received */
	MARBLE_EVTYPE_WNDDMINIMIZED, /* window was minimized */
	MARBLE_EVTYPE_WNDMAXIMIZED,  /* window was maximized */
	/* window was resized (incl. min/max) actions */
	MARBLE_EVTYPE_WNDRESIZED,
	MARBLE_EVTYPE_WNDGOTFOCUS,   /* window got focus */
	MARBLE_EVTYPE_WNDLOSTFOCUS,  /* window lost focus */

	MARBLE_EVTYPE_KBPRESSED,     /* a key has been pressed down */
	MARBLE_EVTYPE_KBRELEASED,    /* a key has been released */
	MARBLE_EVTYPE_KBREPEATED,    /* a key is being held */

	MARBLE_EVTYPE_MBTNPRESSED,   /* a mouse button has been pressed */
	MARBLE_EVTYPE_MBTNRELEASED,  /* a mouse button has been released */
	MARBLE_EVTYPE_MMOVED,        /* mouse was moved */

#if (defined MB_DYNAMIC_LIBRARY)
	__MARBLE_NUMEVTYPES__        /* only for internal use */
#endif
};


/* 
 * Basic event structure
 * 
 * Contains data shared by all derived
 * event types. Every new specialized event
 * type needs to have a variable of this type
 * at its starting address.
 */
struct marble_event {
	enum marble_evcat  m_cat;  /* event category (s.o. for further doc) */
	enum marble_evtype m_type; /* event type (s.o. for further doc) */
	
	/*
	 * After the event has been generated, it gets pushed through
	 * the layerstack. Each layer now has the chance to process the
	 * event. Additionally to processing the event, a layer can set this
	 * flag to true to block further layers to process the event as well.
	 * After the entire layerstack has been traversed, the event will be
	 * removed.
	 */
	bool     m_ishandled;
	uint64_t m_timestamp; /* time at event post; currently unused */
};


/* window-related event */
struct marble_windowevent {
	struct marble_event _base;

	union marble_windowevent_data {
		/* additional data required for handling window resizing */
		struct marble_windowevent_resizeddata {
			uint32_t m_width;  /* new window width (in pixels) */
			uint32_t m_height; /* new window height (in pixels) */
		} _resized;
	} _data;
};

/* keyboard-related event */
struct marble_keyboardevent {
	struct marble_event _base;

	/* additional data required for handling keyboard events */
	struct marble_keyboardevent_data {
		uint32_t m_keycode;  /* virtual key-code */
		bool     m_issyskey; /* system key? */ 
	} _data;
};

/* mouse-related event */
struct marble_mouseevent {
	struct marble_event _base;

	/* additional data required for handling mouse events */
	struct marble_mouseevent_data {
		uint32_t m_buttoncode; /* virtual mouse button code */

		struct marble_pointi2d ms_pos; /* position on the screen */
	} _data;
};


#if (defined MB_DYNAMIC_LIBRARY)
/*
 * Formats event data in a way it then gets passed
 * along within the system.
 * 
 * Returns nothing.
 */
extern void marble_event_construct(
	void *p_evptr,           /* pointer to memory receiving data */
	enum marble_evtype type, /* type of the event */ 
	void const *p_evdata     /* additional data */
);

/*
 * Gets mouse event type from a Windows message.
 * 
 * If the message is not a valid windows mouse message,
 * the function returns MARBLE_EVTYPE_UNKNOWN.
 * 
 * Returns converted mouse event type.
 */
extern enum marble_evtype marble_event_getmouseevent(
	uint32_t msgid /* message id to convert */
);
#endif


