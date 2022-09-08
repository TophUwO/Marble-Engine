#pragma once

#include <asset.h>


/*
 * layer types
 * 
 * Denotes in what way the information given
 * in the layer is to be interpreted.
 */
enum marble_levelasset_layertype {
	/*
	 * unknown/unspecified layer type, used
	 * as error value
	 */
	MARBLE_LVLLAYERTYPE_UNKNOWN = 0,

	MARBLE_LVLLAYERTYPE_TEXTURE, /* layer containing tile indices */

#if (defined MB_DYNAMIC_LIBRARY)
	__MARBLE_NUMLVLLAYERTYPES__  /* for internal use */
#endif
};


/*
 * layer tile object
 * 
 * An object representing a tile on a map in a specific
 * layer. Contains internal objects for each layer
 * type.
 * 
 * The "active" portion of this composite object
 * is determined by the type of the layer this object
 * belongs to.
 */
union marble_levelasset_layerobj {
	/*
	 * Object representing an index to a
	 * specific tile in a specific dependency,
	 * in this case a tileset.
	 * 
	 *     0   1   2   3   4   5   6   7   8   9   x + +
	 *   +---+---+---+---+---+---+---+---+---+---+
	 * 0 |   |   |   |   |   |   |   |   |   |   |
	 *   |   |   |   |   |   |   |   |   |   |   |
	 *   +---+---+---+---+---+---+---+---+---+---+
	 * 1 |   |   |   |   |   |   |   |   |   |   |
	 *   |   |   |   |   |   |   |   |   |   |   |
	 *   +---+---+---+---+---+---+---+---+---+---+
	 * 2 |   |   |   |   |   |   |   |   |   |   |
	 *   |   |   |   |   |   |   |   |   |   |   |
	 *   +---+---+---+---+---+---+---+---+---+---+
	 * 3 |   |   |   |   |   |   |   |   |   |   |
	 *   |   |   |   |   |   |   |   |   |   |   |
	 *   +---+---+---+---+---+---+---+---+---+---+
	 * y
	 * +
	 * +
	 * 
	 * Linear index format:
	 *     i = y * width + x
	 * 
	 *     x := { 0, width  - 1 }
	 *     y := { 0, height - 1 }
	 */
	struct marble_levelasset_texobj {
		int m_depindex; /* index into the dep-table */
		/*
		 * index into the dependency
		 * 
		 * The index is represented by a format
		 * similar to the one described by **i**.
		 */
		int m_x;
		int m_y;
	} _tex;
};

/*
 * layer structure
 */
struct marble_levelasset_layer {
	enum marble_levelasset_layertype m_type; /* layer type */

	/*
	 * Bare array representing the layer data in a contiguous way.
	 * 
	 * It is not a vector because resizing and reordering
	 * the data is easier without an abstraction. Also,
	 * the array does not dynamically grow.
	 */
	union marble_levelasset_layerobj *mpu_layerdata;
};

/*
 * Asset representing a level. 
 */
struct marble_levelasset {
	struct marble_asset _base; /* common asset type */

	size_t m_width;  /* width of each layer, in tiles */
	size_t m_height; /* height of each layer, in tiles */
	int m_nlayers;   /* number of layers */

	/*
	 * List of layers. The topmost layer is the
	 * first element in this list. 
	 */
	struct marble_util_vec *mps_layers;
};


#if (defined MB_DYNAMIC_LIBRARY) || (defined MB_ECOSYSTEM)
_Critical_ MB_API marble_ecode_t marble_levelasset_new(
	_In_                 int width,
	_In_                 int height,
	_Init_(pps_lvlasset) struct marble_levelasset **pps_lvlasset
);

MB_API void marble_levelasset_destroy(
	struct marble_levelasset **pps_lvlasset
);

MB_API marble_ecode_t marble_levelasset_addlayer(
	_In_ struct marble_levelasset *ps_lvlasset,
	     enum marble_levelasset_layertype type
);
#endif


