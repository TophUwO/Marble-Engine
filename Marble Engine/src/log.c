#include <log.h>
#include <system.h>

#include <external/tfmt.h>


/* Log levels; only used internally. */
enum marble_log_level {
	MARBLE_LOGLVL_UNSPECIFIED = 0,

	MARBLE_LOGLVL_INFO,
	MARBLE_LOGLVL_DEBUG,
	MARBLE_LOGLVL_WARN,
	MARBLE_LOGLVL_ERROR,
	MARBLE_LOGLVL_FATAL
};


/*
 * Interal structure holding global state information
 * regarding the logging library.
 */
static struct {
	bool  isinit;    /* ready state */
	bool  isansicol; /* support for ANSI colors */
	FILE *p_handle;  /* file handle */
	char *pz_buffer; /* format buffer */

	/* Windows-only. */
#if (defined _WIN32)
	HANDLE mp_hdout; /* Windows console handle */
#endif
} gls_logctxt = { 0 };

#if (MB_LOG_LARGEBUF != false)
static size_t const      gl_bufsize   = 512 * 4;         /* large buffer size, in bytes */
#else
static size_t const      gl_bufsize   = 512;             /* normal buffer size, in bytes */
#endif
static char const *const glpz_mcreate = "w";             /* create fopen flag */
static char const *const glpz_mappend = "a";             /* append fopen flag */
static char const *const glpz_pattern = "%s %-5s %s%s";  /* log message pattern */
static char const *const glpz_tstamp  = "%T";            /* time-stamp pattern */

/*
 * Look-up table with log levels and their respective color code. Color
 * values are unused if colors are not used by defining 'MB_LOG_COLORS'
 * as 0 (false).
 * 
 * Reference: https://ss64.com/nt/syntax-ansi.html
 */
static struct {
	enum marble_log_lvl m_lvl; /* log level identifier */
	
	char const *mpz_str;       /* level string */
	char const *mpz_color;     /* ANSI color code */
	uint16_t    m_oldcol;      /* legacy Windows color value, used only on Windows. */
} const glsa_loglvlinfo[] = {
	{ MARBLE_LOGLVL_UNSPECIFIED, "n/a",   "\033[37m", 7 /* reset */     },

	{ MARBLE_LOGLVL_INFO,        "INFO",  "\033[37m", 7 /* reset */     },
	{ MARBLE_LOGLVL_DEBUG,       "DEBUG", "\033[90m", 8 /* dark grey */ },
	{ MARBLE_LOGLVL_WARN,        "WARN",  "\033[33m", 6 /* yellow */    },
	{ MARBLE_LOGLVL_ERROR,       "ERROR", "\033[31m", 4 /* red */       },
	{ MARBLE_LOGLVL_FATAL,       "FATAL", "\033[35m", 5 /* magenta */   }
};



/*
 * Attempts to enable ANSI escape sequences (Virtual Terminal)
 * in the current console. Note that almost all current terminal
 * emulators except for the Windows Console natively support ANSI colors,
 * however, beginning with Windows 10 Creators Update (Build: 15063), even
 * the default Windows Terminal supports VT100 sequences.
 * Furthermore, the function sets the codepage of the Windows Console to
 * UTF-8 (65001).
 * 
 * If OS build number is 15063 or later, we use VT100 sequences, otherwise
 * we just use good ol' "SetConsoleTextAttribute()".
 * 
 * Returns nothing.
 */
static void marble_log_internal_setupconsole(void) {
#if (defined _WIN32)
	typedef NTSTATUS (*RtlGetVersion_t)(_Out_ PRTL_OSVERSIONINFOW);

	/*
	 * Redirect stdout to our newly-allocated console.
	 * 
	 * This allows us to use standard C I/O functions
	 * such as printf() and friends.
	 */
#if (defined _WIN32) && ((defined _DEBUG) || (defined MB_DEVBUILD))
	if (AllocConsole()) {
		FILE *p_tmp = NULL;

		if (freopen_s(&p_tmp, "CONOUT$", "w", stdout) != 0)
			return;
	}
#endif

	/* Set Console codepage to UTF-8. */
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	gls_logctxt.isansicol = false;
#if (MB_LOG_COLORS != false)
	/* Get manifest-independent OS version. */
	HMODULE p_ntdll = LoadLibrary(TEXT("ntdll.dll"));
	if (p_ntdll == NULL)
		return;
	
	RtlGetVersion_t pfn_getver = (RtlGetVersion_t)GetProcAddress(p_ntdll, "RtlGetVersion");
	if (pfn_getver == NULL)
		goto lbl_END;

	RTL_OSVERSIONINFOW s_version = { 0 };
	(*pfn_getver)(&s_version);

	/* If build number is 15063 or later, enable ANSI colors. */
	if (s_version.dwBuildNumber >= 15063) {
		gls_logctxt.mp_hdout = GetStdHandle(STD_OUTPUT_HANDLE);
		if (gls_logctxt.mp_hdout == NULL)
			goto lbl_END;

		/* Get current console mode. */
		DWORD mode = 0;
		if (GetConsoleMode(gls_logctxt.mp_hdout, &mode) == false)
			goto lbl_END;

		/* Add VT100 flag. */
		mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		if (SetConsoleMode(gls_logctxt.mp_hdout, mode) == false)
			goto lbl_END;

		gls_logctxt.isansicol = true;
	}

lbl_END:
	FreeLibrary(p_ntdll);
#endif
#else
	gls_logctxt.isansicol = true;
#endif
}

/*
 * Sets the output color to the one specified for
 * the log level represented by **lvl**.
 * Use 'MARBLE_LOGLVL_UNSPECIFIED' to reset the color.
 * 
 * Returns nothing.
 */
static void marble_log_internal_setcolor(
	enum marble_log_level lvl /* log level */
) {
#if (MB_LOG_COLORS != false)
	if (gls_logctxt.isansicol == false) {
#if (defined _WIN32)
		SetConsoleTextAttribute(
			gls_logctxt.mp_hdout,
			glsa_loglvlinfo[lvl].m_oldcol
		);
#endif
		return;
	}

	printf(glsa_loglvlinfo[lvl].mpz_color);
#endif
}

/*
 * Formats a log message. Expects input data to be valid.
 * 
 * Returns nothing.
 */
static bool marble_log_internal_formatmessage(
	           bool isplain,
	           enum marble_log_level lvl,
	_In_opt_z_ char const *pz_function,
	_In_z_     char const *pz_fmt,
	_In_opt_   va_list p_args
) {
	size_t written = 0;
	if (isplain == true)
		goto lbl_FMTMSG;

	char a_tsbuf[16] = { 0 };
	tfmt_strftime(a_tsbuf, glpz_tstamp, sizeof a_tsbuf, NULL);

	written = (size_t)sprintf_s(
		gls_logctxt.pz_buffer,
		gl_bufsize,
		glpz_pattern,
		a_tsbuf,
		glsa_loglvlinfo[lvl].mpz_str,
		pz_function == NULL ? "" : pz_function,
		pz_function == NULL ? "" : "(): "
	);
	if (written == (size_t)(-1))
		return false;

lbl_FMTMSG:
	vsprintf_s(
		gls_logctxt.pz_buffer + written,
		gl_bufsize - written,
		pz_fmt,
		p_args
	);

	return true;
}

static void marble_log_internal_outputmessage(
	           bool isplain,
	           enum marble_log_level lvl,
	_In_opt_z_ char const *pz_function,
	_In_z_     char const *pz_fmt,
	_In_opt_   va_list p_args
) {
	if (marble_log_internal_formatmessage(
		isplain,
		lvl,
		pz_function,
		pz_fmt,
		p_args
	) == false)
		return;

	marble_log_internal_setcolor(lvl);
	puts(gls_logctxt.pz_buffer);
	marble_log_internal_setcolor(MARBLE_LOGLVL_UNSPECIFIED);

	if (gls_logctxt.p_handle != NULL) {
		fputs(gls_logctxt.pz_buffer, gls_logctxt.p_handle);

		/*
		 * Unlike "puts()", "fputs()" does not automatically append
		 * a newline ('\n') character after the string, so we have to
		 * append it manually.
		 */
		fputc('\n', gls_logctxt.p_handle);
	}
}

static void marble_log_internal_welcomemessage(void) {
	marble_log_plain(
		u8"*******************************************************************************\n"
		u8" Marble Engine (" MB_VERSIONSTR ", " MB_ARCHSTR ")\n"
		u8" (C) 2022 TophUwO <tophuwo01@gmail.com> All rights reserved.\n"
		u8"*******************************************************************************\n\n",
	NULL);
}


_Critical_ marble_ecode_t marble_log_init(
	_In_opt_z_ char const *pz_logfile
) { MB_ERRNO
	if (gls_logctxt.isinit == true)
		return MARBLE_EC_COMPSTATE;

	/*
	 * This logging library uses an internal buffer that
	 * is used to format the log message, afer which the
	 * buffer is written to stdout and, if present,
	 * the logfile.
	 * This buffer should be reasonably large.
	 */
	ecode = marble_system_alloc(
		gl_bufsize,
		true,
		false,
		&gls_logctxt.pz_buffer
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* If **pz_logfile** is not NULL and not empty, attempt to open the file. */
	if (pz_logfile != NULL && *pz_logfile != '\0') {
		errno_t ret = fopen_s(
			&gls_logctxt.p_handle,
			pz_logfile,
#if (MB_LOG_APPEND != false)
			glpz_mappend
#else
			glpz_mcreate
#endif
		);
		if (ret != 0) {
			ecode = MARBLE_EC_OPENFILE;

			goto lbl_END;
		}
	}

	gls_logctxt.isinit = true;

lbl_END:
	/* Restore the original state of the logging library. */
	if (ecode != MARBLE_EC_OK) {
		free(gls_logctxt.pz_buffer);

		gls_logctxt.pz_buffer = NULL;
	}

	/*
	 * Attempt to enable ANSI color sequences on a Windows
	 * terminal.
	 */
	marble_log_internal_setupconsole();
	/* Print a "start-up banner" */
#if (MB_LOG_NOLOGO == false)
	marble_log_internal_welcomemessage();
#endif
	return ecode;
}

void marble_log_uninit(void) {
	if (gls_logctxt.isinit == false)
		return;

	if (gls_logctxt.p_handle != NULL) {
		/*
		* Print some newlines so that we can distinguish "log sessions"
		* if we are appending logfiles instead of creating them every
		* time a session starts.
		*/
		fputs("\n\n\n\n\n\n\n\n", gls_logctxt.p_handle);

		fflush(gls_logctxt.p_handle);
		fclose(gls_logctxt.p_handle);

		gls_logctxt.p_handle = NULL;
	}

	free(gls_logctxt.pz_buffer);
	gls_logctxt.pz_buffer = NULL;

	gls_logctxt.isinit = false;
}


#pragma region LOG-OUTPUT_FUNCTIONS
void __cdecl marble_log_plain(
	_In_z_ char const *pz_fmt,
	...
) {
	if (gls_logctxt.isinit == false || pz_fmt == NULL || *pz_fmt == '\0')
		return;

	va_list p_args;
	va_start(p_args, pz_fmt);

	marble_log_internal_outputmessage(
		true,
		MARBLE_LOGLVL_UNSPECIFIED,
		NULL,
		pz_fmt,
		p_args
	);

	va_end(p_args);
}

void __cdecl marble_log_debug(
	_In_opt_z_ char const *pz_fn,
	_In_z_     char const *pz_fmt,
	...
) {
	if (gls_logctxt.isinit == false || pz_fmt == NULL || *pz_fmt == '\0')
		return;

	va_list p_args;
	va_start(p_args, pz_fmt);

	marble_log_internal_outputmessage(
		false,
		MARBLE_LOGLVL_DEBUG,
		pz_fn,
		pz_fmt,
		p_args
	);

	va_end(p_args);
}

void __cdecl marble_log_info(
	_In_opt_z_ char const *pz_fn,
	_In_z_     char const *pz_fmt,
	...
) {
	if (gls_logctxt.isinit == false || pz_fmt == NULL || *pz_fmt == '\0')
		return;

	va_list p_args;
	va_start(p_args, pz_fmt);

	marble_log_internal_outputmessage(
		false,
		MARBLE_LOGLVL_INFO,
		pz_fn,
		pz_fmt,
		p_args
	);

	va_end(p_args);
}

void __cdecl marble_log_warn(
	_In_opt_z_ char const *pz_fn,
	_In_z_     char const *pz_fmt,
	...
) {
	if (gls_logctxt.isinit == false || pz_fmt == NULL || *pz_fmt == '\0')
		return;

	va_list p_args;
	va_start(p_args, pz_fmt);

	marble_log_internal_outputmessage(
		false,
		MARBLE_LOGLVL_WARN,
		pz_fn,
		pz_fmt,
		p_args
	);

	va_end(p_args);
}

void __cdecl marble_log_error(
	_In_opt_z_ char const *pz_fn,
	_In_z_     char const *pz_fmt,
	...
) {
	if (gls_logctxt.isinit == false || pz_fmt == NULL || *pz_fmt == '\0')
		return;

	va_list p_args;
	va_start(p_args, pz_fmt);

	marble_log_internal_outputmessage(
		false,
		MARBLE_LOGLVL_ERROR,
		pz_fn,
		pz_fmt,
		p_args
	);

	va_end(p_args);
}

void __cdecl marble_log_fatal(
	_In_opt_z_ char const *pz_fn,
	_In_z_     char const *pz_fmt,
	...
) {
	if (gls_logctxt.isinit == false || pz_fmt == NULL || *pz_fmt == '\0')
		return;

	va_list p_args;
	va_start(p_args, pz_fmt);

	marble_log_internal_outputmessage(
		false,
		MARBLE_LOGLVL_FATAL,
		pz_fn,
		pz_fmt,
		p_args
	);

	va_end(p_args);
}
#pragma endregion


