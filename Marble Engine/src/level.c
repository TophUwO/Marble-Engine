#include <application.h>

#define MB_LA_DEPINDEX_NOTSET (0)


/*
 * chunk structure 
 */
struct marble_levelasset_chunk {
    /*
     * Number of entities currently populated (i.e. non-NULL);
     * Note that a number of 0 indicates that the chunk is
     * unused. A number of n=1 or higher indicates that the chunk
     * is in use and has n-1 entities actually populated.
     */
    int16_t m_nents;
    /*
     * xpos and ypos of the chunk relative to the
     * upper-left corner (0, 0) of the map
     */
    uint16_t m_xpos;
    uint16_t m_ypos;

    /*
     * chunk entity
     * 
     * represents a single entity on a map, i.e.
     *  (1) a static texture (tile)
     */
    union marble_levelasset_chunkentity {
        /* static texture */
        struct {
            /*
             * Index into the dependency table.
             * 
             * NOTE: A value of 0 is reserved for the state
             * of no set dependency index. So the index of
             * the first dep is not 0 but 1.
             */
            uint16_t m_depindex;

            /*
             * xpos and ypos of the tile in the tileset,
             * relative to the upper-left corner of the
             * tileset
             */
            uint8_t m_depx;
            uint8_t m_depy;
        } _stex;
    } muaa_data[16][16];
};

/*
 * Asset-subtype representing a level
 */
MB_DEFSUBTYPE(marble_levelasset, struct marble_asset, {
    uint16_t m_width;   /* width of each layer, in chunks */
    uint16_t m_height;  /* height of each layer, in chunks */
    uint32_t m_nlayers; /* number of layers */

    /*
     * List of layers. The topmost layer is the
     * first element in this list. 
     */
    struct marble_util_vec *mps_layers;
});
static_assert(_Alignof(union marble_levelasset) == _Alignof(struct marble_asset), "");

/* size fields, in bytes -- for the asset manager */
size_t const gl_lvlassetsize      = sizeof(union marble_levelasset);
size_t const gl_lvlassetchunksize = sizeof(struct marble_levelasset_chunk);


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


/*
 * hard limits regarding the level asset 
 */
static MB_ASSETTYPELIMITS(levelasset) const gls_limits = {
    .m_minwidth    = 1,
    .m_minheight   = 1,
    .m_maxwidth    = 0xFFFF,
    .m_maxheight   = 0xFFFF,
    .m_maxlayers   = 32,
    .m_maxtilesets = 0xFF,
    .m_chunkwidth  = 16,
    .m_chunkheight = 16
};


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


#pragma region UTILFN
int16_t marble_levelasset_internal_getchunkflag(
    _In_ struct marble_levelasset_chunk *ps_chunk /* address of the chunk */
) {
    if (ps_chunk == NULL)
        return SHORT_MIN;

    return ps_chunk->m_nents;
}

void marble_levelasset_internal_setchunkflag(
    _In_ struct marble_levelasset_chunk *ps_chunk, /* address of the chunk */
         int16_t flag                              /* flag to set */
) {
    if (ps_chunk == NULL)
        return;

    ps_chunk->m_nents = flag;
}
#pragma endregion (UTILFN)

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

/*
 * Retrieves a pointer to a chunk object in a given layer.
 * 
 * Returns the pointer to the chunk. On failure, the return value
 * is MB_INVPTR.
 */
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

/*
 * Retrieves the address of a specific chunk entity in a given
 * level asset.
 * 
 * Returns the pointer to the chunk entity. On failure, the return value
 * is MB_INVPTR.
 */
static _Check_return_ _Success_ptr_ union marble_levelasset_chunkentity *marble_levelasset_internal_getchunkentaddr(
    _In_         union marble_levelasset *ps_lvlasset,       /* level asset */
                 uint32_t lindex,                            /* layer index */
                 uint32_t xpos,                              /* xpos, in tiles */
                 uint32_t ypos,                              /* ypos, in tiles */
    _Outptr_opt_ struct marble_levelasset_chunk **pps_chunk, /* chunk pointer */
    _Out_        marble_ecode_t *p_ecode,                    /* error code */
                 bool createchunk                            /* create chunk if it does not exist? */
) {
    if (ps_lvlasset == NULL || !marble_levelasset_internal_isvalidlayerid(ps_lvlasset, lindex) || p_ecode == NULL) {
        if (p_ecode != NULL)
            *p_ecode = MARBLE_EC_PARAM;

        return MB_INVPTR;
    }

    /* Get reference to layer. */
    struct marble_levelasset_layer *ps_layer = marble_util_vec_get(ps_lvlasset->mps_layers, lindex);
    if (ps_layer == NULL) {
        *p_ecode = MARBLE_EC_LAYERADDR;

        return MB_INVPTR;
    }

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
    if (ps_chunk == NULL || ps_chunk == MB_INVPTR) {
        if (!createchunk) {
            *p_ecode = MARBLE_EC_CHUNKADDR;

            return MB_INVPTR;
        }

        /*
         * If no chunk with the given coordinates could
         * be found, create a new one.
         */
        *p_ecode = marble_assetman_internal_allocatechunk(
            ps_lvlasset->_base.mps_refparent,
            &ps_chunk
        );
        if (*p_ecode != MARBLE_EC_OK)
            return MB_INVPTR;

        /* Add the chunk to the array. */
        *p_ecode = marble_util_array2d_insert(
            ps_layer->mps_chunks,
            (size_t)xpos,
            (size_t)ypos,
            false,
            ps_chunk,
            NULL
        );
        if (*p_ecode != MARBLE_EC_OK) {
            marble_assetman_internal_freechunk(
                ps_lvlasset->_base.mps_refparent,
                ps_chunk
            );

            return MB_INVPTR;
        }
    }
    if (pps_chunk != NULL)
        *pps_chunk = ps_chunk;

    /* Get pointer to chunk entity. */
    *p_ecode = MARBLE_EC_OK;

    return &ps_chunk->muaa_data[s_clpt.m_x][s_clpt.m_y];
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
           MB_INRANGE_INCL(ps_params->m_width, 1, gls_limits.m_maxwidth)
        && MB_INRANGE_INCL(ps_params->m_height, 1, gls_limits.m_maxheight);
}

bool MB_QUERYHARDLIMITSFN(levelasset)(
    _In_           size_t ssize,
    _Outsz_(ssize) void *p_limits
) {
    if (p_limits == NULL || ssize != sizeof(struct marble_levelasset_limits))
        return false;

    marble_system_cpymem(p_limits, &gls_limits, ssize);
    return true;
}

void marble_levelasset_tile2chunk(
              struct marble_pointi2d s_tcoords,
    _Out_opt_ struct marble_pointi2d *ps_chcoords,
    _Out_opt_ struct marble_pointi2d *ps_chloccoords
) {
    if (ps_chcoords != NULL)
        *ps_chcoords = (struct marble_pointi2d){
            .m_x = (s_tcoords.m_x / gls_limits.m_chunkwidth) % gls_limits.m_maxwidth,
            .m_y = (s_tcoords.m_y / gls_limits.m_chunkheight) % gls_limits.m_maxheight
        };

    if (ps_chloccoords != NULL)
        *ps_chloccoords = (struct marble_pointi2d){
            .m_x = s_tcoords.m_x % gls_limits.m_chunkwidth,
            .m_y = s_tcoords.m_y % gls_limits.m_chunkheight
        };
}

void marble_levelasset_chunk2tile(
              struct marble_pointi2d s_chcoords,
              struct marble_pointi2d s_chloccoords,
    _Out_opt_ struct marble_pointi2d *ps_tcoords
) {
    if (ps_tcoords != NULL)
        *ps_tcoords = (struct marble_pointi2d){
            .m_x = s_chcoords.m_x * gls_limits.m_chunkwidth + s_chloccoords.m_x % gls_limits.m_chunkwidth,
            .m_y = s_chcoords.m_y * gls_limits.m_chunkheight + s_chloccoords.m_y % gls_limits.m_chunkheight
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

_Success_ok_ marble_ecode_t marble_levelasset_getstatictexture(
    _In_  union marble_levelasset *ps_lvlasset,
          uint32_t lindex,
          uint32_t xpos,
          uint32_t ypos,
    _Out_ uint16_t *p_depindex,
    _Out_ uint8_t *p_depx,
    _Out_ uint8_t *p_depy
) { MB_ERRNO
    if (p_depindex == NULL || p_depx == NULL || p_depy == NULL)
        return MARBLE_EC_PARAM;

    /* Get pointer to desired chunk entity. */
    union marble_levelasset_chunkentity *ps_ent = marble_levelasset_internal_getchunkentaddr(
        ps_lvlasset,
        lindex,
        xpos,
        ypos,
        NULL,
        &ecode,
        false
    );
    if (ps_ent == MB_INVPTR || ecode != MARBLE_EC_OK) {
        *p_depindex = 0;
        *p_depx     = 0;
        *p_depy     = 0;

        return ecode;
    }

    /* Write data. */
    *p_depindex = ps_ent->_stex.m_depindex;
    *p_depx     = ps_ent->_stex.m_depx;
    *p_depy     = ps_ent->_stex.m_depy;

    return MARBLE_EC_OK;
}

_Success_ok_ marble_ecode_t marble_levelasset_setstatictexture(
    _In_ union marble_levelasset *ps_lvlasset,
         uint32_t lindex,
         uint32_t xpos,
         uint32_t ypos,
    _In_ uint16_t depindex,
         uint8_t depx,
         uint8_t depy
) { MB_ERRNO
    /* Get pointer to desired chunk entity. */
    struct marble_levelasset_chunk *ps_chunk = NULL;

    union marble_levelasset_chunkentity *ps_ent = marble_levelasset_internal_getchunkentaddr(
        ps_lvlasset,
        lindex,
        xpos,
        ypos,
        &ps_chunk,
        &ecode,
        true
    );
    if (ps_ent == MB_INVPTR || ecode != MARBLE_EC_OK)
        return ecode;

    /* Update chunk entity. */
    ps_ent->_stex.m_depindex = depindex;
    ps_ent->_stex.m_depx     = depx;
    ps_ent->_stex.m_depy     = depy;

    /*
     * If the chunk entity was deleted, check if
     * the chunk itself can be freed.
     */
    if (depindex == 0)
        if (--ps_chunk->m_nents <= 0)
            marble_assetman_internal_freechunk(
                ps_lvlasset->_base.mps_refparent,
                ps_chunk
            );

    return MARBLE_EC_OK;
}


