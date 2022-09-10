#pragma once

#include <..\vcpp\resource.h>

#include <tsetview.h>

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
		HFONT  mp_hguifont; /* GUI font */
		HBRUSH mp_hbrwhite; /* white brush */
		HBRUSH mp_hbrblack; /* black brush */
		HPEN   mp_hpsel;    /* selection outline pen */
		HPEN   mp_hpgrid;   /* grid pen */
	} ms_res;

	/* Application-wide flags. */
	struct {
		BOOL mf_isshift; /* state of shift key */
		BOOL mf_isdest;  /* destroy flag */
	} ms_flags;

	struct mbe_tsetview ms_tsview;   /* tileset view */
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


