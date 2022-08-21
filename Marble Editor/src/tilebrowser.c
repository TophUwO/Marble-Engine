#include <editor.h>

#define MB_WM_INITRES (WM_USER + 1)


static TCHAR const *const glpz_wndclassname = TEXT("marble_editor_tbrowser");
static int const gl_ctlwidth = 150;
static bool gl_regwndclass = false;

static LRESULT CALLBACK marble_tilebrowser_internal_wndproc(
	_In_ HWND p_hwnd,
	_In_ UINT msg,
	     WPARAM wparam,
	     LPARAM lparam
) {
	HDC p_hdc;
	PAINTSTRUCT s_ps;
	struct marble_tilebrowser *ps_tbrowser = (struct marble_tilebrowser *)GetWindowLongPtr(p_hwnd, GWLP_USERDATA);

	switch (msg) {
		case WM_CREATE:
			SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lparam)->lpCreateParams);

			SendMessage(p_hwnd, MB_WM_INITRES, 0, 0);
			return 0;
		case MB_WM_INITRES:
			ps_tbrowser->p_hbkgnd = CreateSolidBrush(RGB(0, 0, 0));

			return 0;
		case WM_PAINT:
			p_hdc = BeginPaint(p_hwnd, &s_ps);

			FillRect(p_hdc, &s_ps.rcPaint, ps_tbrowser->p_hbkgnd);

			EndPaint(p_hwnd, &s_ps);
			return 0;
		case WM_DESTROY:
			DeleteObject(ps_tbrowser->p_hbkgnd);

			return 0;
	}

	return DefWindowProc(p_hwnd, msg, wparam, lparam);
}


marble_ecode_t marble_tilebrowser_create(
	struct marble_tilebrowser **pps_tbrowser
) {
	if (gl_regwndclass == false) {
		WNDCLASSEX s_class = {
			.cbSize        = sizeof s_class,
			.lpfnWndProc   = (WNDPROC)&marble_tilebrowser_internal_wndproc,
			.lpszClassName = glpz_wndclassname,
			.hInstance     = gls_editorapp.mp_hinst
		};
		if (RegisterClassEx(&s_class) == false)
			return MARBLE_EC_REGWNDCLASS;

		gl_regwndclass = true;
	}

	*pps_tbrowser = malloc(sizeof **pps_tbrowser);
	if (*pps_tbrowser == NULL)
		return MARBLE_EC_MEMALLOC;

	RECT s_rect;
	GetClientRect(gls_editorapp.mp_hwnd, &s_rect);

	(*pps_tbrowser)->mp_handle = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		glpz_wndclassname,
		TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		0,
		0,
		gl_ctlwidth,
		s_rect.bottom,
		gls_editorapp.mp_hwnd,
		NULL,
		gls_editorapp.mp_hinst,
		*pps_tbrowser
	);
	if ((*pps_tbrowser)->mp_handle == NULL) {
		marble_tilebrowser_destroy(pps_tbrowser);

		return MARBLE_EC_CREATEWND;
	}

	return MARBLE_EC_OK;
}

void marble_tilebrowser_destroy(
	struct marble_tilebrowser **pps_tbrowser
) {
	free(*pps_tbrowser);
	*pps_tbrowser = NULL;
}


void marble_tilebrowser_resize(
	struct marble_tilebrowser *ps_tbrowser,
	int nwidth,
	int nheight
) {
	if (ps_tbrowser == NULL)
		return;

	SetWindowPos(ps_tbrowser->mp_handle, NULL, 0, 0, gl_ctlwidth, nheight, SWP_NOMOVE | SWP_NOACTIVATE);
}


