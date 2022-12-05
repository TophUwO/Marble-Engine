#include <application.h>
#include <level.h>


#pragma region ASSETSUBTYPE
/*
 * static data-structure holding information about each
 * asset sub-type
 */
static struct {
    enum marble_asset_type type; /* asset type */
    
    size_t size;           /* size of complete sub-type, in bytes */
    void  *fn_create;      /* sub-type constructor */
    void  *fn_destroy;     /* sub-type destructor */
    void  *fn_crpvalidate; /* create-params validator */
    void  *fn_load;        /* load-from-resource callback */
} const glsa_assetsubtypeinfos[] = {
    { MARBLE_ASSETTYPE_UNKNOWN, 0 },

    { MARBLE_ASSETTYPE_LEVEL,
        sizeof(union marble_levelasset),
        (void *)&MB_CREATEFN(levelasset),
        (void *)&MB_DESTROYFN(levelasset),
        (void *)&MB_VALIDATECRPSFN(levelasset),
        (void *)&MB_LOADFN(levelasset)
    }
};
#pragma endregion (ASSETSUBTYPE)


#pragma region GENERICASSET
/*
 * Common asset container file head. Assets are always
 * organized in this container format so we can merge
 * files into one package more easily. This can simplify
 * the resolving of dependencies.
 * 
 * All fields are little endian.
 */
struct marble_resource_head {
    uint8_t  ma_magic[8]; /* magic number " mb_res " */
    uint64_t m_fsize;     /* total file size, in bytes */
    uint32_t m_nframes;   /* number of frames, i.e. embedded resources */
    uint32_t m_taboff;    /* offset to the frame table */
};

struct marble_resource_frame {
    uint8_t       ma_magic[8]; /* magic number " mb_fra " */
    uint64_t      m_frsize;    /* size of the frame, in bytes */
    marble_uuid_t mu_uuid;     /* resource identifier */
    uint32_t      m_flags;     /* optional flags */
    uint16_t      m_type;      /* resource type */
    uint16_t      m_ndep;      /* number of dependencies */
    uint64_t      m_dtabsize;  /* size of deptable, in bytes */
};


static void marble_asset_internal_destroy(
    _Uninit_(pps_asset) struct marble_asset **pps_asset
);

/*
 * Checks whether asset type is valid.
 * 
 * Returns true if asset type is valid, false if not.
 */
static bool marble_asset_internal_isvalidtype(
    enum marble_asset_type type /* type to check */
) {
    return type < __MARBLE_NUMASSETTYPES__ && type > MARBLE_ASSETTYPE_UNKNOWN;
}

/*
 * Gets the size of the asset subtype specified by **type**.
 * 
 * Returns the size of the subtype, in bytes. If the type is
 * unknown, the function returns 0.
 */
static size_t marble_asset_internal_getsubtypesize(
    _In_ enum marble_asset_type type /* asset type */
) {
    if (!marble_asset_internal_isvalidtype(type))
        return 0;

    return glsa_assetsubtypeinfos[type].size;
}

/*
 * Creates a generic asset, holding basic information needed
 * to initialize sub-types.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static _Critical_ marble_ecode_t marble_asset_internal_creategenericasset(
    _In_              struct marble_assetman *ps_refassetman,     /* reference to parent */
    _In_              struct marble_resource_frame *ps_framehead, /* frame head */
    _Init_(pps_asset) struct marble_asset **pps_asset             /* pointer to receive asset pointer */
) { MB_ERRNO
    if (ps_refassetman == NULL || ps_framehead == NULL || pps_asset == NULL)
        return MARBLE_EC_INTERNALPARAM;
    *pps_asset = NULL;

    size_t const stsize = marble_asset_internal_getsubtypesize(ps_framehead->m_type);
    if (stsize == 0) {
        ecode = MARBLE_EC_ASSETTYPE;

        goto lbl_END;
    }

    /* Allocate memory for the new asset. */
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        stsize,
        true,
        false,
        pps_asset
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

    (*pps_asset)->m_refcount    = 1;
    (*pps_asset)->m_type        = ps_framehead->m_type;
    (*pps_asset)->m_flags       = ps_framehead->m_flags;
    (*pps_asset)->mps_refparent = ps_refassetman;
    
    /* Copy UUID from head into tmp. */
    marble_uuid_t u_tmpuuid;
    marble_uuid_copy(
        &u_tmpuuid,
        &ps_framehead->mu_uuid
    );

    /*
     * If the copied UUID is a NULL-UUID, generate
     * a new one. We compare against the UUID field
     * in the newly-allocated asset structure, as it
     * is zeroed by the allocator.
     */
    if (memcmp(&(*pps_asset)->mu_uuid, &u_tmpuuid, sizeof u_tmpuuid) == 0)
        marble_uuid_generate(&u_tmpuuid);

    /* Copy the valid UUID to the asset structure. */
    marble_uuid_copy(&(*pps_asset)->mu_uuid, &u_tmpuuid);

    ecode = marble_util_vec_create(
        ps_framehead->m_ndep,
        NULL,
        &(*pps_asset)->mps_deps
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

lbl_END:
    if (ecode != MARBLE_EC_OK)
        marble_asset_internal_destroy(pps_asset);

    return ecode;
}

/*
 * Sets or clears the given bit-flag of the specified asset.
 * 
 * Returns old value (true or false), or false on error.
 */
static bool marble_asset_internal_setflag(
    _In_ struct marble_asset *ps_asset, /* asset to modify state of */ 
         enum marble_asset_flags flag,  /* flag to set/clear */
         bool enable                    /* (true = set, false = clear) flag */
) {
    if (ps_asset == NULL)
        return false;

    /* Save old state of flag. */
    bool old = (bool)(ps_asset->m_flags & flag);

    /* Update state. */
    if (enable)
        ps_asset->m_flags |= flag;
    else
        ps_asset->m_flags &= ~flag;

    return old;
}

/*
 * Loads the dependencies of an asset, according to its dependency
 * table.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static _Critical_ marble_ecode_t marble_asset_internal_loaddeps(
    _In_ struct marble_util_file *ps_fdesc, /* file descriptor */
    _In_ struct marble_asset *ps_parent     /* parent asset */
) { MB_ERRNO
    // TODO: read dep-info
    // locate dep
    // check if dep is registered
    // load dep, including its deps

lbl_END:
    return ecode;
}
#pragma endregion (GENERICASSET)


#pragma region CONTAINERFMT
/*
 * Opens a container file, establishes a file descriptor,
 * and reads the resource container head.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static _Critical_ marble_ecode_t marble_container_internal_open(
    _In_z_            char const *pz_fname,                 /* file name, NUL-terminated */
    _Init_(pps_fdesc) struct marble_util_file **pps_fdesc,  /* pointer to a file descriptor */
    _Out_             struct marble_resource_head *ps_rhead /* pointer to the resource head */
) { MB_ERRNO
    if (pz_fname == NULL || *pz_fname == '\0' || pps_fdesc == NULL || ps_rhead == NULL)
        return MARBLE_EC_INTERNALPARAM;

    /* Open file descriptor. */
    ecode = marble_util_file_open(pz_fname, 0, pps_fdesc);
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

    /* Read container head. */
    ecode = marble_util_file_read(
        *pps_fdesc,
        sizeof *ps_rhead,
        ps_rhead
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

    /*
     * Check whether file size is congruent with the file
     * size written into the file itself.
     */
    struct _stat64 s_info = { 0 };
    marble_util_file_getinfo(*pps_fdesc, &s_info);

    if ((uint64_t)s_info.st_size != ps_rhead->m_fsize) {
        ecode = MARBLE_EC_FILESIZE;

        goto lbl_END;
    }

lbl_END:
    if (ecode != MARBLE_EC_OK)
        marble_util_file_destroy(pps_fdesc);

    return ecode;
}

/*
 * Loads a frame header into memory.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static _Success_ok_ marble_ecode_t marble_container_internal_getnextframehead(
    _In_  struct marble_util_file *ps_fdesc,     /* file descriptor */
    _Out_ struct marble_resource_frame *ps_frame /* pointer to the frame head */
) {
    if (ps_fdesc == NULL || ps_frame == NULL)
        return MARBLE_EC_INTERNALPARAM;

    /* Read frame head. */
    return marble_util_file_read(ps_fdesc, sizeof *ps_frame, ps_frame);
}

/*
 * Loads a frame (i.e. a resource/asset, etc.) into memory.
 * If the resource is already loaded, the pointer pointed to
 * by **pps_asset** will be populated with the currently loaded
 * instance of the resource.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static _Success_ok_ marble_ecode_t marble_container_internal_loadframe(
    _In_              struct marble_assetman *ps_assetman,    /* asset manager */
    _In_              struct marble_util_file *ps_fdesc,      /* file descriptor */
    _In_              struct marble_resource_frame *ps_frame, /* pointer to the frame head */
    _Init_(pps_asset) struct marble_asset **pps_asset         /* asset pointer */
) { MB_ERRNO
    if (ps_fdesc == NULL || ps_frame == NULL || pps_asset == NULL)
        return MARBLE_EC_INTERNALPARAM;
    
    // TODO: add code
    
    return MARBLE_EC_UNIMPLFEATURE;
}
#pragma endregion (CONTAINERFMT)


#pragma region ASSETMAN
/*
 * implementation of assetman type 
 */
struct marble_assetman {
    bool m_isinit; /* Is the asset manager present? */

    /*
     * sub-component of the asset manager dedicated to
     * managing the loading and organizing of level
     * resources
     */
    struct marble_assetman_levelman {
        /* reference to the currently focused level */
        union marble_levelasset *mpu_currlevel;
        
        /* memory pool for keeping the chunk resources */
        struct marble_util_vec *mps_pools;
    } ms_levelman;

    struct marble_util_htable *mps_table; /* asset storage */
};

/*
 * memory pool for level manager
 */
struct marble_assetman_mempool {
    size_t m_size;       /* size of entire memory pool, in bytes */
    size_t m_firstempty; /* first empty element */
    size_t m_nobjs;      /* number of objects currently allocated */
    size_t m_objsize;    /* object size, in bytes */

    void *mps_data;      /* pointer to the actual memory block */
};


#pragma region LEVELMAN
#define MB_ASSETMAN_MEMPOOL_FREE  (0)
#define MB_ASSETMAN_MEMPOOL_ALLOC (1)
#define MB_ASSETMAN_MEMPOOL_COUNT (1024)

/*
 * Destroys a memory pool used by the level manager.
 * 
 * Returns nothing.
 */
static void marble_assetman_internal_destroymempool(
    _Uninit_(pps_mempool) struct marble_assetman_mempool **pps_mempool /* memory pool to destroy */
) {
    if (pps_mempool == NULL || *pps_mempool == NULL)
        return;

    free((*pps_mempool)->mps_data);

    free(*pps_mempool);
    *pps_mempool = NULL;
}

/*
 * Creates a new memory pool.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static _Critical_ marble_ecode_t marble_assetman_internal_createmempool(
    _In_                size_t size,                                 /* size of pool, in objects */
    _Init_(pps_mempool) struct marble_assetman_mempool **pps_mempool /* memory pool to create */
) { MB_ERRNO
    if (pps_mempool == NULL)
        return MARBLE_EC_INTERNALPARAM;

    /* Allocate memory for memory pool structure. */
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        sizeof **pps_mempool,
        true,
        false,
        pps_mempool
    );
    if (ecode != MARBLE_EC_OK)
        return ecode;

    /* Init object size. */
    (*pps_mempool)->m_objsize = sizeof(struct marble_levelasset_chunk);

    /* Allocate actual pool. */
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        size * (*pps_mempool)->m_objsize,
        true,
        false,
        &(*pps_mempool)->mps_data
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

    /* Initialize state. */
    (*pps_mempool)->m_size       = size * (*pps_mempool)->m_objsize;
    (*pps_mempool)->m_firstempty = 0;
    (*pps_mempool)->m_nobjs      = 0;

lbl_END:
    if (ecode != MARBLE_EC_OK)
        marble_assetman_internal_destroymempool(pps_mempool);

    return ecode;
}

/*
 * Initializes the level manager sub-component of the
 * asset manager.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static _Critical_ marble_ecode_t marble_assetman_internal_initlevelman(
    _Inout_ struct marble_assetman *ps_assetman /* assetman to init levelman of */
) { MB_ERRNO
    if (ps_assetman == NULL)
        return MARBLE_EC_INTERNALPARAM;
    
    /* Allocate memory for the pool organizer. */
    ecode = marble_util_vec_create(
        8,
        (marble_dtor_t)&marble_assetman_internal_destroymempool,
        &ps_assetman->ms_levelman.mps_pools
    );
    if (ecode != MARBLE_EC_OK)
        return ecode;

    /* NULL signifies that no level is currently loaded. */
    ps_assetman->ms_levelman.mpu_currlevel = NULL;

    return MARBLE_EC_OK;
}


/*
 * Requests a new chunk from the level manager.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ marble_ecode_t marble_assetman_internal_allocatechunk(
    _In_              struct marble_assetman *ps_assetman,       /* asset manager */
    _Init_(pps_chunk) struct marble_levelasset_chunk **pps_chunk /* address of chunk */
) { MB_ERRNO
    if (ps_assetman == NULL || !marble_assetman_isok(ps_assetman))
        return MARBLE_EC_INTERNALPARAM;

    /* Get current number of memory pools. */
    size_t const len = marble_util_vec_count(ps_assetman->ms_levelman.mps_pools);

    struct marble_assetman_mempool *ps_mempool = NULL;
    /*
     * Find first non-empty pool out of the available
     * ones.
     */
    for (size_t i = 0; i < len; i++) {
        MB_VOIDCAST(
            ps_tmp,
            marble_util_vec_get(ps_assetman->ms_levelman.mps_pools, i),
            struct marble_assetman_mempool
        );
        if (ps_tmp == NULL)
            continue;

        /* Check if pool is empty. */
        if (ps_tmp->m_nobjs < ps_tmp->m_size / ps_tmp->m_objsize) {
            ps_mempool = ps_tmp;

            break;
        }
    }

    /*
     * If
     *  (1) all currently available pools are full or
     *  (2) there are no pools available yet,
     * create a new pool.
     */
    if (ps_mempool == NULL) {
        /* Create pool. */
        ecode = marble_assetman_internal_createmempool(
            MB_ASSETMAN_MEMPOOL_COUNT,
            &ps_mempool
        );
        if (ecode != MARBLE_EC_OK)
            goto lbl_END;

        /* Add memory pool to manager. */
        ecode = marble_util_vec_pushback(
            ps_assetman->ms_levelman.mps_pools,
            ps_mempool
        );
        if (ecode != MARBLE_EC_OK) {
            marble_assetman_internal_destroymempool(&ps_mempool);

            goto lbl_END;
        }

        /* Output info message. */
        marble_log_debug(
            __func__,
            "Created memory pool (%zu bytes) at 0x%p.",
            ps_mempool->m_size,
            ps_mempool
        );
    }

    /* Calculate chunk offset. */
    *pps_chunk = &((struct marble_levelasset_chunk *)ps_mempool->mps_data)[ps_mempool->m_firstempty];

    /*
     * Find next empty chunk in the pool.
     * If the chunk is now full, we do not update the pool state
     * as the next chunk allocation will automatically create
     * a new pool.
     */
    for (size_t i = ps_mempool->m_firstempty + 1; i < (ps_mempool->m_size / ps_mempool->m_objsize); i++) {
        struct marble_levelasset_chunk *ps_tmp = &((struct marble_levelasset_chunk *)ps_mempool)[i];

        if (ps_tmp->m_nents == MB_ASSETMAN_MEMPOOL_FREE) {
           ps_mempool->m_firstempty = i;

           break;
        }
    }

    /* Mark chunk as allocated. */
    (*pps_chunk)->m_nents = MB_ASSETMAN_MEMPOOL_ALLOC;

lbl_END:
    if (ecode != MARBLE_EC_OK)
        *pps_chunk = NULL;

    return ecode;
}
#pragma endregion (LEVELMAN)


/*
 * Find callback for finding an asset inside a given asset manager.
 * 
 * For information on return values, consult the documentation
 * of the "marble_util_htable_find()" function.
 */
static bool marble_assetman_internal_cbfind(
    _In_ void *p_key,                  /* key */
    _In_ struct marble_asset *ps_asset /* asset to compare */
) {
    if (ps_asset == NULL)
        return false;

    return 
        memcmp(
            p_key,
            &ps_asset->mu_uuid,
            sizeof ps_asset->mu_uuid
        ) == 0;
}

/*
 * Add the asset to the asset storage temporarily. This allows
 * us to keep track of all the assets we have either finished
 * loading or are currently loading.
 * 
 * Returns true on success, false on failure.
 */
static bool marble_assetman_internal_regtmpasset(
    _In_ struct marble_assetman *ps_assetman,
    _In_ struct marble_asset *ps_asset
) {
    if (ps_assetman == NULL || ps_asset == NULL || !marble_assetman_isok(ps_assetman))
        return false;

    /* Set "loading" flag. */
    marble_asset_internal_setflag(ps_asset, MARBLE_ASSETFLAG_LOADING, true);

    /* Add asset to storage. */
    return marble_util_htable_insert(
        ps_assetman->mps_table,
        &ps_asset->mu_uuid,
        sizeof ps_asset->mu_uuid,
        ps_asset
    ) == MARBLE_EC_OK;
}


_Critical_ marble_ecode_t marble_assetman_create(
    _Init_(pps_assetman) struct marble_assetman **pps_assetman
) { MB_ERRNO
    if (pps_assetman == NULL)
        return MARBLE_EC_PARAM;

    /* Allocate memory for assetman. */
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        sizeof **pps_assetman,
        true,
        false,
        pps_assetman
    );
    if (ecode != MARBLE_EC_OK)
        return ecode;

    /* Create asset registry. */
    ecode = marble_util_htable_create(
        0,
        (void (*)(void **))&marble_asset_internal_destroy,
        &(*pps_assetman)->mps_table
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

    /* Create level manager. */
    ecode = marble_assetman_internal_initlevelman(*pps_assetman);
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

    /* Update init-state. */
    (*pps_assetman)->m_isinit = true;

lbl_END:
    if (ecode != MARBLE_EC_OK)
        marble_assetman_destroy(pps_assetman);

    return ecode;
}

void marble_assetman_destroy(
    _Uninit_(pps_assetman) struct marble_assetman **pps_assetman
) {
    if (pps_assetman == NULL || *pps_assetman == NULL)
        return;

    /*
     * Destroy registry.
     * All assets will be destroyed as well.
     */
    marble_util_htable_destroy(&(*pps_assetman)->mps_table);

    /* Destroy level manager. */
    marble_util_vec_destroy(&(*pps_assetman)->ms_levelman.mps_pools);

    /* Destroy asset manager structure. */
    free(*pps_assetman);
    *pps_assetman = NULL;
}

bool marble_assetman_isok(
    _In_ struct marble_assetman *ps_assetman
) {
    if (ps_assetman == NULL)
        return false;

    return ps_assetman->m_isinit;
}
#pragma endregion (ASSETMAN)


#pragma region ASSET
/*
 * Checks whether the **p_createparams** parameter of
 * asset functions is valid for a specific asset
 * type.
 * 
 * Returns true if the parameter has to be valid, false
 * if not.
 */
static bool marble_asset_internal_iscrpsvalid(
    _In_     enum marble_asset_type type, /* asset type */
    _In_opt_ void *p_createparams         /* create-params to validate */
) {
    if (!marble_asset_internal_isvalidtype(type))
        return false;

    /* Get create-params validation function. */
    MB_FNCAST(fn_validate, glsa_assetsubtypeinfos[type].fn_crpvalidate, bool, (void *));
    if (fn_validate == NULL)
        return true;

    return (*fn_validate)(p_createparams);
}

/*
 * Destroys the given asset and deallocates all resources
 * used by the asset.
 * 
 * Returns nothing.
 */
static void marble_asset_internal_destroy(
    _Uninit_(pps_asset) struct marble_asset **pps_asset /* asset to destroy */
) {
    if (pps_asset == NULL || *pps_asset == NULL)
        return;

    /* Destroy dependency table */
    marble_util_vec_destroy(&(*pps_asset)->mps_deps);

    /* Get type-specific destructor and run it. */
    MB_FNCAST(fn_destroy, glsa_assetsubtypeinfos[(*pps_asset)->m_type].fn_destroy, void, (void **));
    if (fn_destroy != NULL)
        (*fn_destroy)(pps_asset);

    /*
     * Remove asset from asset storage if it is
     * still associated with one.
     */
    if ((*pps_asset)->mps_refparent != NULL)
        marble_util_htable_erase(
            (*pps_asset)->mps_refparent->mps_table,
            &(*pps_asset)->mu_uuid,
            sizeof (*pps_asset)->mu_uuid,
            (bool (*)(void *, void *))&marble_assetman_internal_cbfind,
            false
        );

    /* Destroy asset structure. */
    free(*pps_asset);
    *pps_asset = NULL;
}

/*
 * Decreases the ref-count of every dependency inside the asset's
 * dependency table when the asset gets destroyed.
 * 
 * Returns nothing.
 */
static void marble_asset_internal_cbdecrefcount(
    struct marble_asset *ps_asset /* asset to decrease ref-count of */
) {
    if (ps_asset == NULL)
        return;

    marble_asset_release(ps_asset);
}


_Critical_ marble_ecode_t marble_asset_createnew(
    _In_              enum marble_asset_type type,
    _In_opt_          int flags,
    _In_opt_          void *p_createparams,
    _In_              struct marble_assetman *ps_parent,
    _Init_(pps_asset) struct marble_asset **pps_asset
) { MB_ERRNO
    if (pps_asset == NULL)                                        return MARBLE_EC_PARAM;
    if (!marble_asset_internal_isvalidtype(type))                 return MARBLE_EC_ASSETTYPE;
    if (!marble_assetman_isok(ps_parent))                         return MARBLE_EC_COMPSTATE;
    if (!marble_asset_internal_iscrpsvalid(type, p_createparams)) return MARBLE_EC_CREATEPARAMS;

    /* Create generic asset structure. */
    struct marble_resource_frame s_frame = {
        .m_flags = flags,
        .m_type  = type
    };
    ecode = marble_asset_internal_creategenericasset(
        ps_parent,
        &s_frame,
        pps_asset
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

    /* Get asset-specific constructor and run it. */
    MB_FNCAST(fn_create, glsa_assetsubtypeinfos[type].fn_create, marble_ecode_t, (void *, struct marble_asset *));
    if (fn_create != NULL) {
        ecode = (*fn_create)(p_createparams, *pps_asset);

        if (ecode != MARBLE_EC_OK)
            goto lbl_END;
    }

    /* Register asset. */
    ecode = marble_util_htable_insert(
        ps_parent->mps_table,
        &(*pps_asset)->mu_uuid,
        sizeof (*pps_asset)->mu_uuid,
        *pps_asset
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

lbl_END:
    if (ecode != MARBLE_EC_OK)
        marble_asset_internal_destroy(pps_asset);

    return ecode;
}

_Critical_ marble_ecode_t marble_asset_loadfromfile(
    _In_z_ char const *pz_path,
    _In_   struct marble_assetman *ps_assetman
) { MB_ERRNO
    if (pz_path == NULL || *pz_path == '\0') return MARBLE_EC_PARAM;
    if (!marble_assetman_isok(ps_assetman))  return MARBLE_EC_COMPSTATE;

    int64_t currpos;
    struct marble_util_file *ps_fdesc   = NULL;
    struct marble_resource_head s_rhead = { 0 };

    /* Open file stream and read common head. */
    ecode = marble_container_internal_open(
        pz_path,
        &ps_fdesc,
        &s_rhead
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;

    /* Return if there is nothing to do. */
    if (s_rhead.m_nframes == 0)
        goto lbl_END;

    /* Go to the start of the first frame head. */
    marble_util_file_goto(ps_fdesc, s_rhead.m_taboff);

    /* Load all frames (if there are any) in order. */
    for (uint32_t i = 0; i < s_rhead.m_nframes; i++) {
        struct marble_asset *ps_asset        = NULL;
        struct marble_resource_frame s_frame = { 0 };

        /* Read frame head. */
        ecode = marble_util_file_read(ps_fdesc, sizeof s_frame, &s_frame);
        if (ecode != MARBLE_EC_OK)
            break;

        /*  Create generic asset from asset head. */
        ecode = marble_asset_internal_creategenericasset(
            ps_assetman,
            &s_frame,
            &ps_asset
        );
        if (ecode == MARBLE_EC_MEMALLOC)
            break;
        else if (ecode != MARBLE_EC_OK) {
            /*
             * If the loading fails, but it's not fatal, we still
             * want to load more frames if there are any. So we
             * have to jump to the next one first.
             */
            if ((currpos = marble_util_file_tell(ps_fdesc)) != -1)
                marble_util_file_goto(
                    ps_fdesc,
                    currpos + s_frame.m_frsize
                );

            continue;
        }

        /*
         * Register asset as temporary. Once the asset and all
         * its (possibly) recursive dependencies are loaded,
         * the asset will have its "tmp" flag cleared.
         * In case there are circular dependencies, this allows
         * us to only resolve assets we have not yet started to
         * load. In any case, circular dependencies will be
         * optimized away.
         */
        if (!marble_assetman_internal_regtmpasset(ps_assetman, ps_asset)) {
            ecode = MARBLE_EC_REGTMPASSET;

            break;
        }

        /* Load dependencies. */
        ecode = marble_asset_internal_loaddeps(ps_fdesc, ps_asset);
        if (ecode != MARBLE_EC_OK) {
            marble_asset_internal_destroy(&ps_asset);

            goto lbl_END;
        }

        /* Load the actual asset. */
        ecode = MB_LOADFN(levelasset)(ps_fdesc, (union marble_levelasset *)ps_asset);
        if (ecode != MARBLE_EC_OK) {
            marble_asset_internal_destroy(&ps_asset);

            goto lbl_END;
        }

        /*
         * Clear "loading" flag now that we have loaded all
         * dependencies. If we come to this point, the asset
         * has been successfully loaded.
         */
        marble_asset_internal_setflag(ps_asset, MARBLE_ASSETFLAG_LOADING, false);
    }

lbl_END:
    /* Close file. */
    marble_util_file_destroy(&ps_fdesc);

    return ecode;
}

_Success_ptr_ struct marble_asset *marble_asset_exists(
         bool addref,
    _In_ struct marble_assetman *ps_assetman,
    _In_ marble_uuid_t *p_uuid
) {
    if (p_uuid == NULL || !marble_assetman_isok(ps_assetman))
        return NULL;

    /* Locate the asset inside the given hashtable. */
    struct marble_asset *ps_asset = marble_util_htable_find(
        ps_assetman->mps_table,
        (void *)p_uuid,
        sizeof *p_uuid,
        (bool (*)(void *, void *))&marble_assetman_internal_cbfind
    );
    if (ps_asset == NULL)
        return NULL;

    /* Add a reference if the **addref** parameter is true. */
    if (addref == true)
        marble_asset_addref(ps_asset);

    return ps_asset; 
}

_Success_ptr_ struct marble_asset *marble_asset_addref(
    _Inout_ struct marble_asset *ps_asset
) {
    if (ps_asset == NULL)
        return NULL;

    ++ps_asset->m_refcount;

    return ps_asset;
}

void marble_asset_release(
    _In_ struct marble_asset *ps_asset
) {
    if (ps_asset == NULL)
        return;

    /*
     * If ref-count reaches zero, add it
     * to the assetman's waste-bin.
     */
    if (--ps_asset->m_refcount == 0) {
        // delete asset
    }
}
#pragma endregion (ASSET)


