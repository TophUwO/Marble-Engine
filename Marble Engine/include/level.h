#pragma once

#include <asset.h>

#define MB_LA_CHUNKW (16)
#define MB_LA_CHUNKH (16)


MB_BEGIN_HEADER


/*
 * layer types 
 */
enum marble_levelasset_layertype {
    MARBLE_LA_LTYPE_UNKNOWN = 0, /* unknown/unspecified */

    MARBLE_LA_LTYPE_TEXTURE,     /* texture layer */

#if (defined MB_DYNAMIC_LIBRARY)
    __MARBLE_NUMLATYPES__        /* for internal use */
#endif
};


/*
 * create-parameters for the "marble_asset_create()"
 * function
 */
struct marble_levelasset_crparams {
    uint16_t m_width;  /* width, in chunks */
    uint16_t m_height; /* height, in chunks */
};

/*
 * chunk structure 
 */
struct marble_levelasset_chunk {
    /*
     * xpos and ypos of the chunk relative to the
     * upper-left corner (0, 0) of the map
     */
    uint16_t m_xpos;
    uint16_t m_ypos;
    /* number of entities currently populated (i.e. non-NULL) */
    uint8_t  m_nents;

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
    } muaa_data[MB_LA_CHUNKW][MB_LA_CHUNKH];
};

/*
 * layer structure
 * 
 * A layer is solely a container for
 * a list of chunks.
 */
struct marble_levelasset_layer {
    uint32_t                         m_flags; /* flags; currently unused */
    enum marble_levelasset_layertype m_type;  /* layer type */

    struct marble_util_htable *mps_chunks;    /* chunk map */
};

/*
 * Asset-subtype representing a level
 */
MB_DEFSUBTYPE(marble_levelasset, struct marble_asset, {
    uint16_t m_width;   /* width of each layer, in tiles */
    uint16_t m_height;  /* height of each layer, in tiles */
    uint32_t m_nlayers; /* number of layers */

    /*
     * List of layers. The topmost layer is the
     * first element in this list. 
     */
    struct marble_util_vec *mps_layers;
});


#if (defined MB_DYNAMIC_LIBRARY) || (defined MB_ECOSYSTEM)
#if (defined MB_DYNAMIC_LIBRARY)
/*
 * API functions used internally by the
 * asset manager 
 */

/*
 * Creates a new level-asset inside **ps_lvlasset**.
 * The memory for the structure has to be allocated
 * in advance.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ extern marble_ecode_t MB_CREATEFN(levelasset)(
    _In_    void *p_crparams,                    /* create-params */
	_Inout_ union marble_levelasset *ps_lvlasset /* pointer to an asset structure */
);

/*
 * Loads an asset from a resource file.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ extern marble_ecode_t MB_LOADFN(levelasset)(
    _In_    struct marble_util_file *ps_fdesc,   /* file descriptor */
    _Inout_ union marble_levelasset *ps_lvlasset /* asset pointer */
);

/*
 * Destroys a level-asset structure. The structure
 * memory is not released.
 * 
 * Returns nothing.
 */
extern void MB_DESTROYFN(levelasset)(
    _Inout_ union marble_levelasset *ps_lvlasset /* pointer to the level-asset */
);

/*
 * Validates create-parameters.
 * 
 * Returns true if the parameters are valid,
 * false if not.
 */
extern bool MB_VALIDATECRPSFN(levelasset)(
    _In_ void *p_crparams /*  pointer to the create-params struct */
);
#endif

/*
 * Returns the maximum supported width and height, in chunks.
 */
MB_API void marble_levelasset_getmaxdims(
    _Out_ struct marble_sizei2d *ps_size /* structure to receive the size */
);

/*
 * Returns the width and the height of a layer chunk.
 */
MB_API void marble_levelasset_getchunksize(
    _Out_ struct marble_sizei2d *ps_size /* structure to receive the size */
);

/*
 * Converts tile coordinates to chunk (and, optionally, chunk-
 * local) coordinates.
 * 
 * Returns nothing.
 */
MB_API void marble_levelasset_tile2chunk(
              struct marble_pointi2d s_tcoords,      /* tile coordinates */
    _Out_opt_ struct marble_pointi2d *ps_chcoords,   /* buffer to receive chunk coordinates */
    _Out_opt_ struct marble_pointi2d *ps_chloccoords /* opt. buffer to receive chunk-local tile coordinates */
);

/*
 * Converts chunk (and chunk-local) coordinates to tile coordinates.
 * 
 * Returns nothing.
 */
MB_API void marble_levelasset_chunk2tile(
              struct marble_pointi2d s_chcoords,    /* chunk coordinates */
              struct marble_pointi2d s_chloccoords, /* opt. chunk-local tile coordinates */
    _Out_opt_ struct marble_pointi2d *ps_tcoords    /* buffer to receive tile coordinates */
);

/*
 * Adds a layer to the level. No chunks will be automatically added.
 * Higher values passed for **index** denote a position further down
 * layer stack.
 *
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ MB_API marble_ecode_t marble_levelasset_addlayer(
    _In_ union marble_levelasset *ps_lvlasset,  /* level asset */
    _In_ enum marble_levelasset_layertype type, /* layer type */
         uint32_t index                         /* insert index */
);

/*
 * Removes and destroys a layer.
 * 
 * Returns nothing.
 */
MB_API void marble_levelasset_removelayer(
    _In_ union marble_levelasset *ps_lvlasset, /* level asset */
         uint32_t index                        /* index of the layer to remove */
);

/*
 * Changes the position of a layer in the level's layer-stack.
 * If the layer has a flag set that fixes its size inside the
 * stack, this function does nothing. To change the position
 * of such a layer, first clear the flag, call this function,
 * and optionally restore the flag.
 * NOTE: Currently, no custom flags are supported.
 * 
 * Returns the old position.
 */
MB_API uint32_t marble_levelasset_movelayer(
    _In_ union marble_levelasset *ps_lvlasset, /* level asset */
         uint32_t index,                       /* layer index */
         uint32_t newpos                       /* new position */
);

/*
 * Inserts an entity (tile, etc.) into a given position on the
 * level. If an entity already exists on that given position,
 * it will be replaced.
 * Pass NULL for **ps_entity** to remove the entity at the given
 * position.
 * This function expects **xpos** and **ypos** to be in tile
 * coordinates.
 * 
 * Returns nothing.
 */
MB_API void marble_levelasset_setentity(
    _In_ union marble_levelasset *ps_lvlasset, /* level asset */
         uint32_t index,                       /* layer index */
         uint32_t xpos,                        /* xpos, in tiles */
         uint32_t ypos,                        /* ypos, in tiles */
         /*
          * entity; the function will automatically choose the right part of
          * the union depending on the type of **ps_layer**. Passing NULL for
          * this parameter will cause the function to remove the entity at
          * the given position, if there is any.
          */
         union marble_levelasset_chunkentity *ps_entity
);
#endif


MB_END_HEADER


