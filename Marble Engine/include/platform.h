#pragma once


#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
	#define MB_PLATFORM_WINDOWS
	#if (defined(__GNUC__) || defined(__MINGW32__))
		#define MB_COMPILER_MINGW
	#elif (defined(_MSC_VER) && !defined(__INTEL_COMPILER))
		#define MB_COMPILER_MSVC
	#elif (defined(__INTEL_COMPILER))
		#define MB_COMPILER_INTEL
	#else
		#error Unknown compiler.
	#endif
#else
	#error Marble only targets Microsoft Windows for now.
#endif


