#pragma once


#ifdef MB_DYNAMIC_LIBRARY
	#define MB_API extern __declspec(dllexport)
#else
	#define MB_API extern __declspec(dllimport)
#endif
#define MB_CALLBACK  __cdecl

/*
 * Shorten some very long SAL annotations, improving
 * readability. Also used for combining annotations in a more
 * readable and shorter form.
 */
#if (defined MB_DYNAMIC_LIBRARY) || (defined MB_ECOSYSTEM)
	/*
	 * Used for functions that allocate and initialize
	 * an "object"; these functions are usually named
	 * "*_create()". This annotation is used with a
	 * function parameter.
	 */
	#define _Init_(param)   _Post_satisfies_(*param != NULL) _On_failure_(_Post_satisfies_(param != NULL ? (*param == NULL) : 1))
	/*
	 * Used for functions that destroy an "object" and
	 * release all of its resources it occupied. These
	 * functions act as counterparts to "*_create()"
	 * and are appropriately named "*_destroy()".
	 * This annotation is used with a function parameter.
	 */
	#define _Uninit_(param) _Post_satisfies_(*param == NULL)
	/*
	 * Denotes functions that return a value of type "marble_ecode_t"
	 * what exact value denotes a successful completion. This
	 * annotation is used with a function.
	 */
	#define _Success_ok_    _Success_(return == MARBLE_EC_OK)
	/*
	 * A combination of '_Success_ok_' and an annotation
	 * throwing a warning if the caller does not inspect
	 * the function's return value. Used for "*_create()"
	 * functions, for instance. This annotation is used
	 * with a function.
	 */
	#define _Critical_      _Check_return_ _Success_ok_
	/*
	 * Denoting that a parameter has to be of **size** bytes
	 * at the very least. This annotation is used with a
	 * function parameter.
	 */
	#define _Size_(size)    _Out_writes_bytes_(size)
	/*
	 * Just shortening a long annotation. his annotation is
	 * used with a function parameter.
	 */
	#define _Maybe_valid_   _Outptr_result_maybenull_
	/*
	 * Writes data if function succeeds, leaves the buffer unchanged on failure. 
	 */
	#define _Maybe_out_     _When_(return == MARBLE_EC_OK, _Out_)
#endif


/*
 * Version number
 * 
 * Used internally; can be used by a user application
 * to check for a specific (min-) version. Updated solely by
 * the maintainer of this software.
 * 
 * 0 = indev > development version; first release version is 1.0.
 */
#define MB_VERSION     ((uint32_t)(0))
#define MB_VERSIONSTR  "indev"
/*
 * Maximum number of characters in an ID string (incl.
 * NUL-terminator). Bytes outside of this boundary are ignored,
 * and a NUL-terminator is automatically inserted at position
 * MB_STRINGIDMAX - 1.
 */
#define MB_STRINGIDMAX ((size_t)(32))
/*
 * Numeric flag instructing the receiving end to use 
 * default (sometimes system-defined) values.
 */
#define MB_DEFAULT     (-1)
/*
 * String macro representing the current build config.
 * Used by the logging library.
 */
#if (defined _DEBUG)
	#define MB_BUILDCONFIG "Release"
#elif (defined MB_DEVBUILD)
	#define MB_BUILDCONFIG "Development Build"
#else
	#define MB_BUILDCONFIG "Debug"
#endif
/*
 * String representation of target architecture (n bits).
 */
#if (defined _WIN64)
	#define MB_ARCHSTR "x86-64"
#else
	#define MB_ARCHSTR "x86"
#endif


#if (defined MB_DYNAMIC_LIBRARY) || (defined MB_ECOSYSTEM)
	/*
	 * Checks whether a specific numeric value is 
	 * in the given range (bounds not exclusive).
	 */
	#define MB_INRANGE_EXCL(n, lo, hi) ((bool)(n > lo && n < hi))
	 /*
	 * Checks whether a specific numeric value is 
	 * in the given range (bounds not exclusive).
	 */
	#define MB_INRANGE_INCL(n, lo, hi) ((bool)(n >= lo && n <= hi))
	/*
	 * Shorten __func__, __LINE__ parameters. 
	 */
	#define MB_CALLER_INFO __func__, (size_t)(__LINE__)
	/*
	 * Shorten char const * pointer validation.
	 */
	#define MB_ISINVSTR(str) str == NULL || *str == '\0'
#endif


