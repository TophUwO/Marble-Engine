#pragma once

#include <tsetview.h>


extern struct marble_editor {
	HINSTANCE mp_hinst;
	HWND      mp_hwnd;

	struct marble_editor_tsetview ms_tsview;
} gls_editorapp;


extern marble_ecode_t marble_editor_init(
	_In_   HINSTANCE p_hinst,
	_In_z_ LPSTR pz_cmdline
);

extern marble_ecode_t marble_editor_run(
	void
);


