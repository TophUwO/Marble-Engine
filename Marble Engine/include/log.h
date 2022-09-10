#pragma once

#include <error.h>


/*
 * Various flags (macros) that control and override the logging library's
 * default behavior if one wishes to do so. They can be either defined to
 * expand to non-zero (interpreted as true (1)), or zero (interpreted as
 * false (0)).
 */
#pragma region LOG-SETTINGS
/*
 * Should the library internally use a larger buffer to be able to format and print
 * longer error messages? Define this before including "log.h" to enable this
 * behavior.
 */
#if (!defined MB_LOG_LARGEBUF)
	#define MB_LOG_LARGEBUF (false)
#endif

/*
 * By default, if the **pz_logfile** member of "marble_log_init()" is not NULL and
 * not empty, the file is created and overwritten if it already existed. Defining
 * this flag before including "log.h" overrides this behavior and rather appends
 * the new "log session" without overwriting a possibly existing file.
 */
#if (!defined MB_LOG_APPEND)
	#define MB_LOG_APPEND (false)
#endif

/*
 * Use colored output for logging library? By default, the library uses colored
 * log messages.
 */
#if (!defined MB_LOG_COLORS)
	#define MB_LOG_COLORS (true)
#endif

/* Suppress start-up banner. */
#if (!defined MB_LOG_NOLOGO)
	#define MB_LOG_NOLOGO (false)
#endif
#pragma endregion


#if (defined MB_DYNAMIC_LIBRARY) || (defined MB_ECOSYSTEM)
/*
 * Initializes logger. If **pz_logfile** is not NULL,
 * the file is created and opened; an existing file is
 * overwritten.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ MB_API marble_ecode_t marble_log_init(
	_In_opt_z_ char const *pz_logfile
);

/*
 * Uninitializes logger. All resources are released, and the
 * logfile, if opened, is flushed and closed.
 * The logging libary is completely reset, allowing for a
 * clean reinitialization.
 * 
 * Returns nothing.
 */
MB_API void marble_log_uninit(void);
#endif


/*
 * Plain output functions; should not be used directly, better
 * use the macros underneath this region.
 * Note that passing invalid/an incorrect number of parameters
 * to the output functions causes undefined behavior.
 */
#pragma region LOG-OUTPUT_FUNCTIONS
/*
 * Outputs a plain message to the console (stdout) and, if
 * present, the logfile.
 * 
 * Returns nothing.
 */
MB_API void __cdecl marble_log_plain(
	_In_z_     char const *pz_fmt, /* format string */
	...                            /* arguments for **pz_fmt** */
);

/*
 * Outputs a debug message to the console (stdout) and, if
 * present, the logfile.
 * 
 * Returns nothing.
 */
MB_API void __cdecl marble_log_debug(
	_In_opt_z_ char const *pz_fn,  /* current function */
	_In_z_     char const *pz_fmt, /* format string */
	...                            /* arguments for **pz_fmt** */
);

/*
 * Outputs an info message to the console (stdout) and, if
 * present, the logfile.
 * 
 * Returns nothing.
 */
MB_API void __cdecl marble_log_info(
	_In_opt_z_ char const *pz_fn,  /* current function */
	_In_z_     char const *pz_fmt, /* format string */
	...                            /* arguments for **pz_fmt** */
);

/*
 * Outputs a warning message to the console (stdout) and, if
 * present, the logfile.
 * 
 * Returns nothing.
 */
MB_API void __cdecl marble_log_warn(
	_In_opt_z_ char const *pz_fn,  /* current function */
	_In_z_     char const *pz_fmt, /* format string */
	...                            /* arguments for **pz_fmt** */
);

/*
 * Outputs an error message to the console (stdout) and, if
 * present, the logfile.
 * 
 * Returns nothing.
 */
MB_API void __cdecl marble_log_error(
	_In_opt_z_ char const *pz_fn,  /* current function */
	_In_z_     char const *pz_fmt, /* format string */
	...                            /* arguments for **pz_fmt** */
);

/*
 * Outputs a fatal error message to the console (stdout) and, if
 * present, the logfile.
 * 
 * Returns nothing.
 */
MB_API void __cdecl marble_log_fatal(
	_In_opt_z_ char const *pz_fn,  /* current function */
	_In_z_     char const *pz_fmt, /* format string */
	...                            /* arguments for **pz_fmt** */
);
#pragma endregion


/*
 * Macros to wrap the compulsory __func__ into a macro
 * passing the parameter automatically.
 */
#define MB_LOG_PLAIN(fmt, ...) marble_log_plain(fmt, __VA_ARGS__)
#define MB_LOG_DEBUG(fmt, ...) marble_log_debug(__func__, fmt, __VA_ARGS__)
#define MB_LOG_INFO(fmt, ...)  marble_log_info(__func__, fmt, __VA_ARGS__)
#define MB_LOG_WARN(fmt, ...)  marble_log_warn(__func__, fmt, __VA_ARGS__)
#define MB_LOG_ERROR(fmt, ...) marble_log_error(__func__, fmt, __VA_ARGS__)
#define MB_LOG_FATAL(fmt, ...) marble_log_fatal(__func__, fmt, __VA_ARGS__)


