#pragma once

#pragma warning (disable: 4201) /* non-standard unnamed struct/union */
#pragma warning (disable: 5105) /* macro expansion producing preproc symbol is UB */

#include <system.h>
#include <marble.h>
#include <util.h>
#include <window.h>
#include <renderer.h>
#include <asset.h>
#include <level.h>
#include <log.h>
#include <uuid.h>


MB_BEGIN_HEADER


/* User-defined custom window messages */
#define MB_WM_START (WM_USER)
#define MB_WM_FATAL (MB_WM_START + 1)


/* application state IDs */
enum marble_app_stateid {
	MARBLE_APPSTATE_UNKNOWN = 0, /* reserved */

	MARBLE_APPSTATE_INIT,        /* during initialization */
	MARBLE_APPSTATE_RUNNING,     /* running */
	MARBLE_APPSTATE_SHUTDOWN,    /* during an orderly shutdown */
	/*
	 * during a shutdown initiated by
	 * a fatal application error
	 */
	MARBLE_APPSTATE_FORCEDSHUTDOWN
};


/* Layer structure */
struct marble_layer {
	char  maz_stringid[MB_STRINGIDMAX]; /* string ID; used for debugging */
	void *mp_userdata;                  /* userdata */
	int   m_id;                         /* integer ID */
	bool  m_isenabled;                  /* active? */
	bool  m_istopmost;                  /* topmost layer? */
	bool  m_ispushed;                   /* on the layer stack? */

	/*
	 * Layer callbacks
	 * 
	 * Contains functions that will be executed
	 * whenever a layer is being updated, i.e. once per
	 * frame, or an application event occurs.
	 * Additional layer functions can be defined to 
	 * allow executing code when a layer is being pushed
	 * and popped from the internal layer stack.
	 */
	struct marble_layer_cbs ms_cbs;
};


/*
 * Marble's application instance; acts as a 
 * singleton and holds the ownership to all
 * objects and components created by or submitted
 * to the engine.
 */
struct marble_application {
	HINSTANCE mp_inst;      /* instance to the application */
	HANDLE    mp_mainthrd;  /* handle to main thread */
	bool      m_hasmainwnd; /* Do we already have a main window? */
	/* structure holding state information about the application */
	struct marble_app_state {
		enum marble_app_stateid m_id; /* state id */

		bool m_isfatal; /* fatal error occurred? */
		int  m_param;   /* additional parameter */
	} ms_state;

	struct marble_window *ps_wnd; /* renderwindow */
	/* 
	 * Layer stack
	 * 
	 * Keeps layers organized and therefore controls
	 * in which order the application updates.
	 */
	struct marble_layerstack {
		bool   m_isinit;    /* Is the layer stack present? */
        size_t m_lastlayer; /* greatest non-overlay layer index */

		/*
         * vector structure holding the
         * layer pointers
         */
		struct marble_util_vec *mps_vec;
	} ms_layerstack;
    
    /*
     * Asset manager
     * 
     * Responsible for keeping and cleaning-up all assets that
     * has been submitted to or created by Marble throughout
     * its life-time.
     */
    struct marble_assetman *mps_assets;

    /* HPC -- used for frametime, etc. */
    struct marble_util_clock ms_ftclock;
};


/*
 * Sets app-state.
 * 
 * This function is currently only used for
 * debugging purposes.
 * 
 * Returns nothing.
 */
extern void marble_application_setstate(
	bool isfatal, 
	int param, 
	enum marble_app_stateid newid
);


/*
 * Raises a fatal error.
 * 
 * The application will forcefully quit the next time
 * the message queue is traversed.
 * 
 * Returns nothing.
 */
extern void marble_application_raisefatalerror(
	marble_ecode_t ecode /* error code that will be returned to host environment */
);


MB_END_HEADER


