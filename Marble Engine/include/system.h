#pragma once

#include <api.h>
#include <error.h>

#define _Maybe_out_z_ _When_(return == MARBLE_EC_OK, _Out_writes_z_(size))


extern void marble_application_raisefatalerror(marble_ecode_t ecode);


/*
 * Memory allocator
 *
 * Calls "malloc()" or "calloc()" depending on **needzeroed**. Can raise
 * a fatal application error (causing a forced shutdown) if **iscritical**
 * is TRUE. To free the memory allocated by this function, use 
 * standard "free()".
 *
 * Returns 0 on success, non-zero on failure. If the function fails,
 * the pointer pointed to by **pp_memptr** is guaranteed to be
 * initialized with NULL.
 */
_Critical_ marble_ecode_t inline marble_system_alloc(
	_In_              size_t size,     /* requested size of the memory block, in bytes */
	                  bool needzeroed, /* Should the requested memory be zeroed? */
	                  /*
	                   * If this parameter is set to TRUE, the allocation is considered
	                   * critical; if the allocation fails, the function raises a fatal
	                   * error, resulting in a forced shutdown. This parameter should really
	                   * only be set to true for allocating resources used by Marble's subsystems.
	                   */
	                  bool iscritical,
	/* 
	 * Pointer to receive pointer to newly allocated memory.
	 * This parameter must not be NULL. 
	 */
	_Init_(pp_memptr) void **pp_memptr
) {
	if (pp_memptr == NULL || size == 0) {
		if (pp_memptr != NULL)
			*pp_memptr = NULL;

		return MARBLE_EC_INTERNALPARAM;
	}

	/*
	 * If **needzeroed** is non-zero, we call "calloc()" instead.
	 * Calling "malloc()" + "memset()" to achieve the same
	 * is completely asinine.
	 */
	(void)(needzeroed != false
		? (*pp_memptr = calloc(1, size))
		: (*pp_memptr = malloc(size))
		);

	if (*pp_memptr == NULL) {
		if (iscritical == true)
			marble_application_raisefatalerror(MARBLE_EC_MEMALLOC);

		return MARBLE_EC_MEMALLOC;
	}

	printf("Allocator: Allocated memory of %i bytes, starting at address: 0x%p.\n", 
		(int)size,
		*pp_memptr
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


