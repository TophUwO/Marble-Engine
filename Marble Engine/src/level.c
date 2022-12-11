#include <application.h>

#define MB_LA_DEPINDEX_NOTSET (0)


/*
 * layer structure
 * 
 * A layer is solely a container for
 * a list of chunks.
 */
struct marble_levelasset_layer {
    uint32_t                         m_flags; /* flags; currently unused */
    enum marble_levelasset_layertype m_type;  /* layer type */

    struct marble_util_array2d *mps_chunks;   /* chunk storage */
};


static int const gl_maxlevelwidth  = 0xFFFF;
static int const gl_maxlevelheight = 0xFFFF;


/*
 * Requests a new chunk from the level manager.
 * IMPL: asset.c
 * 
 * Returns 0 on success, non-zero on failure.
 */
extern _Critical_ marble_ecode_t marble_assetman_internal_allocatechunk(
    _In_              struct marble_assetman *ps_assetman,       /* asset manager */
    _Init_(pps_chunk) struct marble_levelasset_chunk **pps_chunk /* address of chunk */
);

/*
 * Marks the chunk as unused so the allocator can
 * reuse the memory once a new chunk must be
 * allocated.
 * 
 * Returns nothing.
 */
extern void marble_assetman_internal_freechunk(
    _In_ struct marble_assetman *ps_assetman,     /* asset manager */
    _In_ struct marble_levelasset_chunk *ps_chunk /* address of the chunk */
);


/*
 * Destroys a layer chunk of a given layer type.
 * 
 * Returns nothing.
 */
static void marble_levelasset_internal_destroychunk(
    _In_                enum marble_levelasset_layertype type,     /* type of chunk data */
    _Uninit_(pps_chunk) struct marble_levelasset_chunk **pps_chunk /* chunk to destroy */
) {
    if (pps_chunk == NULL || *pps_chunk == NULL)
        return;

    switch (type) {
        case MARBLE_LA_LTYPE_TEXTURE:
            free(*pps_chunk);

            break;
    }

    *pps_chunk = NULL;
}

/*
 * Destroys a layer. All resources used by it are released.
 * 
 * Returns nothing.
 */
static void marble_levelasset_internal_destroylayer(
    _Uninit_(pps_layer) struct marble_levelasset_layer **pps_layer /* layer to destroy */
) {
    if (pps_layer == NULL || *pps_layer == NULL)
        return;

    /* Destroy chunks. */
    marble_util_array2d_destroy(&(*pps_layer)->mps_chunks);

    /* Free struct memory. */
    free(*pps_layer);
    *pps_layer = NULL;
}

/*
 * Checks whether a layer type is valid or not.
 * 
 * Returns true if the layer type is valid, or
 * false if not.
 */
static _Check_return_ bool marble_levelasset_internal_isvalidlayertype(
    _In_ enum marble_levelasset_layertype type /* type ID to check validity of */
) {
    return
        MB_INRANGE_EXCL(
            type,
            MARBLE_LA_LTYPE_UNKNOWN,
            __MARBLE_NUMLATYPES__
        );
}

/*
 * Checks whether the given layer ID is valid.
 * 
 * Returns true if the layer ID is valid, or
 * false if not.
 */
static _Check_return_ bool marble_levelasset_internal_isvalidlayerid(
    _In_ union marble_levelasset *ps_lvlasset, /* level asset */
    uint32_t layerid                           /* layer ID to check validity of */
) {
    if (ps_lvlasset == NULL)
        return false;

    return layerid >= ps_lvlasset->m_nlayers;
}

/*
 * Creates a layer of a given type.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static _Success_ok_ marble_ecode_t marble_levelasset_internal_createlayer(
    _In_              enum marble_levelasset_layertype type,     /* layer type ID */
    _In_opt_          uint32_t flags,                            /* optional flags */
    _In_              uint16_t width,                            /* width, in chunks */
    _In_              uint16_t height,                           /* height, in chunks */
    _Init_(pps_layer) struct marble_levelasset_layer **pps_layer /* layer to init */ 
) { MB_ERRNO
    if (pps_layer == NULL || !marble_levelasset_internal_isvalidlayertype(type))
        return MARBLE_EC_INTERNALPARAM;

    /* Compute number of chunks. */
    size_t const rsize = (size_t)width * height;
    if (rsize == 0)
        return MARBLE_EC_LEVELSIZE;

    /* Allocate memory. */
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        sizeof **pps_layer,
        true,
        false,
        pps_layer
    );
    if (ecode != MARBLE_EC_OK)
        return ecode;

    /* Set common fields. */
    (*pps_layer)->m_type  = type;
    (*pps_layer)->m_flags = flags;

    /* Initialize chunk storage. */
    ecode = marble_util_array2d_create(
        width,
        height,
        NULL,
        &(*pps_layer)->mps_chunks
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

lbl_END:
    if (ecode != MARBLE_EC_OK)
        marble_levelasset_internal_destroylayer(pps_layer);

    return ecode;
}

static _Check_return_ _Success_ptr_ struct marble_levelasset_chunk *marble_levelasset_internal_getchunk(
    _In_ struct marble_levelasset_layer *ps_layer, /* layer to look in */
         struct marble_pointi2d s_pt               /* chunk coordinates */
) {
    if (ps_layer == NULL)
        return MB_INVPTR;

    return marble_util_array2d_get(
        ps_layer->mps_chunks,
        (size_t)s_pt.m_x,
        (size_t)s_pt.m_y
    );
}


_Critical_ marble_ecode_t MB_CREATEFN(levelasset)(
    _In_    void *p_crparams,
	_Inout_ union marble_levelasset *ps_lvlasset
) { MB_ERRNO
    if (p_crparams == NULL || ps_lvlasset == NULL)
        return MARBLE_EC_INTERNALPARAM;

    MB_VOIDCAST(ps_params, p_crparams, struct marble_levelasset_crparams);

    /* Init common fields. */
    ps_lvlasset->m_width   = ps_params->m_width;
    ps_lvlasset->m_height  = ps_params->m_height;
    ps_lvlasset->m_nlayers = 0;

    /* Create layer storage. */
    ecode = marble_util_vec_create(
        16,
        (void (*)(void **))&marble_levelasset_internal_destroylayer,
        &ps_lvlasset->mps_layers
    );

    return ecode;
}

_Critical_ marble_ecode_t MB_LOADFN(levelasset)(
    _In_    struct marble_util_file *ps_fdesc,
    _Inout_ union marble_levelasset *ps_lvlasset
) {
    // TODO: add code

    return MARBLE_EC_OK;
}

void MB_DESTROYFN(levelasset)(
    _Inout_ union marble_levelasset *ps_lvlasset
) {
	if (ps_lvlasset == NULL)
        return;

    /* Destroy layer stack. */
	marble_util_vec_destroy(&ps_lvlasset->mps_layers);
}

bool MB_VALIDATECRPSFN(levelasset)(
    _In_ void *p_crparams
) {
    if (p_crparams == NULL)
        return false;

    MB_VOIDCAST(ps_params, p_crparams, struct marble_levelasset_crparams);

    return
           MB_INRANGE_INCL(ps_params->m_width, 1, gl_maxlevelwidth)
        && MB_INRANGE_INCL(ps_params->m_height, 1, gl_maxlevelheight);
}

void marble_levelasset_getmaxdims(
    _Out_ struct marble_sizei2d *ps_size
) {
    if (ps_size == NULL)
        return;

    ps_size->m_width  = gl_maxlevelwidth;
    ps_size->m_height = gl_maxlevelheight;
}

void marble_levelasset_getchunksize(
    _Out_ struct marble_sizei2d *ps_size
) {
    if (ps_size == NULL)
        return;

    ps_size->m_width  = MB_LA_CHUNKW;
    ps_size->m_height = MB_LA_CHUNKH;
}

void marble_levelasset_tile2chunk(
              struct marble_pointi2d s_tcoords,
    _Out_opt_ struct marble_pointi2d *ps_chcoords,
    _Out_opt_ struct marble_pointi2d *ps_chloccoords
) {
    if (ps_chcoords != NULL)
        *ps_chcoords = (struct marble_pointi2d){
            .m_x = (s_tcoords.m_x / MB_LA_CHUNKW) % gl_maxlevelwidth,
            .m_y = (s_tcoords.m_y / MB_LA_CHUNKH) % gl_maxlevelheight
        };

    if (ps_chloccoords != NULL)
        *ps_chloccoords = (struct marble_pointi2d){
            .m_x = s_tcoords.m_x % MB_LA_CHUNKW,
            .m_y = s_tcoords.m_y % MB_LA_CHUNKH
        };
}

void marble_levelasset_chunk2tile(
              struct marble_pointi2d s_chcoords,
              struct marble_pointi2d s_chloccoords,
    _Out_opt_ struct marble_pointi2d *ps_tcoords
) {
    if (ps_tcoords != NULL)
        *ps_tcoords = (struct marble_pointi2d){
            .m_x = s_chcoords.m_x * 16 + s_chloccoords.m_x % MB_LA_CHUNKW,
            .m_y = s_chcoords.m_y * 16 + s_chloccoords.m_y % MB_LA_CHUNKH
        };
}

_Critical_ marble_ecode_t marble_levelasset_addlayer(
    _In_ union marble_levelasset *ps_lvlasset,
    _In_ enum marble_levelasset_layertype type,
         uint32_t index
) { MB_ERRNO
    if (ps_lvlasset == NULL)
        return MARBLE_EC_PARAM;

    struct marble_levelasset_layer *ps_layer = NULL;

    /* Create the layer. */
    ecode = marble_levelasset_internal_createlayer(
        type,
        0,
        ps_lvlasset->m_width,
        ps_lvlasset->m_height,
        &ps_layer
    );
    if (ecode != MARBLE_EC_OK)
        return ecode;

    /* Insert it into the layer stack. */
    ecode = marble_util_vec_insert(
        ps_lvlasset->mps_layers,
        ps_layer,
        min(marble_util_vec_count(ps_lvlasset->mps_layers), index)
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

lbl_END:
    if (ecode != MARBLE_EC_OK)
        marble_levelasset_internal_destroylayer(&ps_layer);

    return ecode;
}

void marble_levelasset_removelayer(
    _In_ union marble_levelasset *ps_lvlasset,
         uint32_t index
) {
    if (ps_lvlasset == NULL)
        return;

    /*
     * Remove the layer from the layer stack and
     * free all resources used by it.
     */
    marble_util_vec_erase(
        ps_lvlasset->mps_layers,
        (size_t)index,
        true
    );
}

uint32_t marble_levelasset_movelayer(
    _In_ union marble_levelasset *ps_lvlasset,
         uint32_t index,
         uint32_t newpos
) {
    // TODO: add code

    return 0;
}

marble_ecode_t marble_levelasset_setentity(
    _In_ union marble_levelasset *ps_lvlasset,
         uint32_t lindex,
         uint32_t xpos,
         uint32_t ypos,
         union marble_levelasset_chunkentity *ps_entity
) { MB_ERRNO
    if (ps_lvlasset == NULL || !marble_levelasset_internal_isvalidlayerid(ps_lvlasset, lindex))
        return MARBLE_EC_PARAM;

    /* Get reference to layer. */
    struct marble_levelasset_layer *ps_layer = marble_util_vec_get(ps_lvlasset->mps_layers, lindex);
    if (ps_layer == NULL)
        return MARBLE_EC_OUTOFRANGE;

    /* Calculate chunk (and chunk-local) coordinates. */
    struct marble_pointi2d s_cpt = { 0 }, s_clpt = { 0 };
    marble_levelasset_tile2chunk(
        (struct marble_pointi2d){ xpos, ypos },
        &s_cpt,
        &s_clpt
    );

    /* Get reference to chunk. */
    struct marble_levelasset_chunk *ps_chunk = marble_levelasset_internal_getchunk(
        ps_layer,
        s_cpt
    );
    if (ps_chunk == MB_INVPTR) {
        ecode = MARBLE_EC_INTERNALPARAM;

        goto lbl_END;
    } else if (ps_chunk == NULL) {
        /*
         * If no chunk with the given coordinates could
         * be found, create a new one.
         */
        ecode = marble_assetman_internal_allocatechunk(
            ps_lvlasset->_base.mps_refparent,
            &ps_chunk
        );
        if (ecode != MARBLE_EC_OK)
            goto lbl_END;

        /* Add the chunk to the array. */
        ecode = marble_util_array2d_insert(
            ps_layer->mps_chunks,
            (size_t)xpos,
            (size_t)ypos,
            false,
            ps_chunk,
            NULL
        );
        if (ecode != MARBLE_EC_OK)
            goto lbl_END;
    }

    /* Get reference to target entity. */
    union marble_levelasset_chunkentity *ps_ent = &ps_chunk->muaa_data[s_clpt.m_x][s_clpt.m_y];

    /*
     * If the source entity is NULL, remove the existing
     * entity from the position.
     */
    if (ps_entity == NULL) {
        if (ps_ent->_stex.m_depindex == MB_LA_DEPINDEX_NOTSET)
            goto lbl_END;

        ps_ent->_stex.m_depindex = MB_LA_DEPINDEX_NOTSET;

        /*
         * If entity count reaches zero or below, free the
         * chunk so the level manager can reuse it.
         */
        if (--ps_chunk->m_nents <= 0)
            marble_assetman_internal_freechunk(
                ps_lvlasset->_base.mps_refparent,
                ps_chunk
            );

        goto lbl_END;
    }

    /*
     * If no entity is currently set at the given
     * coordinates, increase the entity count.
     */
    if (ps_ent->_stex.m_depindex == MB_LA_DEPINDEX_NOTSET)
        ++ps_chunk->m_nents;
    
    /* Copy entity. */
    switch (ps_layer->m_type) {
        case MARBLE_LA_LTYPE_TEXTURE:
            marble_system_cpymem(
                ps_ent,
                &ps_entity->_stex,
                sizeof ps_entity->_stex
            );

            break;
    }

lbl_END:
    return ecode;
}

union marble_levelasset_chunkentity *marble_levelasset_getentity(
    _In_ union marble_levelasset *ps_lvlasset,
         uint32_t lindex,
         uint32_t xpos,
         uint32_t ypos
) {
    if (ps_lvlasset == NULL || !marble_levelasset_internal_isvalidlayerid(ps_lvlasset, lindex))
        return MB_INVPTR;

    /* Get reference to layer. */
    struct marble_levelasset_layer *ps_layer = marble_util_vec_get(ps_lvlasset->mps_layers, lindex);
    if (ps_layer == NULL)
        return MB_INVPTR;

    /* Calculate chunk (and chunk-local) coordinates. */
    struct marble_pointi2d s_cpt = { 0 }, s_clpt = { 0 };
    marble_levelasset_tile2chunk(
        (struct marble_pointi2d){ xpos, ypos },
        &s_cpt,
        &s_clpt
    );

    /* Get reference to chunk. */
    struct marble_levelasset_chunk *ps_chunk = marble_levelasset_internal_getchunk(
        ps_layer,
        s_cpt
    );
    if (ps_chunk == NULL || ps_chunk == MB_INVPTR)
        return ps_chunk == NULL ? NULL : MB_INVPTR;

    return &ps_chunk->muaa_data[s_clpt.m_x][s_clpt.m_y];
}


