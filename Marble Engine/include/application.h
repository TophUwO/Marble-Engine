#pragma once

#pragma warning (disable: 4201) /* non-standard unnamed struct/union */
#pragma warning (disable: 5105) /* macro expansion producing preproc symbol is UB */

#include <system.h>
#include <marble.h>
#include <util.h>
#include <window.h>
#include <renderer.h>
#include <asset.h>
#include <log.h>


/*
 * Macro used to initialize COM error code variable.
 * Will always be put on the line as the function head.
 */
#define MB_COMERRNO HRESULT hres = S_OK;

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
extern struct marble_application {
	HINSTANCE     mp_inst;       /* instance to the application */
	HANDLE        mp_mainthrd;   /* handle to main thread */
	LARGE_INTEGER m_perfcounter; /* performance counter; used for HTPs */
	bool          m_hasmainwnd;  /* Do we already have a main window? */
	/* structure holding state information about the application */
	struct marble_app_state {
		enum marble_app_stateid m_id; /* state id */

		bool m_isfatal; /* fatal error occurred? */
		int  m_param;   /* additional parameter */
	} ms_state;

	struct marble_window   *mps_window;   /* main window */
	struct marble_renderer *mps_renderer; /* renderer associated with main window */
	/* 
	 * Layer stack
	 * 
	 * Keeps layers organized and therefore controls
	 * in which order the application updates.
	 */
	struct marble_layerstack {
		bool m_isinit; /* Is the layer stack present? */
		/* vector structure holding the layer pointers */
		struct marble_util_vec *mps_vec;

		size_t m_lastlayer; /* greatest non-overlay layer index */
	} ms_layerstack;
	/*
	 * Asset manager
	 * 
	 * Responsible for keeping and cleaning-up all assets that
	 * has been submitted to or created by Marble throughout
	 * its life-time.
	 */
	struct marble_app_assetman {
		bool m_isinit; /* Is the asset manager present? */

		struct marble_util_htable *mps_table; /* asset storage */
	} ms_assets;
} gl_app;


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
void inline marble_application_raisefatalerror(
	marble_ecode_t ecode /* error code that will be returned to host environment */
) {
	marble_log_fatal(
		NULL,
		"Fatal error occurred: (%i)\n    %s\n    %s",
		(int)ecode,
		marble_error_getstr(ecode),
		marble_error_getdesc(ecode)
	);

	marble_application_setstate(
		true,
		ecode,
		MARBLE_APPSTATE_FORCEDSHUTDOWN
	);

	/* Ask our main thread to quit. */
	PostThreadMessage(GetThreadId(gl_app.mp_mainthrd), MB_WM_FATAL, 0, 0);
}


