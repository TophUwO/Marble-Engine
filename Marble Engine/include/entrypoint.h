#pragma once

#include <marble.h>


INT APIENTRY
WinMain(
	_In_ HINSTANCE p_inst,
	_In_opt_ HINSTANCE p_previnst,
	_In_ PSTR pz_cmdline,
	_In_ int showcmd
) {
	MB_API marble_ecode_t __cdecl marble_application_init(
		HINSTANCE,
		PSTR,
		marble_ecode_t (MB_CALLBACK *)(
			char const *,
			struct marble_app_settings *
		),
		marble_ecode_t (MB_CALLBACK *)(char const *)
	);
	MB_API marble_ecode_t __cdecl marble_application_run(void);
	/*
	 * This function runs user-application initialization AFTER Marble
	 * successfully finished initializing itself.
	 * This function has to be defined by the user
	 * application, or else the code will not compile.
	 */
	extern marble_ecode_t MB_CALLBACK marble_callback_userinit(
		char const *pz_cmdline /* command-line arguments */
	);
	/*
	 * This function runs user-application initialization BEFORE Marble will
	 * initialize itself. This function should really only be used to fill the
	 * **p_settings** structure.
	 * This function has to be defined by the user
	 * application, or else the code will not compile.
	 */
	extern marble_ecode_t MB_CALLBACK marble_callback_submitsettings(
		char const *pz_cmdline, /* command-line arguments */
		/*
		 * Pointer to a "marble_app_settings" structure which will
		 * receive user-determined app settings. These will then
		 * be used to initialize Marble.
		 */
		struct marble_app_settings *ps_settings
	);

	/* Run any necessary initialization. */
	marble_application_init(
		p_inst,                          /* application instance */
		pz_cmdline,                      /* command-line arguments */
		&marble_callback_submitsettings, /* initialization BEFORE Marble */
		&marble_callback_userinit        /* initialization AFTER Marble */
	);

	return marble_application_run(); /* Start the application. */
}


