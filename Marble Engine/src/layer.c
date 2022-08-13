#include <application.h>


void marble_layer_internal_destroy(
	struct marble_layer **pps_layer
);

/* Global layer ID; gets incremented whenever a layer gets created. */
static int volatile gl_layerid = 0;


/*
 * Layer dummy functions
 * 
 * It is generally not required that all callbacks
 * of a layer be user-defined. In trivial cases,
 * some functions may just be empty.
 * Therefore, when the user submits the callbacks, if a callback
 * function member is NULL, it will be replaced with
 * the corresponding callback defined inside
 * region "dummy callbacks".
 * 
 * This also ensures that the callback pointers are
 * valid at all times, removing the need for extra
 * validation before invoking the them.
 */
#pragma region dummy callbacks
static marble_layer_callback_onpush __marble_layer_internal_cbpush__(
	int layerid,
	void *p_userdata
) {
	UNREFERENCED_PARAMETER(layerid);
	UNREFERENCED_PARAMETER(p_userdata);

	return MARBLE_EC_OK;
}

static marble_layer_callback_onpop __marble_layer_internal_cbpop__(
	int layerid,
	void *p_userdata
) {
	UNREFERENCED_PARAMETER(layerid);
	UNREFERENCED_PARAMETER(p_userdata);

	return MARBLE_EC_OK;
}

static marble_layer_callback_onupdate __marble_layer_internal_cbupdate__(
	int layerid,
	float ft,
	void *p_userdata
) {
	UNREFERENCED_PARAMETER(layerid);
	UNREFERENCED_PARAMETER(ft);
	UNREFERENCED_PARAMETER(p_userdata);

	return MARBLE_EC_OK;
}

static marble_layer_callback_onevent __marble_layer_internal_cbevent__(
	         int layerid,
	struct marble_event *p_event,
	void *p_userdata
) {
	UNREFERENCED_PARAMETER(layerid);
	UNREFERENCED_PARAMETER(p_event);
	UNREFERENCED_PARAMETER(p_userdata);

	return MARBLE_EC_OK;
}
#pragma endregion


/*
 * "Fixes" layer callback structure.
 * This means changing the NULL callbacks to the "dummy callbacks"
 * defined above, making it valid to invoke any of the callbacks
 * directly, and without extra validation.
 */
static void marble_layer_internal_fixcbs(
	struct marble_layer_callbacks *ps_cbs /* callback structure to "fix" */
) {
	ps_cbs->cb_onpush   = ps_cbs->cb_onpush   ? ps_cbs->cb_onpush   : (marble_layer_callback_onpush)&__marble_layer_internal_cbpush__;
	ps_cbs->cb_onpop    = ps_cbs->cb_onpop    ? ps_cbs->cb_onpop    : (marble_layer_callback_onpop)&__marble_layer_internal_cbpop__;
	ps_cbs->cb_onupdate = ps_cbs->cb_onupdate ? ps_cbs->cb_onupdate : (marble_layer_callback_onupdate)&__marble_layer_internal_cbupdate__;
	ps_cbs->cb_onevent  = ps_cbs->cb_onevent  ? ps_cbs->cb_onevent  : (marble_layer_callback_onevent)&__marble_layer_internal_cbevent__;
}

/*
 * Sets layer callbacks.
 * 
 * Returns nothing.
 */
static void marble_layer_internal_setcbs(
	struct marble_layer_callbacks *restrict ps_dest,     /* callback structure to set */
	struct marble_layer_callbacks const *restrict ps_src /* source callback structure */
) {
	if (ps_src != NULL)
		*ps_dest = *ps_src;

	/*
	 * Fix the callbacks. This works even if **ps_src** is NULL. In this case,
	 * all callbacks will be "fixed" to the given "dummy callbacks".
	 */
	marble_layer_internal_fixcbs(ps_dest);
}

/*
 * Sets userdata.
 * 
 * Returns old userdata.
 */
static void *marble_layer_setuserdata(
	void **pp_dest,   /* userdata to set */
	void const *p_src /* userdata pointer */
) {
	void *p_oldudata = *pp_dest;

	*pp_dest = (void *)p_src;
	return p_oldudata;
}

/*
 * Create an empty layer structure.
 * 
 * For it to work, callbacks and possibly userdata
 * still have to be submitted.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static int marble_layer_internal_create(
	bool isenabled,
	struct marble_layer **pps_layer
) { MB_ERRNO
	if (pps_layer == NULL)
		return MARBLE_EC_INTERNALPARAM;

	ecode = marble_system_alloc(sizeof **pps_layer, false, false, pps_layer);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	(*pps_layer)->m_isenabled = isenabled;
	(*pps_layer)->m_id        = InterlockedIncrement((LONG volatile *)&gl_layerid);

	return MARBLE_EC_OK;
}

/*
 * Pushes layer onto the internal layer stack.
 * 
 * If the layer is already pushed, the function will not push
 * the layer again.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static int marble_layer_internal_push(
	struct marble_layer *ps_layer, /* layer to push */
	bool istopmost                 /* push as topmost? */
) { MB_ERRNO
	if (ps_layer == NULL)
		return MARBLE_EC_INTERNALPARAM;
	if (ps_layer->m_ispushed == true)
		return MARBLE_EC_LAYERALREADYPUSHED;

	if (istopmost)
		ecode = marble_util_vec_pushback(gl_app.ms_layerstack.mps_vec, ps_layer);
	else
		ecode = marble_util_vec_insert(
			gl_app.ms_layerstack.mps_vec,
			ps_layer,
			gl_app.ms_layerstack.m_lastlayer
		);

	if (ecode == MARBLE_EC_OK) {
		ps_layer->m_istopmost = istopmost;
		if (!istopmost)
			++gl_app.ms_layerstack.m_lastlayer;

		/*
		 * We will simply pass-through the return value. If "cb_onpush"
		 * fails, we will catch the error code in "marble_application_createlayer()"
		 * and handle it appropriately.
		 */
		ecode = (*ps_layer->ms_cbs.cb_onpush)(
			ps_layer->m_id,
			ps_layer->mp_userdata
		);
	}

	return ecode;
}

/*
 * Removes a layer from the layer stack. 
 */
static int marble_layer_internal_pop(
	struct marble_layer *ps_layer /* layer to pop */
) {
	if (ps_layer == NULL)
		return MARBLE_EC_INTERNALPARAM;

	/* Scan the entire layer stack. */
	size_t index = marble_util_vec_find(
		gl_app.ms_layerstack.mps_vec, 
		ps_layer,
		0,
		0
	);

	/*
	 * If the layer could be found, execute its
	 * "cb_onpop" handler and erase it from the layer stack.
	 */
	if (index != (size_t)(-1)) {
		marble_util_vec_erase(
			gl_app.ms_layerstack.mps_vec,
			index,
			false
		);

		/*
		 * Execute "cb_onpop" handler if it hasn't
		 * been executed yet. 
		 */
		if (ps_layer->m_ispushed == true) {
			(*ps_layer->ms_cbs.cb_onpop)(
				ps_layer->m_id,
				ps_layer->mp_userdata
			);

			ps_layer->m_ispushed = false;
		}

		if (ps_layer->m_istopmost == false)
			--gl_app.ms_layerstack.m_lastlayer;
	}

	return MARBLE_EC_OK;
}


int marble_application_createlayer(
	bool isenabled,
	bool istopmost,
	struct marble_layer_callbacks const *p_callbacks,
	void const *p_userdata,                          													  
	char const *pz_stringid,
	int *p_layerid
) { MB_ERRNO
	if (p_layerid == NULL)
		return MARBLE_EC_PARAM;
	if (gl_app.ms_layerstack.m_isinit == false) {
		*p_layerid = -1;

		return MARBLE_EC_COMPSTATE;
	}

	struct marble_layer *ps_layer;
	ecode = marble_layer_internal_create(isenabled, &ps_layer);
	if (ecode != MARBLE_EC_OK) {
		*p_layerid = -1;

		return ecode;
	};

	marble_layer_internal_setcbs(&ps_layer->ms_cbs, p_callbacks);
	marble_layer_setuserdata(&ps_layer->mp_userdata, p_userdata);
	
	ecode = marble_layer_internal_push(ps_layer, istopmost);
	if (ecode != MARBLE_EC_OK) {
		marble_layer_internal_destroy(&ps_layer);

		*p_layerid = -1;
		return ecode;
	};

	*p_layerid = ps_layer->m_id;
	marble_system_cpystr(ps_layer->maz_stringid, pz_stringid, MB_STRINGIDMAX);

	return MARBLE_EC_OK;
}

/*
 * Destroys a layer.
 * 
 * If the layer that is to be destroyed is still pushed onto 
 * the layer stack, it will be popped automatically.
 * 
 * Returns nothing.
 */
void marble_layer_internal_destroy(
	struct marble_layer **pps_layer /* layer to destroy */
) { MB_ERRNO
	if (pps_layer == NULL || *pps_layer == NULL || gl_app.ms_layerstack.m_isinit == false)
		return;

	/*
	 * Pop the layer. Ideally, the "cb_onpop" handler will take care of
	 * free'ing the userdata. If the user fails to do so, the memory that
	 * is being occupied by the userdata will leak. This is only true, however,
	 * when **marble_layer::mp_userdata** itself points to dynamically allocated memory
	 * or **marble_layer::mp_userdata** contains pointers to dynamically allocated
	 * memory.
	 * We cannot do anything with the userdata here because we do now know the layout
	 * of the struct.
	 */
	ecode = marble_layer_internal_pop(*pps_layer);
	if (ecode != MARBLE_EC_OK)
		return;

	/* Free-up layer memory. */
	free(*pps_layer);
	*pps_layer = NULL;
}


