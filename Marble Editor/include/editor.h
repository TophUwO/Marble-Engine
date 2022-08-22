#pragma once

#include <tsetview.h>


/*
 * Structure representing the application instance
 * of the editor. Everything that is created by the editor
 * throughout its lifetime is owned by this structure.
 */
extern struct mbeditor_application {
	HINSTANCE mp_hinst; /* application instance */
	HWND      mp_hwnd;  /* main window */

	/* system resources */
	struct {
		HFONT mp_hguifont; /* GUI font */
	} ms_res;

	/* tileset view */
	struct mbeditor_tsetview ms_tsview;
} gls_editorapp;


/*
 * Initializes application.
 * 
 * Returns 0 on success, non-zero on failure.
 */
extern marble_ecode_t mbeditor_init(
	_In_   HINSTANCE p_hinst,
	_In_z_ LPSTR pz_cmdline
);

/*
 * Starts main loop.
 * 
 * Returns error code to return back to the host environment.
 */
extern marble_ecode_t mbeditor_run(
	void
);


