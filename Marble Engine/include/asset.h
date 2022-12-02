#pragma once

#include <api.h>


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
_Critical_ MB_API marble_ecode_t marble_asset_createnew(
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
_Critical_ MB_API marble_ecode_t marble_asset_loadfromfile(
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
_Success_ptr_ MB_API struct marble_asset *marble_asset_exists(
         bool addref,                         /* add ref to potentially found asset? */
    _In_ struct marble_assetman *ps_assetman, /* assetman to look in */
    _In_ marble_uuid_t *p_uuid                /* UUID to look for */
);

/*
 * Increments the ref-count of the given asset.
 * 
 * Returns **ps_asset**.
 */
_Success_ptr_ MB_API struct marble_asset *marble_asset_addref(
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


MB_END_HEADER


