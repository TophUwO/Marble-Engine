#pragma once

#include <api.h>


MB_BEGIN_HEADER

#if (defined __cplusplus)
    #pragma warning (disable: 26812) /* unscoped "enum"; prefer enum class */
#endif


/*
 * Error codes
 * 
 * Any function that returns an int
 * can return any of these values. Their
 * purpose becomes clear when looking at
 * their description strings.
 * 
 * To make the code a little bit shorter,
 * error codes are passed along in the system
 * as ints.
 */
typedef enum marble_ecode {
	MARBLE_EC_OK = 0,
	MARBLE_EC_UNKNOWN,

	MARBLE_EC_UNIMPLFEATURE, 
	MARBLE_EC_MEMALLOC,
	MARBLE_EC_MEMREALLOC,
	MARBLE_EC_MEMCPY,
	MARBLE_EC_PARAM,
	MARBLE_EC_INTERNALPARAM,
	MARBLE_EC_OPENFILE,
	MARBLE_EC_READFILE,
	MARBLE_EC_FSEEK,
	MARBLE_EC_REGWNDCLASS,
	MARBLE_EC_INITCOM,
	MARBLE_EC_CREATEWND,
    MARBLE_EC_INITUTILS,
	MARBLE_EC_COMPSTATE,
	MARBLE_EC_CREATED2DFAC,
	MARBLE_EC_CREATED3D11DEV,
	MARBLE_EC_GETDXGIDEV,
	MARBLE_EC_CREATED2DDEV,
	MARBLE_EC_CREATED2DDEVCTXT,
	MARBLE_EC_GETDXGIADAPTER,
	MARBLE_EC_GETDXGIFAC,
	MARBLE_EC_CREATESWAPCHAIN,
	MARBLE_EC_GETDXGIBACKBUFFER,
	MARBLE_EC_CREATED2DBMPFROMDXGISURFACE,
	MARBLE_EC_RESIZERENDERER,
	MARBLE_EC_ASSETTYPE,
	MARBLE_EC_LAYERALREADYPUSHED,
	MARBLE_EC_RENDERAPI,
    MARBLE_EC_CREATERENDERER,
	MARBLE_EC_NOTFOUND,
	MARBLE_EC_GETMONITORINFO,
	MARBLE_EC_CALCWNDSIZE,
	MARBLE_EC_QUERYWINDOWRECT,
    MARBLE_EC_FILESIZE,
    MARBLE_EC_CREATEPARAMS,
    MARBLE_EC_REGTMPASSET,
    MARBLE_EC_LEVELSIZE,
    MARBLE_EC_OUTOFRANGE,
    MARBLE_EC_PCONTRADICTION,
    MARBLE_EC_LAYERADDR,
    MARBLE_EC_CHUNKADDR,
    MARBLE_EC_ALREADYDEP,
    MARBLE_EC_NOTDEP,
    MARBLE_EC_INITLOCK,

	__MARBLE_NUMERRORCODES__ /* for internal use */
} marble_ecode_t;


/*
 * Gets string for error code.
 * 
 * If the error code is invalid, the function
 * returns the error code string for
 * MARBLE_EC_UNKNOWN.
 * 
 * Returns error code string.
 */
MB_API char const *const marble_error_getstr(
    _In_range_(MARBLE_EC_OK, __MARBLE_NUMERRORCODES__ - 1)
    marble_ecode_t ecode /* error code to get string representation for */
);

/*
 * Gets description string for error code.
 * 
 * If the error code is invalid, the function
 * returns the description string for
 * MARBLE_EC_UNKNOWN.
 * An error description is a brief message that
 * is supposed to give hints on what went wrong
 * and possible fixes.
 * 
 * Returns error code description string.
 */
MB_API char const *const marble_error_getdesc(
    _In_range_(MARBLE_EC_OK, __MARBLE_NUMERRORCODES__ - 1)
	marble_ecode_t ecode /* error code to get description string for */
);


MB_END_HEADER


