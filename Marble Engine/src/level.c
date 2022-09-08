#include <application.h>


static void marble_levelasset_internal_destroylayer(struct marble_levelasset_layer **pps_layer) {
	if (pps_layer == NULL || *pps_layer == NULL)
		return;

	free((*pps_layer)->mpu_layerdata);

	free(*pps_layer);
	*pps_layer = NULL;
}


_Critical_ marble_ecode_t marble_levelasset_new(
	_In_                 int width,
	_In_                 int height,
	_Init_(pps_lvlasset) struct marble_levelasset **pps_lvlasset
) { MB_ERRNO
	if (pps_lvlasset == NULL || width * height < 1)
		return MARBLE_EC_PARAM;

	width  = abs(width);
	height = abs(height);

	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof **pps_lvlasset,
		false,
		false,
		pps_lvlasset
	);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	ecode = marble_util_vec_create(
		4,
		(void (MB_CALLBACK *)(void **))&marble_levelasset_internal_destroylayer,
		&(*pps_lvlasset)->mps_layers
	);
	if (ecode != MARBLE_EC_OK) {
		marble_levelasset_destroy(pps_lvlasset);

		return ecode;
	}

	(*pps_lvlasset)->m_width = width;
	(*pps_lvlasset)->m_height = height;

	return MARBLE_EC_OK;
}

void marble_levelasset_destroy(
	struct marble_levelasset **pps_lvlasset
) {
	if (pps_lvlasset == NULL || *pps_lvlasset == NULL)
		return;

	marble_util_vec_destroy(&(*pps_lvlasset)->mps_layers);

	/*
	 * will be removed once asset manager
	 * is present in the editor.
	 */
#if 1
	// Leak dependency table -- this is fine for now.

	free(*pps_lvlasset);
	*pps_lvlasset = NULL;
#endif
}

marble_ecode_t marble_levelasset_addlayer(
	_In_ struct marble_levelasset *ps_lvlasset,
	     enum marble_levelasset_layertype type
) { MB_ERRNO
	if (ps_lvlasset == NULL)
		return MARBLE_EC_PARAM;

	struct marble_levelasset_layer *ps_layer = NULL;
	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof *ps_layer,
		true,
		false,
		&ps_layer
	);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		ps_lvlasset->m_width * ps_lvlasset->m_height * sizeof(union marble_levelasset_layerobj),
		true,
		false,
		&ps_layer->mpu_layerdata
	);
	if (ecode != MARBLE_EC_OK) {
		marble_levelasset_internal_destroylayer(&ps_layer);

		return ecode;
	}

	ecode = marble_util_vec_pushback(ps_lvlasset->mps_layers, ps_layer);
	if (ecode != MARBLE_EC_OK) {
		marble_levelasset_internal_destroylayer(&ps_layer);

		return ecode;
	}

	ps_layer->m_type = type;
	return MARBLE_EC_OK;
}


