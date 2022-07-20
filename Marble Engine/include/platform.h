#pragma once

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
	#define MARBLE_PLATFORM_WINDOWS
	#if (defined(__GNUC__) || defined(__MINGW32__))
		#define MARBLE_COMPILER_MINGW
	#elif (defined(_MSC_VER) && !defined(__INTEL_COMPILER))
		#define MARBLE_COMPILER_MSVC
	#elif (defined(__INTEL_COMPILER))
		#define MARBLE_COMPILER_INTEL
	#else
		#error Unknown compiler.
	#endif
#else
	#error Marble only targets Microsoft Windows for now.
#endif


