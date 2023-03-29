#include <application.h>


/* Error code strings */
static char const *const glapz_ecodestrs[] = {
	"MARBLE_EC_OK",
	"MARBLE_EC_UNKNOWN",

	"MARBLE_EC_UNIMPLFEATURE",
	"MARBLE_EC_MEMALLOC",
	"MARBLE_EC_MEMREALLOC",
	"MARBLE_EC_MEMCPY",
	"MARBLE_EC_PARAM",
	"MARBLE_EC_INTERNALPARAM",
	"MARBLE_EC_OPENFILE",
	"MARBLE_EC_READFILE",
	"MARBLE_EC_FSEEK",
	"MARBLE_EC_REGWNDCLASS",
	"MARBLE_EC_INITCOM",
	"MARBLE_EC_CREATEWND",
	"MARBLE_EC_INITUTILS",
	"MARBLE_EC_COMPSTATE",
	"MARBLE_EC_CREATED2DFAC",
	"MARBLE_EC_CREATED3D11DEV",
	"MARBLE_EC_GETDXGIDEV",
	"MARBLE_EC_CREATED2DDEV",
	"MARBLE_EC_CREATED2DDEVCTXT",
	"MARBLE_EC_GETDXGIADAPTER",
	"MARBLE_EC_GETDXGIFAC",
	"MARBLE_EC_CREATESWAPCHAIN",
	"MARBLE_EC_GETDXGIBACKBUFFER",
	"MARBLE_EC_CREATED2DBMPFROMDXGISURFACE",
	"MARBLE_EC_RESIZERENDERER",
	"MARBLE_EC_ASSETTYPE",
	"MARBLE_EC_LAYERALREADYPUSHED",
	"MARBLE_EC_RENDERAPI",
	"MARBLE_EC_CREATERENDERER",
	"MARBLE_EC_NOTFOUND",
	"MARBLE_EC_GETMONITORINFO",
	"MARBLE_EC_CALCWNDSIZE",
	"MARBLE_EC_QUERYWINDOWRECT",
    "MARBLE_EC_FILESIZE",
    "MARBLE_EC_CREATEPARAMS",
    "MARBLE_EC_REGTMPASSET",
    "MARBLE_EC_LEVELSIZE",
    "MARBLE_EC_OUTOFRANGE",
    "MARBLE_EC_PCONTRADICTION",
    "MARBLE_EC_LAYERADDR",
    "MARBLE_EC_CHUNKADDR",
    "MARBLE_EC_ALREADYDEP",
    "MARBLE_EC_NOTDEP",
    "MARBLE_EC_INITLOCK"
};

/* Error code descriptions */
static char const *const glapz_ecodedescs[] = {
	"Ok; no error occurred.",
	"Unknown error code.",

	"The requested feature is currently unimplemented.",
	"Failed to allocate memory.",
	"Failed to change array size.",
	"Failed to copy memory.",
	"Invalid parameter has been passed (by the user) to a function.",
	"Invalid parameter has been internally passed.",
	"Could not open file.",
	"Could not read from file.",
	"Could not jump to file position.",
	"Could not register window class.",
	"Failed to initialize the component object model (COM).",
	"Failed to create window.",
	"Could not initialize utilities.",
	"Operation could not be carried out due to one or mode components not being available.",
	"Could not create Direct2D factory.",
	"Failed to create Direct3D (11) device.",
	"Failed to create DXGI device.",
	"Failed to create Direct2D device.",
	"Failed to create Direct2D device context..",
	"Failed to obtain DXGI adapter.",
	"Failed to create DXGI factory.",
	"Failed to create swap-chain.",
	"Failed to get DXGI buffer.",
	"Could not create Direct2D bitmap from DXGI surface.",
	"Could not resize renderer buffers.",
	"Invalid asset type.",
	"Layer is already pushed onto the internal layer stack.",
	"Invalid renderer API.",
	"Failed to create and initialize renderer.",
	"The specified object could not be found.",
	"Could not get monitor info.",
	"Could not calculate window dimensions.",
	"Could not query window or client rectangle.",
    "File size does not equal to the expected size",
    "Create-parameters are invalid.",
    "Failed to register temporary asset.",
    "Invalid (<= 0) level dimensions.",
    "Position/Offset out of range.",
    "Parameter contradiction.",
    "Malformed layer address",
    "Malformed chunk address",
    "Dependency is already part of the dependency chain.",
    "Dependency is not a direct dependency of the asset.",
    "Failed to initialize synchronization lock."
};


char const *const marble_error_getstr(
	_In_range_(MARBLE_EC_OK, __MARBLE_NUMERRORCODES__ - 1)
	marble_ecode_t ecode
) {
	if (!MB_INRANGE_EXCL(ecode, MARBLE_EC_OK, __MARBLE_NUMERRORCODES__ - 1))
		return marble_error_getstr(MARBLE_EC_UNKNOWN);

	return glapz_ecodestrs[ecode];
}

char const *const marble_error_getdesc(
	_In_range_(MARBLE_EC_OK, __MARBLE_NUMERRORCODES__ - 1)
	marble_ecode_t ecode
) {
	if (!MB_INRANGE_EXCL(ecode, MARBLE_EC_OK, __MARBLE_NUMERRORCODES__ - 1))
		return marble_error_getdesc(MARBLE_EC_UNKNOWN);

	return glapz_ecodedescs[ecode];
}


