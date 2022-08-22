#pragma once

#include <editor.h>
#include <tsetview.h>


marble_ecode_t mbeditor_tsetview_init(
	HWND p_hwndparent,
	struct mbeditor_tsetview *ps_tsetview
) {
	if (ps_tsetview == NULL)
		return MARBLE_EC_PARAM;

	/* Create tab view window. */
	ps_tsetview->mp_hwnd = CreateWindowEx(
		WS_EX_WINDOWEDGE | WS_EX_COMPOSITED,
		WC_TABCONTROL,
		TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		0,
		0,
		150,
		700,
		p_hwndparent,
		NULL,
		gls_editorapp.mp_hinst,
		ps_tsetview
	);
	if (ps_tsetview->mp_hwnd == NULL)
		return MARBLE_EC_CREATEWND;

	/* Set tab text font. */
	SendMessage(
		ps_tsetview->mp_hwnd,
		WM_SETFONT,
		(WPARAM)gls_editorapp.ms_res.mp_hguifont,
		(LPARAM)TRUE
	);

	/* Add dummy item. */
	TCITEM s_item = {
		.mask    = TCIF_TEXT,
		.pszText = TEXT("<empty>")
	};
	TabCtrl_InsertItem(ps_tsetview->mp_hwnd, 0, &s_item);

	return MARBLE_EC_OK;
}

void mbeditor_tsetview_uninit(
	struct mbeditor_tsetview *ps_tsetview
) {
	
}

void mbeditor_tsetview_resize(
	struct mbeditor_tsetview *ps_tsetview,
	int nwidth,
	int nheight
) {
	SetWindowPos(
		ps_tsetview->mp_hwnd,
		NULL,
		0,
		0,
		150,
		nheight,
		SWP_NOACTIVATE | SWP_NOMOVE
	);
}


