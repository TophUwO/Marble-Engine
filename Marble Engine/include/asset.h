#pragma once

#include <api.h>


/* asset type IDs */
enum marble_asset_type {
	MARBLE_ASSETTYPE_UNKNOWN    = 0, /* reserved */

	MARBLE_ASSETTYPE_LEVEL,          /* level/map asset */

	__MARBLE_NUMASSETTYPES__         /* for internal use */
};

/*
 * Asset dependency table commands
 * 
 * Because these values will be read from and 
 * written to a file, for compatibility reasons, these
 * values are assigned explicitly. 
 */
enum marble_asset_deptablecmd {
	MARBLE_ASSETDEPTABLECMD_UNKNOWN   = 0, /* reserved */

	MARBLE_ASSETDEPTABLECMD_LOADASSET = 1  /* command to load an asset */
};


// TODO: proper struct packing
/*
 * Common asset head.
 * 
 * Every asset file starts with a representation of this
 * structure. The byte-order of the asset head is always
 * little-endian.
 */
struct marble_asset_commonhead {
	/*
	 * Because the magic number consists of
	 *  (1) the actual magic number 'mb' and
	 *  (2) the asset type ID,
	 * it's better to be able to read them individually
	 * without fancy bit-shifting. That just, you know,
	 * muddies the waters and is really easy to forget
	 * when maintaining the code and adding information
	 * to the magic number.
	 */
	union {
		uint32_t m_magic32;       /* full magic number */

		struct {
			uint16_t m_magic16;   /* magic number */
			uint16_t m_assettype; /* asset type ID */
		};
	};
	/*
	 * m_flags reference
	 * 
	 * Used for additional flags.
	 * 
	 * +--------+--------+--------+--------+
	 * |        |        |        |        |
	 * +--------+--------+--------+--------+
	 * 0        7        15       23       31
	 * 
	 * [0] ... asset is persistent (will not be unloaded when refcount becomes 0)
	 */
	uint32_t m_flags;
	char     maz_strid[MB_STRINGIDMAX]; /* asset string ID */
	uint32_t m_numofdeps;               /* number of dependency table entries */
	uint32_t m_depheadoff;              /* offset to asset-specific head from begin of file */
	uint32_t m_dataoff;                 /* offset to asset data from begin of file */
};

/*
 * Generic asset type.
 * 
 * Every specialized asset type inherits all
 * the fields of this type.
 */
struct marble_asset {
	int   m_type;                     /* asset type ID */
	char  maz_strid[MB_STRINGIDMAX];  /* asset string ID */
	int   m_refcount;                 /* internal ref-count */

	struct marble_util_vec *mps_deps; /* dependencies loaded by the asset */ 
};


