#pragma once

#include <api.h>
#include <error.h>
#include <log.h>

#define _Maybe_out_z_ _When_(return == MARBLE_EC_OK, _Out_writes_z_(size))


extern void marble_application_raisefatalerror(marble_ecode_t ecode);


/*
 * Memory allocator
 *
 * Calls "malloc()" or "calloc()" depending on **needzeroed**. Can raise
 * a fatal application error (causing a forced shutdown) if **iscritical**
 * is true. To free the memory allocated by this function, use 
 * standard "free()".
 *
 * Returns 0 on success, non-zero on failure. If the function fails,
 * the pointer pointed to by **pp_mpt** is guaranteed to be
 * initialized with NULL.
 */
_Critical_ marble_ecode_t inline marble_system_alloc(
	_In_opt_z_     char const *pz_fn, /* caller function */
	_In_opt_       size_t line,       /* line in caller file */
	_In_           size_t size,       /* requested size of the memory block, in bytes */
	               bool needzeroed,   /* Should the requested memory be zeroed? */
	               /*
	                * If this parameter is set to true, the allocation is considered
	                * critical; if the allocation fails, the function raises a fatal
	                * error, resulting in a forced shutdown. This parameter should really
	                * only be set to true for allocating resources used by Marble's subsystems.
	                */
	               bool iscritical,
	/* 
	 * Pointer to receive pointer to newly allocated memory.
	 * This parameter must not be NULL. 
	 */
	_Init_(pp_mpt) void **pp_mpt
) {
	if (pp_mpt == NULL || size == 0) {
		if (pp_mpt != NULL)
			*pp_mpt = NULL;

		return MARBLE_EC_INTERNALPARAM;
	}

	/*
	 * If **needzeroed** is non-zero, we call "calloc()" instead.
	 * Calling "malloc()" + "memset()" to achieve the same
	 * is completely asinine.
	 */
	(void)(needzeroed != false
		? (*pp_mpt = calloc(1, size))
		: (*pp_mpt = malloc(size))
		);

	if (*pp_mpt == NULL) {
#if (defined MB_DYNAMIC_LIBRARY)
		if (iscritical == true)
			marble_application_raisefatalerror(MARBLE_EC_MEMALLOC);
#endif

		return MARBLE_EC_MEMALLOC;
	}

	MB_LOG_DEBUG(
		"Allocator: sz=%zu [ori: %s (l: %zu)]",
		(int)size,
		pz_fn,
		line
	);

	return MARBLE_EC_OK;
}

/*
 * Just a wrapper for strcpy_s to avoid different 
 * usage of functions across the application. Allows
 * **pz_src** to be optional, so no extra validation
 * is needed by the caller.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_system_cpystr(
	_Maybe_out_z_ char *restrict pz_dest,      /* destination memory */
	_In_opt_z_    char const *restrict pz_src, /* source memory */
	_In_          size_t size				   /* destination size, in bytes */
) {
	if (pz_dest == NULL || pz_src == NULL)
		return MARBLE_EC_INTERNALPARAM;

	errno_t ret = strcpy_s(pz_dest, size, pz_src);
	if (ret != 0)
		return MARBLE_EC_MEMCPY;

	return MARBLE_EC_OK;
}


