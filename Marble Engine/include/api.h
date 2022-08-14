#pragma once

#pragma warning (disable: 5105) /* macro expansion producing UB */


#include <def.h>
#include <platform.h>

#include <windows.h>
#include <windowsx.h>
#include <wincodec.h>
#include <tchar.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#ifdef _DEBUG
	#include <crtdbg.h>
#endif

/*
 * Define error code variable only for
 * the engine itself.
 */
#if (defined MB_DYNAMIC_LIBRARY)
	#define MB_ERRNO marble_ecode_t ecode = MARBLE_EC_OK;
#endif


/*
 * Structure representing an integer point
 * in 2D-space.
 */
struct marble_pointi2d { int32_t m_x, m_y; };

/*
 * Structure representing integer extends in
 * 2D-space.
 */
struct marble_sizei2d { int32_t m_width, m_height; };


