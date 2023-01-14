#pragma once

#include <asset.h>


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


/* main type declarations */
union marble_levelasset;
struct marble_levelasset_chunk;


#if (defined MB_DYNAMIC_LIBRARY) || (defined MB_ECOSYSTEM)
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
MB_API _Critical_ marble_ecode_t marble_levelasset_addlayer(
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
 * Retrieves a specific tile entity from a given layer, using
 * world coordinates.
 * 
 * Returns 0 on success, non-zero on failure. If the function
 * fails, all parameters marked by "_Out_" will be initialized
 * to 0. This signifies an error since 0 is not a valid
 * dependency index.
 */
MB_API _Success_ok_ marble_ecode_t marble_levelasset_getstatictexture(
    _In_  union marble_levelasset *ps_lvlasset, /* level asset */
          uint32_t lindex,                      /* layer index */
          uint32_t xpos,                        /* xpos, in tiles */
          uint32_t ypos,                        /* ypos, in tiles */
    _Out_ uint16_t *p_depindex,                 /* dependency index */
    _Out_ uint8_t *p_depx,                      /* x-pos of the tile in the dependency */
    _Out_ uint8_t *p_depy                       /* y-pos of the tile in the dependency */
);

/*
 * Inserts a static texture into the given layer. If the layer
 * is not a texture layer, the function will not do anything.
 * If an entity already exists on that given position,
 * it will be replaced.
 * If **depindex**, **depx**, or **depy** are invalid, the
 * function will do nothing.
 *
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_levelasset_setstatictexture(
    _In_ union marble_levelasset *ps_lvlasset, /* level asset */
         uint32_t lindex,                      /* layer index */
         uint32_t xpos,                        /* xpos, in tiles */
         uint32_t ypos,                        /* ypos, in tiles */
    _In_ uint16_t depindex,                    /* index of the dependency in the dependency table */
         uint8_t depx,                         /* xpos of the tile in the dependency, in tile coordinates */
         uint8_t depy                          /* ypos of the tile in the dependency, in tile coordinates */
);
#endif


MB_END_HEADER


