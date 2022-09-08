#include <application.h>


uint32_t gl_hashseed = 0;
uint64_t gl_pfreq = 0;
struct marble_application gls_app = { NULL };


#pragma region ASSETMAN
extern void marble_asset_destroy(
	_Uninit_(pps_asset) struct marble_asset **pps_asset
);


/* 
 * Uninitializes asset manager component.
 *
 * Returns nothing.
 */
static void marble_application_internal_uninitassetman(void) {
	if (gls_app.ms_assets.m_isinit == false)
		return;

	gls_app.ms_assets.m_isinit = false;

	/*
	 * Destroy asset registry.
	 * Destroying the registry automatically calls
	 * "marble_asset_internal_destroy()" on the 
	 * contents.
	 */
	marble_util_htable_destroy(&gls_app.ms_assets.mps_table);
}

/*
 * Initializes asset manager component.
 * 
 * Returns 0 on success, non-zero on error.
 */
_Critical_ static marble_ecode_t marble_application_internal_initassetman(void) { MB_ERRNO
	/* 
	* If asset manager is already initialized, block further
	* attempts to (re-)initialize. 
	*/
	if (gls_app.ms_assets.m_isinit == true)
		return MARBLE_EC_COMPSTATE;
	
	/* Create asset registry. */
	ecode = marble_util_htable_create(
		128,
		(void (*)(void **))&marble_asset_destroy,
		&gls_app.ms_assets.mps_table
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_CLEANUP;
	
	/* Change component state to "initialized" (= active). */
	gls_app.ms_assets.m_isinit = true;
	
lbl_CLEANUP:
	if (ecode != MARBLE_EC_OK)
		marble_application_internal_uninitassetman();
	
	return ecode;
}
#pragma endregion


#pragma region LAYERSTACK
extern void marble_layer_destroy(
	_Uninit_(pps_layer) struct marble_layer **pps_layer
);


/*
 * Uninitializes layer stack.
 * This will cause all layers to be popped (calling their
 * "onpop" callbacks, and then destroyed.
 * 
 * Returns nothing.
 */
static void marble_application_internal_uninitlayerstack(void) {
	if (gls_app.ms_layerstack.m_isinit == false)
		return;

	/*
	 * Destroy layers manually instead of letting the vector do it. This allows
	 * for better flexibility in case we do want to destroy in a specific order
	 * or we want to treat layers differently.
	 */
	for (size_t i = 0; i < gls_app.ms_layerstack.mps_vec->m_size; i++) {
		struct marble_layer *ps_layer = marble_util_vec_get(gls_app.ms_layerstack.mps_vec, i);

		/*
		 * Even though the layer is technically still pushed to the
		 * layerstack, we set the "ispushed" flag to false in order
		 * to prevent "marble_layer_destroy()" from popping the layer.
		 *
		 * "marble_layer_destroy()" should only pop the layer if the
		 * layerstack itself is to be destroyed.
		 */
		ps_layer->m_ispushed = false;

		/* Destroy the layer. */
		marble_layer_destroy(&ps_layer);
	}

	marble_util_vec_destroy(&gls_app.ms_layerstack.mps_vec);
	gls_app.ms_layerstack.m_isinit = false;
}

/*
 * Initializes layer stack.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ static marble_ecode_t marble_application_internal_initlayerstack_impl(void) { MB_ERRNO
	if (gls_app.ms_layerstack.m_isinit == true)
		return MARBLE_EC_COMPSTATE;
	
	ecode = marble_util_vec_create(
		0,
		NULL,
		&gls_app.ms_layerstack.mps_vec
	);
	if (ecode != MARBLE_EC_OK) {
		marble_application_internal_uninitlayerstack();

		return ecode;
	}
	
	gls_app.ms_layerstack.m_lastlayer = 0;
	gls_app.ms_layerstack.m_isinit    = true;

	return ecode;
}
#pragma endregion


/*
 * These functions just wrap all of the component initialization
 * functions.
 * This allows passing an error code variable that will be 
 * set whenever an error happens. As soon as this variable gets set,
 * no more components will be initialized and Marble will
 * perform a forced shutdown.
 */
#pragma region initialization functions
static void marble_application_internal_initlog(
	_Inout_ marble_ecode_t *p_ecode /* pointer to error code variable */
) {
	if (*p_ecode != MARBLE_EC_OK)
		return;

	*p_ecode = marble_log_init("log.txt");
	if (*p_ecode != MARBLE_EC_OK)
		marble_application_raisefatalerror(*p_ecode);

	marble_log_info(NULL, "init: log");
}

static void marble_application_internal_inithpc(
	_Inout_ marble_ecode_t *p_ecode /* pointer to error code variable */
) {
	if (*p_ecode != MARBLE_EC_OK)
		return;
		
	marble_log_info(NULL, "init: high-precision clock");

	/*
	 * Check whether an HPC is present in the system; should
	 * not fail on computers that are still relevant nowadays.
	 */
	if (!QueryPerformanceFrequency((LARGE_INTEGER *)&gl_pfreq))
		marble_application_raisefatalerror(*p_ecode = MARBLE_EC_INITHPC);
}

static void marble_application_internal_initcom(
	_Inout_ marble_ecode_t *p_ecode /* pointer to error code variable */
) { MB_ERRNO
	if (*p_ecode != MARBLE_EC_OK)
		return;

	marble_log_info(NULL, "init: component object model (COM)");

	/*
	 * Initialize COM. If "CoInitializeEx()" returns non-zero,
	 * this generally indicates that an error occurred. However,
	 * if the user decides to init COM in "marble_callback_submitsettings()"
	 * which is unsupported behavior but still has to be accounted for,
	 * then another call to "CoInitializeEx()" returns S_FALSE. In this case
	 * we do not want to quit the app.
	 */
	ecode = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (ecode && ecode != S_FALSE)
		marble_application_raisefatalerror(*p_ecode = MARBLE_EC_INITCOM);
}

static void marble_application_internal_initstate(
	_Inout_ marble_ecode_t *p_ecode, /* pointer to error code variable */
	_In_    HINSTANCE p_inst         /* application instance */
) {
	if (*p_ecode != MARBLE_EC_OK)
		return;

	marble_log_info(NULL, "init: application state");

	marble_application_setstate(
		false,
		0,
		MARBLE_APPSTATE_INIT
	);
	gls_app.mp_inst = p_inst;
}

static void marble_application_internal_createwindow(
	_Inout_ marble_ecode_t *p_ecode,                      /* pointer to error code variable */
	_In_    struct marble_app_settings const *ps_settings /* pointer to user-specified settings */
) {
	if (*p_ecode != MARBLE_EC_OK)
		return;

	/* Initialize window system. */
	marble_log_info(NULL, "init: window system");

	*p_ecode = marble_windowsys_init();
	if (*p_ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* Initialize window. */
	marble_log_info(NULL, "init: window");

	*p_ecode = marble_window_create(
		ps_settings,
		true,
		&gls_app.ps_wnd
	);

lbl_END:
	if (*p_ecode != MARBLE_EC_OK)
		marble_application_raisefatalerror(*p_ecode);
}

static void marble_application_internal_initlayerstack(
	_Inout_ marble_ecode_t *p_ecode /* pointer to error code variable */
) {
	if (*p_ecode != MARBLE_EC_OK)
		return;

	marble_log_info(NULL, "init: layer stack");

	*p_ecode = marble_application_internal_initlayerstack_impl();
	if (*p_ecode != MARBLE_EC_OK)
		marble_application_raisefatalerror(*p_ecode);
}

static void marble_application_internal_initassetmanager(
	_Inout_ marble_ecode_t *p_ecode /* pointer to error code variable */
) {
	if (*p_ecode != MARBLE_EC_OK)
		return;

	marble_log_info(NULL, "init: asset manager");

	*p_ecode = marble_application_internal_initassetman();
	if (*p_ecode != MARBLE_EC_OK)
		marble_application_raisefatalerror(*p_ecode);
}

static void marble_application_internal_douserinit(
	_Inout_ marble_ecode_t *p_ecode,
	_In_z_  char const *pz_cmdline,
	_In_    marble_ecode_t (MB_CALLBACK *cb_userinit)(_In_z_ char const *)
) {
	if (*p_ecode != MARBLE_EC_OK)
		return;

	marble_log_info(NULL, "init: user application");

	/*
	 * When user initialization fails, this is also considered
	 * a fatal error. However, the user can still choose to return
	 * 0 even if something failed, in which case the application
	 * will not forcefully quit.
	 */
	*p_ecode = cb_userinit(pz_cmdline);
	if (*p_ecode != MARBLE_EC_OK)
		marble_application_raisefatalerror(*p_ecode);
}
#pragma endregion


/*
 * Updates a window and renders content to it.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ static marble_ecode_t marble_application_internal_updateandrender(
	struct marble_window *ps_window, /* window to update and render */
	float frametime                  /* time it took to render and present last frame */
) {
	marble_renderer_begindraw(ps_window->mps_renderer);
	marble_renderer_clear(ps_window->mps_renderer, 0.0f, 0.0f, 0.0f, 1.0f);

	/* just for debugging purposes */
#if (defined _DEBUG) || (defined MB_DEVBUILD)
	D2D1_RECT_F s_rect = {
		ps_window->mps_renderer->m_orix,
		ps_window->mps_renderer->m_oriy,
		ps_window->mps_renderer->m_orix + ps_window->ms_data.ms_ext.ms_client.m_width,
		ps_window->mps_renderer->m_oriy + ps_window->ms_data.ms_ext.ms_client.m_height
	};
	D2D1_BRUSH_PROPERTIES s_brushprops = {
		.opacity = 1.0f
	};
	D2D1_COLOR_F s_color = { 1.0f, 1.0f, 1.0f, 1.0f };
	ID2D1SolidColorBrush *p_brush = NULL;
	D2DWr_DeviceContext_CreateSolidColorBrush(ps_window->mps_renderer->ms_d2drenderer.mp_devicectxt, &s_color, &s_brushprops, &p_brush);
	D2DWr_DeviceContext_FillRectangle(ps_window->mps_renderer->ms_d2drenderer.mp_devicectxt, &s_rect, (ID2D1Brush *)p_brush);

	D2DWr_SolidColorBrush_Release(p_brush);
#endif

	for (size_t i = 0; i < gls_app.ms_layerstack.mps_vec->m_size; i++) {
		struct marble_layer *ps_layer = marble_util_vec_get(gls_app.ms_layerstack.mps_vec, i);

		if (ps_layer->m_isenabled)
			(*ps_layer->ms_cbs.cb_onupdate)(ps_layer->m_id, frametime, ps_layer->mp_userdata);
	}

	marble_renderer_enddraw(ps_window->mps_renderer);
	marble_window_update(ps_window, frametime);

	return marble_renderer_present(&ps_window->mps_renderer);
}

/*
 * Global clean-up function. Will always be called
 * when Marble quits.
 * 
 * Passes through the given error code to the host environment.
 * Returns passed error code.
 */
_Success_ok_ static marble_ecode_t marble_application_internal_cleanup(
	marble_ecode_t ecode /* error code to return to system */
) {
	marble_log_info(NULL, "System shutdown ...");

	marble_window_destroy(&gls_app.ps_wnd);

	marble_application_internal_uninitlayerstack();
	marble_application_internal_uninitassetman();
	marble_log_uninit();
	marble_windowsys_uninit();
	CoUninitialize();

#if (defined _DEBUG)
	_CrtDumpMemoryLeaks();
#endif

	return ecode;
}


void marble_application_setstate(
	bool isfatal,
	int param,
	enum marble_app_stateid newid
) {
	gls_app.ms_state = (struct marble_app_state){
		.m_isfatal = isfatal,
		.m_id      = newid,
		.m_param   = param
	};
}


/*
 * The functions underneath this comment do not have any annotations, as they
 * can be called from outside the application, and I do not want to enfore the
 * use of annotations outside of the engine itself.
 */

MB_API marble_ecode_t __cdecl marble_application_init(
	HINSTANCE p_inst,
	PSTR pz_cmdline,
	void (MB_CALLBACK *cb_usersubmitsettings)(char const *, struct marble_app_settings *),
	marble_ecode_t (MB_CALLBACK *cb_userinit)(char const *)
) { MB_ERRNO
	gl_hashseed         = (uint32_t)time(NULL);
	gls_app.mp_mainthrd  = GetCurrentThread();
	gls_app.m_hasmainwnd = false;

	/* Get submitted user settings. */
	struct marble_app_settings s_settings = { 0 };
	cb_usersubmitsettings(pz_cmdline, &s_settings);

	marble_application_internal_initlog(&ecode);
	marble_application_internal_initstate(&ecode, p_inst);
	marble_application_internal_inithpc(&ecode);
	marble_application_internal_initcom(&ecode);
	marble_application_internal_createwindow(&ecode, &s_settings);
	marble_application_internal_initlayerstack(&ecode);
	marble_application_internal_initassetmanager(&ecode);

	/* Rrun user initialization. */
	marble_application_internal_douserinit(&ecode, pz_cmdline, cb_userinit);

	/* At last, present the window. */
	if (!ecode) {
		UpdateWindow(gls_app.ps_wnd->mp_handle);

		ShowWindow(gls_app.ps_wnd->mp_handle, SW_SHOWNORMAL);
	}

	return MARBLE_EC_OK;
}

MB_API marble_ecode_t __cdecl marble_application_run(void) {
	/*
	 * If a fatal error occurred during initialization,
	 * do not even start the main loop.
	 */
	if (gls_app.ms_state.m_isfatal == true)
		goto lbl_CLEANUP;

	/* update application state now that the main loop is about to start */
	marble_application_setstate(
		false,
		MARBLE_EC_OK,
		MARBLE_APPSTATE_RUNNING
	);

	MB_LOG_PLAIN("-------------------------------------------------------------------------------", 0);
 
	while (true) {
		MSG s_msg;
		uint64_t time;

		QueryPerformanceCounter((LARGE_INTEGER *)&time);
		float frametime = (time - gls_app.m_perfcounter) / (float)gl_pfreq;
		gls_app.m_perfcounter = time;

		while (PeekMessage(&s_msg, NULL, 0, 0, PM_REMOVE) > 0) {
			if (s_msg.message == WM_QUIT || s_msg.message == MB_WM_FATAL)
				goto lbl_CLEANUP;

			TranslateMessage(&s_msg);
			DispatchMessage(&s_msg);
		}

		if (gls_app.ps_wnd->ms_data.m_isminimized == false)
			marble_application_internal_updateandrender(gls_app.ps_wnd, frametime);
	}
	
lbl_CLEANUP:
	MB_LOG_PLAIN("-------------------------------------------------------------------------------", 0);

	if (gls_app.ms_state.m_isfatal == true) {
		TCHAR a_buf[1024] = { 0 };

		_stprintf_s(
			a_buf,
			1024,
			TEXT("Application has to abruptly quit due to the occurence of\n")
			TEXT("a fatal error:\n\n")
			TEXT("Code:\t%i\n")
			TEXT("String:\t%S\n")
			TEXT("Desc:\t%S\n"),
			gls_app.ms_state.m_param,
			marble_error_getstr(gls_app.ms_state.m_param),
			marble_error_getdesc(gls_app.ms_state.m_param)
		);
		MessageBox(NULL, a_buf, TEXT("Fatal Error"), MB_ICONERROR | MB_OK);
	}

	return marble_application_internal_cleanup(gls_app.ms_state.m_param);
}


