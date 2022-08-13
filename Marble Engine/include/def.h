#pragma once


#ifdef MB_DYNAMIC_LIBRARY
	#define MB_API extern __declspec(dllexport)
#else
	#define MB_API extern __declspec(dllimport)
#endif
#define MB_CALLBACK  __cdecl

/*
 * Shorten some very long SAL annotations, improving
 * readability.
 */
#if (defined MB_DYNAMIC_LIBRARY)
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

#if (defined MB_DYNAMIC_LIBRARY)
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
#endif


