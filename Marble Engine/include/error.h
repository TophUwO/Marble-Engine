#pragma once

#include <api.h>


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
	MARBLE_EC_DEBUGCON,
	MARBLE_EC_MEMALLOC,
	MARBLE_EC_MEMREALLOC,
	MARBLE_EC_MEMCPY,
	MARBLE_EC_PARAM,
	MARBLE_EC_INTERNALPARAM,
	MARBLE_EC_OPENFILE,
	MARBLE_EC_READFILE,
	MARBLE_EC_FSEEK,
	MARBLE_EC_PATHLENGTH,
	MARBLE_EC_EMPTYPATH,
	MARBLE_EC_INVALIDPATHENC,
	MARBLE_EC_REGWNDCLASS,
	MARBLE_EC_INITCOM,
	MARBLE_EC_CREATEWND,
	MARBLE_EC_INITHPC,
	MARBLE_EC_STATE,
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
	MARBLE_EC_CREATED2DSTROKESTYLE,
	MARBLE_EC_RESIZERENDERER,
	MARBLE_EC_ASSETTYPE,
	MARBLE_EC_ASSETID,
	MARBLE_EC_ELEMENTNOTFOUND,
	MARBLE_EC_DUPEDASSET,
	MARBLE_EC_ASSETNOTFOUND,
	MARBLE_EC_LAYERALREADYPUSHED,
	MARBLE_EC_LAYERSTATE,
	MARBLE_EC_RENDERAPI,
	MARBLE_EC_CREATERENDERER,
	MARBLE_EC_NOTFOUND,
	MARBLE_EC_GETMONITORINFO,
	MARBLE_EC_CALCWNDSIZE,
	MARBLE_EC_QUERYWINDOWRECT,
	MARBLE_EC_LOADRESOURCE,
	MARBLE_EC_CREATEMEMDC,
	MARBLE_EC_CREATEMEMBITMAP,
	MARBLE_EC_GETDC,
	MARBLE_EC_GETWNDCLASSATTRIBS,
	MARBLE_EC_USERINIT,

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
	/* error code to get string representation for */
	_In_range_(MARBLE_EC_OK, __MARBLE_NUMERRORCODES__ - 1)
	marble_ecode_t ecode
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
	/* error code to get description string for */
	_In_range_(MARBLE_EC_OK, __MARBLE_NUMERRORCODES__ - 1)
	marble_ecode_t ecode
);


