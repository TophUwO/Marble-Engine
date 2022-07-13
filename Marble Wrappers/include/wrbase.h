#pragma once

#ifdef __cplusplus
	#define _WR_EXTERN_C_ extern "C" __declspec(dllexport)
#else
	#define _WR_EXTERN_C_ extern __declspec(dllimport)
#endif


