#pragma once

#include <base.h>


/*
 * Structure representing a tab page. The tab view
 * will only be able to render one page at a time
 * as all pages share one window.
 */
struct mbe_tabpage {
	TCHAR *mpz_name; /* name of page */
	TCHAR *mpz_cmt;  /* comment, displayed via tooltip */
	BOOL   m_isinit; /* tab ready-state */
	void  *mp_udata; /* userdata */

	struct mbe_tabview    *ps_parent;  /* parent */
	struct mbe_scrollinfo  ms_scrinfo; /* scrollbar info */
};

/*
 * Structure representing a generic tab view.
 * Serves as a container of sorts and controls sizing
 * and switching of pages.
 */
struct mbe_tabview {
	HWND mp_hwndtab;    /* window of tab view */
	HWND mp_hwndpage;   /* window of visible page */
	RECT ms_dimensions; /* client dimensions */
	BOOL m_isinit;      /* init-state */

	struct mbe_tabpage *mps_cursel;    /* currently selected page */
	struct marble_util_vec *mps_pages; /* list of pages */
};


/*
 * Creates a tab view, including its page window, and sets it up
 * for handling pages.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ extern marble_ecode_t mbe_tabview_create(
	_In_              HWND p_hparent,                    /* parent window handle */
	_In_              struct mbe_wndsize const *ps_size, /* size parameters */
	_Init_(pps_tview) struct mbe_tabview **pps_tview     /* tabview to init */
);

/*
 * Destroys a tileset view, all of its pages and all resources otherwise
 * associated with the view.
 * 
 * Returns nothing.
 */
extern void mbe_tabview_destroy(
	_Uninit_(pps_tview) struct mbe_tabview **pps_tview /* tabview to uninit */
);

/*
 * Moves and/or resizes a tab-view.
 * 
 * Returns nothing.
 */
extern void mbe_tabview_resize(
	_In_ struct mbe_tabview *ps_tview,     /* tabview to resize */
	_In_ struct mbe_wndsize const *ps_dims /* new window position and size */
);


/*
 * Registers a custom window class based on the
 * Common Controls' tab view.
 * 
 * Returns 0 on success, non-zero on failure.
 */
inline marble_ecode_t mbe_tabview_inl_regwndclass(void) {
	extern WNDPROC glfn_tabviewdefproc; /* original tabctrl window procedure */
	/* custom tabctrl window procedure */
	extern LRESULT CALLBACK mbe_tabview_int_wndproc(HWND, UINT, WPARAM, LPARAM);
	/* tabpage window procedure */
	extern LRESULT CALLBACK mbe_tabpage_int_wndproc(HWND, UINT, WPARAM, LPARAM);
	extern TCHAR const *const glpz_tabviewwndcl; /* custom tab view class identifier */
	extern TCHAR const *const glpz_pagewndcl;    /* tab page class identifier */

	WNDCLASSEX s_class;

#pragma region REGTABCTRLCLASS
	/* Get class attributes. */
	if (GetClassInfoEx(GetModuleHandle(TEXT("comctl32.dll")), WC_TABCONTROL, &s_class) == FALSE)
		return MARBLE_EC_GETWNDCLASSATTRIBS;

	/* Save original window procedure. */
	glfn_tabviewdefproc = s_class.lpfnWndProc; 

	/* Setup new class attributes. */
	s_class.cbSize        = sizeof s_class;
	s_class.lpszClassName = glpz_tabviewwndcl;
	s_class.lpfnWndProc   = (WNDPROC)&mbe_tabview_int_wndproc;

	/* Register modified window class. */
	if (RegisterClassEx(&s_class) == FALSE)
		return MARBLE_EC_REGWNDCLASS;
#pragma endregion

#pragma region REGTABPAGECLASS
	s_class = (WNDCLASSEX){
		.cbSize        = sizeof s_class,
		.lpszClassName = glpz_pagewndcl,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.lpfnWndProc   = (WNDPROC)&mbe_tabpage_int_wndproc
	};

	if (RegisterClassEx(&s_class) == FALSE)
		return MARBLE_EC_REGWNDCLASS;
#pragma endregion

	return MARBLE_EC_OK;
}


