#include <application.h>


void marble_asset_destroy(
	struct marble_asset **pps_asset
);

static marble_ecode_t marble_asset_internal_loadfromfile(
	char const *pz_path,
	struct marble_asset **pps_asset,
	struct marble_asset **pps_existingassetptr
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
 * Internal callback function used by the asset registry to locate
 * an asset. As assets have a unique name, we can just uniquely identfy and
 * locate an asset using its identifier.
 * 
 * Returns true if the asset is found, or false if not.
 */
static bool MB_CALLBACK marble_asset_internal_cbfind(
	char const *pz_key, /* key */
	void *p_asset       /* asset to check key against */
) {
	return (bool)!strcmp(pz_key, ((struct marble_asset *)p_asset)->mz_strid);
}

/*
 * Checks if asset with a given ID is already loaded.
 * 
 * Returns true if asset is already loaded, false if not. If
 * **pps_existingassetptr** is not NULL and the asset exists,
 * **pps_existingassetptr** will receive the pointer to the
 * found asset. Otherwise, the pointer value of **pps_existingassetptr**
 * is not changed and remains possibly indeterminate.
 */
static bool marble_asset_internal_isassetalreadyloaded(
	char const *pz_id,                         /* ID to check */
	struct marble_asset **pps_existingassetptr /* optional pointer to receive existing asset pointer */
) {
	struct marble_asset *ps_found = marble_util_htable_find(
		gl_app.ms_assets.mps_table,
		pz_id,
		&marble_asset_internal_cbfind
	);

	if (ps_found != NULL && pps_existingassetptr != NULL)
		*pps_existingassetptr = ps_found;

	return ps_found != NULL;
}

/*
 * Copies the given string ID safely into the asset ID buffer.
 * 
 * Returns nothing.
 */
static void marble_asset_internal_setid(
	struct marble_asset *ps_asset, /* asset to copy new ID into */
	char const *pz_newid           /* new ID to copy */
) {
	marble_system_cpystr(
		ps_asset->mz_strid,
		pz_newid,
		MB_STRINGIDMAX
	);
}

/*
 * Checks whether the ID is usable, that is,
 *  (1) the ID is valid in that it is neither NULL nor its first character is NUL
 *  (2) the ID is not already used (i.e. an asset with that ID is already loaded)
 * 
 * Returns true if the ID is usable, false if not. If
 * **pps_existingassetptr** is not NULL and the asset exists,
 * **pps_existingassetptr** will receive the pointer to the
 * found asset. Otherwise, the pointer value of **pps_existingassetptr**
 * is not changed and remains possibly indeterminate.
 */
static bool marble_asset_internal_isusableid(
	char const *pz_id,                         /* ID to check for validity */
	struct marble_asset **pps_existingassetptr /* optional pointer to receive existing asset pointer */
) {
	if (pz_id == NULL || *pz_id == '\0')
		return FALSE;

	return !marble_asset_internal_isassetalreadyloaded(pz_id, pps_existingassetptr);
}

/*
 * Adds the asset to the asset-manager registry.
 * 
 * Returns 0 on success, non-zero on failure.
 */
static marble_ecode_t marble_asset_internal_register(
	struct marble_asset *ps_asset /* asset to register */
) { MB_ERRNO
	if (ps_asset == NULL || gl_app.ms_assets.m_isinit == false)
		return ps_asset ? MARBLE_EC_PARAM : MARBLE_EC_COMPSTATE;
	
	ecode = marble_util_htable_insert(
		gl_app.ms_assets.mps_table, 
		ps_asset->mz_strid, 
		ps_asset
	);
	if (ecode != MARBLE_EC_OK) {
		printf("AssetManager: Failed to register asset \"%s\" (type = %i); error code: %i (%s).\n",
			ps_asset->mz_strid,
			ps_asset->m_type,
			ecode,
			marble_error_getstr(ecode)
		);

		return ecode;
	}
	
	printf("AssetManager: Successfully registered asset \"%s\" (type = %i).\n",
		ps_asset->mz_strid,
		ps_asset->m_type
	);
	
	return MARBLE_EC_OK;
}

/*
 * Removes an asset from the registry.
 * 
 * The function can, additionally, invoke the asset
 * destructor to free-up all the resources the asset
 * occupies.
 * 
 * Returns non-zero in case of error, 0 on success.
 */
static marble_ecode_t marble_asset_internal_unregister(
	struct marble_asset *ps_asset, /* asset to register */
	bool dofree                    /* Destroy asset as well? */
) {
	if (ps_asset == NULL || gl_app.ms_assets.m_isinit == false)
		return ps_asset != NULL ? MARBLE_EC_PARAM : MARBLE_EC_COMPSTATE;

	void *p_asset = marble_util_htable_erase(
		gl_app.ms_assets.mps_table,
		ps_asset->mz_strid,
		&marble_asset_internal_cbfind,
		dofree
	);

	return p_asset != NULL ? MARBLE_EC_OK : MARBLE_EC_ASSETNOTFOUND;
}

/*
 * Increments ref-count of an asset; works only if the asset is not
 * internally marked as persistent.
 * 
 * Returns nothing.
 */
static void marble_asset_internal_addref(
	struct marble_asset *ps_asset /* asset to add ref to */
) {
	/* Only increment ref-count if it is not -1, meaning the asset is persistent. */
	if (ps_asset->m_refcount != -1)
		++ps_asset->m_refcount;
}

/*
 * Decrements the ref-count.
 * If the ref-count is -1, the asset is persistent, meaning
 * that the asset cannot be released and has to be
 * destroyed to be removed.
 * 
 * Returns nothing.
 */
static void marble_asset_internal_release(
	struct marble_asset *ps_asset /* asset to release */
) {
	if (ps_asset == NULL || ps_asset->m_refcount != -1)
		return;

	// TODO: add asset type id strings
	printf("AssetManager: Releasing asset \"%s\" (type = %i); current ref-count: %i.\n",
		ps_asset->mz_strid,
		ps_asset->m_type,
		ps_asset->m_type
	);

	/*
	 * If the asset's ref-count becomes 0, the asset will automatically 
	 * be unloaded and destroyed.
	 */
	if (--ps_asset->m_refcount == 0)
		marble_asset_internal_unregister(ps_asset, true);
}

/*
 * Sequentially scans the dependency table entries inside the asset file and
 * executes the commands given.
 * 
 * Returns 0 on success, non-zero on error.
 */
static marble_ecode_t marble_asset_internal_evaldeptable(
	struct marble_asset *ps_parent,   /* parent asset that contains the dependency table */
	struct marble_util_file *ps_file, /* open file stream */
	uint32_t numofdeps                /* number of dependency table entries */
) { MB_ERRNO
	if (numofdeps == 0)
		return MARBLE_EC_OK;

	/* Create asset vector in case there are asset dependencies to load */
	ecode = marble_util_vec_create(
		0,
		NULL,
		&ps_parent->mps_deps
	);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	/* Traverse asset dependency table. */
	for (uint32_t i = 0; i < numofdeps; i++) {
		/* Get the command. */
		uint8_t cmd = 0;
		marble_util_file_read8(ps_file, &cmd);

		/* Cxecute the command. */
		switch (cmd) {
			case MARBLE_ASSETDEPTABLECMD_LOADASSET: {
				// TODO: remove magic numbers
				char az_buffer[1024] = { '\0' };
				uint32_t ssize = 0;
				marble_util_file_read32(ps_file, &ssize);
				if (ssize > sizeof az_buffer - 1) {
					ecode = MARBLE_EC_PATHLENGTH;

					goto lbl_CLEANUP;
				}

				/* Read the path. */
				marble_util_file_read(ps_file, ssize, az_buffer);
				if (*az_buffer == '\0') {
					ecode = MARBLE_EC_EMPTYPATH;

					goto lbl_CLEANUP;
				}

				/* Try to load the asset. */
				struct marble_asset *ps_asset = NULL;
				ecode = marble_asset_internal_loadfromfile(
					az_buffer, 
					&ps_asset, 
					&ps_asset
				);
				if (ecode != MARBLE_EC_OK && ecode != MARBLE_EC_DUPEDASSET)
					goto lbl_CLEANUP;
				bool const isdupe = ecode == MARBLE_EC_DUPEDASSET;

				/* Increment the ref-count and add it to the dependency list. */
				marble_asset_internal_addref(ps_asset);
				marble_util_vec_pushback(ps_parent->mps_deps, ps_asset);

				/* 
				 * Only try to register the asset if it is not a 
				 * duplicate; or else we will have the same asset 
				 * registered twice, leading to issues when unloading
				 * the asset etc.
				 */
				if (isdupe == false && (ecode = marble_asset_internal_register(ps_asset)) != MARBLE_EC_OK)
					goto lbl_CLEANUP;
			}
		}

	lbl_CLEANUP:
		if (ecode) {
			printf("AssetManager: Dependency command %i (index: %u) failed; error: %i (%s).\n",
				cmd,
				i,
				ecode,
				marble_error_getstr(ecode)
			);

			return ecode;
		}
	}

	return ecode;
}

/*
 * Creates an empty asset of a specific type and ID.
 * 
 * Returns 0 on success, non-zero on failure. If the asset could
 * be created successfully, the pointer pointed to by
 * **pps_asset** will now hold the pointer to the newly-created
 * asset.
 * If **ptrpExistingAssetPtr** is not NULL and the asset is already loaded,
 * **ptrpExistingAssetPtr** will hold a pointer to the existing asset.
 */
static marble_ecode_t marble_asset_internal_create(
	enum marble_asset_type type,               /* type of the asset */
	char const *pz_id,                         /* string ID */
	struct marble_asset **pps_asset,           /* pointer to receive pointer to the newly-created asset */
	struct marble_asset **pps_existingassetptr /* optional pointer to receive pointer to an existing asset of the same ID */
) { MB_ERRNO
	if (pps_asset == NULL || marble_asset_internal_isvalidtype(type) == false)  return MARBLE_EC_PARAM;
	if (marble_asset_internal_isusableid(pz_id, pps_existingassetptr) == false) return MARBLE_EC_ASSETID;

	*pps_asset = NULL;
	switch (type) {
		default:
			ecode = MARBLE_EC_UNIMPLFEATURE;
	}

	if (ecode == MARBLE_EC_OK)
		marble_asset_internal_setid(*pps_asset, pz_id);
	else
		marble_asset_destroy(pps_asset);

	return ecode;
}

/*
 * Loads an asset from file and attempts to register it.
 * 
 * Returns 0 on success, non-zero on failure. 
 * If the asset could
 * be created successfully, the pointer pointed to by
 * **pps_asset** will now hold the pointer to the newly-created
 * asset.
 * If **ptrpExistingAssetPtr** is not NULL and the asset is already loaded,
 * **ptrpExistingAssetPtr** will hold a pointer to the existing asset.
 */
static marble_ecode_t marble_asset_internal_loadfromfile(
	char const *pz_path,                       /* path to asset file */
	struct marble_asset **pps_asset,           /* pointer to newly-created asset */
	struct marble_asset **pps_existingassetptr /* pointer to existing asset */
) { MB_ERRNO
	if (pps_asset == NULL)
		return MARBLE_EC_PARAM;
	bool wascreated = false;

	/* Open asset file. */
	struct marble_util_file *ps_file = NULL;
	ecode = marble_util_file_open(
		pz_path,
		MARBLE_UTIL_FILEPERM_READ,
		&ps_file
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_CLEANUP;
	
	/* Read common asset head for basic information. */
	struct marble_asset_commonhead s_commonhead;
	ecode = marble_util_file_read(
		ps_file, 
		sizeof s_commonhead, 
		&s_commonhead
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_CLEANUP;

	/* Create asset if it has not been created yet. */
	if (*pps_asset == NULL) {
		wascreated = TRUE;

		ecode = marble_asset_internal_create(
			s_commonhead.m_assettype, 
			s_commonhead.mz_strid,
			pps_asset,
			pps_existingassetptr
		);
		if (ecode != MARBLE_EC_OK)
			goto lbl_CLEANUP;
	}

	/* Evaluate dependency table; executing all of its commands. */
	ecode = marble_asset_internal_evaldeptable(
		*pps_asset, 
		ps_file, 
		s_commonhead.m_numofdeps
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_CLEANUP;

	marble_util_file_goto(ps_file, s_commonhead.m_dataoff);

	/* Call asset-type-specific loading routines. */
	switch (s_commonhead.m_assettype) {
		default:
			ecode = MARBLE_EC_UNIMPLFEATURE;
	}

lbl_CLEANUP:
	if (ecode && wascreated) {
		marble_asset_destroy(pps_asset);

		printf("AssetManager: Failed to create asset \"%s\"; error: %i (%s).\n",
			s_commonhead.mz_strid,
			ecode,
			marble_error_getstr(ecode)
		);
	}

	/* Close file stream. */
	marble_util_file_destroy(&ps_file);
	return ecode;
}


marble_ecode_t marble_application_loadasset(
	char const *pz_path
) { MB_ERRNO
	struct marble_asset *ps_asset = NULL;
	ecode = marble_asset_internal_loadfromfile(
		pz_path,
		&ps_asset,
		NULL
	);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	ecode = marble_asset_internal_register(ps_asset);
	if (ecode != MARBLE_EC_OK)
		marble_asset_destroy(&ps_asset);

	return ecode;
}

/*
 * Destroys (i.e. unregisters and deallocates all resources) the given asset.
 * 
 * Returns nothing.
 */
void marble_asset_destroy(
	struct marble_asset **pps_asset /* asset to destroy */
) {
	if (pps_asset == NULL || *pps_asset == NULL)
		return;

	switch ((*pps_asset)->m_type) {
		// TODO: add specific asset type destructors
	}

	if ((*pps_asset)->mps_deps) {
		for (size_t i = 0; i < (*pps_asset)->mps_deps->m_size; i++)
			marble_asset_internal_release((*pps_asset)->mps_deps->mpp_data[i]);

		marble_util_vec_destroy(&(*pps_asset)->mps_deps);
	}

	printf("AssetManager: Destroyed asset \"%s\" (type = %i), at address 0x%p (remaining references: %i).\n",
		(*pps_asset)->mz_strid,
		(*pps_asset)->m_type,
		*pps_asset,
		(*pps_asset)->m_refcount
	);

	free(*pps_asset);
	*pps_asset = NULL;
}


