#pragma once

#include <event.h>
#include <error.h>
#include <log.h>


/*
 * app settings
 * 
 * Gets automatically submitted by the entry-point
 * and contains data needed for creating a main window,
 * and other flags that control application modes etc.
 * 
 * NOTE: For now, only square tiles are supported and the main
 *       window will always open on the primary monitor
 *       of the system.
 */
struct marble_app_settings {
	char const *mpz_title; /* window title */
	int m_width;           /* width, in tiles */
	int m_height;          /* height, in tiles */
	int m_tilesize;        /* size of a tile, in pixels */
};


#pragma region marble_application
struct marble_layer_cbs;

/*
 * Creates a user-defined layer.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API marble_ecode_t marble_application_createlayer(
	/*
	 * Layers can be either enabled or disabled. While enabled
	 * layers behave as normal, disabled layers, while still present
	 * in the system, will not be rendered or receive updates.
	 * Waking-up a layer can be done by calling "marble_layer_setenabled()"
	 * on a layer id.
	 */
	bool isenabled,
	/*
	 * Layers can be pushed as top-most, meaning it 
	 * will be the first to receive events, but
	 * the last to render. There can be more than one
	 * overlay. Layers (including those that are top-most)
	 * will always be rendered in the order they were pushed.
	 */
	bool istopmost,
	struct marble_layer_cbs const *p_callbacks, /* user-defined callback functions */
	void const *p_userdata,                     /* optional userdata */
	/*
	 * Optional string id; used only internally.
	 * 
	 * Note: An asset ID is a NUL-terminated ASCII string not longer than MB_STRINGIDMAX
	 * bytes (including NUL-terminator).
	 */
	char const *pz_stringid,
	/* 
	 * Pointer to an integer variable receiving the newly 
	 * created layer.
	 * If the function fails, the memory pointed to by this parameter
	 * will be zero and the function will return non-zero.
	 */
	int *p_layerid
);
#pragma endregion


#pragma region marble_layer
/*
 * layer callbacks
 *
 * cb_onpush:   Called directly after the layer was pushed onto the layer stack.
 * cb_onpop:    Called directly before the layer is popped from the layer stack.
 *              If the userdata is dynamically allocated memory, it has to be free'd
 *              from within "cb_onpop".
 * cb_onupdate: Called once every frame.
 * cb_onevent:  Called upon receiving a system event.
 */
struct marble_layer_cbs {
	void (MB_CALLBACK *cb_oncreate)(int layerid, void *p_userdata);
	void (MB_CALLBACK *cb_onupdate)(int layerid, float ft, void *p_userdata);
	void (MB_CALLBACK *cb_onevent)(int layerid, struct marble_event *ps_event, void *p_userdata);
	void (MB_CALLBACK *cb_ondestroy)(int layerid, void* p_userdata);
};


/*
 * Gets current userdata.
 * 
 * Returns a pointer to the userdata structure that
 * id currently attached to the layer. This is the same
 * pointer that was submitted by a past call to 
 * "marble_application_createlayer()" or "marble_layer_submituserdata()".
 * If the layer does not exist, or there is no userdata currently
 * attached to the layer, the function returns NULL.
 */
MB_API void *marble_layer_getuserdata(
	int layerid /* layer to get userdata for */
);

/*
 * Gets current layer state.
 * 
 * Returns true (1) if the layer is currently enabled, or false (0)
 * if the layer is currently disabled.
 */
MB_API bool marble_layer_isenabled(
	int layerid /* layer to get state for */
);

/*
 * Sets the state of a layer.
 * 
 * For more information regarding the state of a layer
 * and a layers behavior in relation to its state,
 * consult the inline documentation for "marble_application_createlayer()".
 * 
 * Returns nothing.
 */
MB_API void marble_layer_setenabled(
	int layerid,   /* layer to set state for */
	bool isenabled /* enable or disable layer? */
);

/*
 * Submits new userdata.
 * 
 * Returns the pointer to the old userdata; this is the same pointer that
 * was submitted by a past call to "marble_application_createlayer()" or
 * "marble_layer_submituserdata()".
 * If the layer does not exist, or there is no userdata to return,
 * the function returns NULL.
 */
MB_API void *marble_layer_submituserdata(
	int layerid,              /* layer to submit userdata to */
	void const *p_newuserdata /* new userdata to submit */
);
#pragma endregion


