#pragma once

#pragma warning (disable: 5105) /* macro expansion producing UB */


#include <uuid.h>

#include <windows.h>
#include <windowsx.h>
#include <wincodec.h>
#include <tchar.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
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
#if (defined MB_DYNAMIC_LIBRARY) || (defined MB_ECOSYSTEM)
	#define MB_ERRNO marble_ecode_t ecode = MARBLE_EC_OK;
#endif

/* Implement virtual inheritance using unions. */
#define MB_DEFSUBTYPE(id, base, def) union id { base _base; struct { base __##id##_metaobj__; struct def; }; };

/* Invalid pointer value; used as ret-val. */
#define MB_INVPTR ((void *)(-1))

/* Define common subtype functions. */
#define __MB_SUBTYPEFN__(subtype, fn) marble_##subtype##_##fn
#define MB_CREATEFN(subtype)          __MB_SUBTYPEFN__(subtype, create)
#define MB_DESTROYFN(subtype)         __MB_SUBTYPEFN__(subtype, destroy)
#define MB_VALIDATECRPSFN(subtype)    __MB_SUBTYPEFN__(subtype, validatecrps)
#define MB_LOADFN(subtype)            __MB_SUBTYPEFN__(subtype, load)

/* Cast a void* to any pointer type. */
#define MB_VOIDCAST(id, source, type)     type *id = (type *)source;
#define MB_FNCAST(id, source, ret, proto) ret (*id)proto = (ret (*)proto)source;


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


