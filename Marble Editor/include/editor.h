#pragma once

#include <..\vcpp\resource.h>

#include <tabview.h>
#include <dialog.h>
#include <levelview.h>


/*
 * Structure representing the application instance
 * of the editor. Everything that is created by the editor
 * throughout its lifetime is owned by this structure.
 */
extern struct mbe_application {
	HINSTANCE mp_hinst; /* application instance */
	HWND      mp_hwnd;  /* main window */

	/* system resources */
	struct {
		HFONT mp_hguifont;  /* GUI font */
		HMENU mp_hmainmenu; /* main window menu bar */

		/* Direct2D resources */
		ID2D1Factory     *mp_d2dfac;   /* for Direct2D rendering */
		ID2D1StrokeStyle *mp_grstroke; /* grid-line stroke style */
	} ms_res;

	/* Application-wide flags. */
	struct {
		BOOL mf_isshift; /* state of shift key */
		BOOL mf_isdest;  /* destroy flag */
	} ms_flags;

	struct mbe_tabview *mps_tsview;  /* tileset view */
	struct mbe_tabview *mps_lvlview; /* level view */
} gls_editorapp;


/*
 * Initializes application.
 * 
 * Returns 0 on success, non-zero on failure.
 */
extern marble_ecode_t mbe_editor_init(
	HINSTANCE p_hinst, /* application instance */
	LPSTR pz_cmdline   /* command line arguments */
);

/*
 * Starts main loop.
 * 
 * Returns error code to return back to the host environment.
 */
extern marble_ecode_t mbe_editor_run(void);


