#pragma once

#include <uuid.h>


MB_BEGIN_HEADER


/* asset type IDs */
enum marble_asset_type {
    MARBLE_ASSETTYPE_UNKNOWN = 0, /* reserved */

    MARBLE_ASSETTYPE_LEVEL,       /* level/map asset */

#if (defined MB_DYNAMIC_LIBRARY)
    __MARBLE_NUMASSETTYPES__      /* for internal use */
#endif
};

/* asset flags */
enum marble_asset_flags {
    MARBLE_ASSETFLAG_DEFAULT = 0,     /* default state */

    MARBLE_ASSETFLAG_LOADING = 1 << 0 /* asset currently loading */
};


/*
 * generic asset type
 * 
 * Every specialized asset type inherits all
 * the fields of this type.
 */
struct marble_asset {
    enum marble_asset_type m_type; /* asset type ID */

    uint32_t                m_flags;       /* flags */
    uint32_t                m_refcount;    /* internal ref-count */
    marble_uuid_t           mu_uuid;       /* UUID */
    struct marble_assetman *mps_refparent; /* reference to the parent */
    struct marble_util_vec *mps_deps;      /* dependencies loaded by the asset */
};

/*
 * structure representing hard limits regarding all
 * currently implemented asset types
 */
struct marble_asset_limits {
    /*
     * structure describing the hard limits of the current
     * version of Marble Engine regarding the level asset
     */
    MB_ASSETTYPELIMITS(levelasset) {
        uint32_t m_minwidth;    /* minimum width, in chunks */
        uint32_t m_minheight;   /* minimum height, in chunks */
        uint32_t m_maxwidth;    /* maximum width, in chunks */
        uint32_t m_maxheight;   /* maximum height, in chunks */
        uint32_t m_maxlayers;   /* maximum number of layers per level */
        uint32_t m_maxtilesets; /* maximum number of tilesets a level can use */
        uint32_t m_chunkwidth;  /* chunk width, in tiles */
        uint32_t m_chunkheight; /* chunk height, in tiles */
    } ms_lvllimits;
};


/*
 * Creates an asset manager.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Critical_ marble_ecode_t marble_assetman_create(
    /*
     * pointer to receive a pointer to the
     * newly created assetman
     */
    _Init_(pps_assetman) struct marble_assetman **pps_assetman
);

/*
 * Destroys the given asset-manager. All assets that are still registered
 * will be destroyed. All references to these assets will become invalid.
 * 
 * Returns nothing.
 */
MB_API void marble_assetman_destroy(
    _Uninit_(pps_assetman) struct marble_assetman **pps_assetman /* assetman to uninitialize */
);

/*
 * Checks whether the given assetman is ready.
 * 
 * Returns true if the assetman is ready, false if not.
 */
MB_API bool marble_assetman_isok(
    _In_ struct marble_assetman *ps_assetman /* assetman to get state of */
);


/*
 * Creates an asset of a specific type.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Critical_ marble_ecode_t marble_asset_createnew(
    _In_              enum marble_asset_type type,         /* asset type */
    _In_opt_          int flags,                           /* opt. flags */
    _In_opt_          void *p_createparams,                /* create-params */
    _In_              struct marble_assetman *ps_assetman, /* asset manager */
    _Init_(pps_asset) struct marble_asset **pps_asset      /* asset pointer */
);

/*
 * Loads an asset from a file path. If the file contains multiple assets,
 * all assets are loaded in the order they are written. After the
 * asset is loaded, it is associated with **ps_assetman**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Critical_ marble_ecode_t marble_asset_loadfromfile(
    _In_z_ char const *pz_path,                /* file path */
    _In_   struct marble_assetman *ps_assetman /* asset manager */
);

/*
 * Checks whether a given asset UUID is already registered.
 * If the asset is registered in the given asset-manager,
 * and **addref** is true, the ref-count is incremented before
 * the function returns.
 *
 * Returns the asset on success, NULL on failure.
 */
MB_API _Success_ptr_ struct marble_asset *marble_asset_exists(
         bool addref,                         /* add ref to potentially found asset? */
    _In_ struct marble_assetman *ps_assetman, /* assetman to look in */
    _In_ marble_uuid_t *p_uuid                /* UUID to look for */
);

/*
 * Increments the ref-count of the given asset.
 * 
 * Returns **ps_asset**.
 */
MB_API _Success_ptr_ struct marble_asset *marble_asset_addref(
    _Inout_ struct marble_asset *ps_asset /* asset to increase ref-count of */
);

/*
 * Decrements the ref-count of the given asset.
 * If the asset's ref-count hits 0 as a result of this
 * function, the asset will be scheduled for deletion.
 * When the function returns to the main-loop, the asset
 * destroyed.
 * 
 * Returns nothing.
 */
MB_API void marble_asset_release(
    _In_ struct marble_asset *ps_asset /* asset to decrease ref-count of */
);

/*
 * Queries the hard limits for each asset type and stores
 * them inside **ps_limits**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_asset_queryhardlimits(
    _In_           size_t ssize,                         /* size of **ps_limits**, in bytes */
    _Outsz_(ssize) struct marble_asset_limits *ps_limits /* pointer to a structure to receive limits values */
);

/*
 * Adds a dependency to an asset.
 *
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_asset_adddependency(
    _In_ struct marble_asset *ps_asset, /* asset to add dependency to */
    _In_ struct marble_asset *ps_dep    /* dependency to add */ 
);

/*
 * Removes a dependency from the dep-chain
 * of a given asset.
 * If the dep is not part of the dep-chain, the
 * function does nothing.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_asset_remdependency(
    _In_ struct marble_asset *ps_asset, /* asset to remove dependency from */
    _In_ struct marble_asset *ps_dep    /* dependency to remove */ 
);


MB_END_HEADER


