#define REXLIB_INTERNALS

#include "cmap.h"
#include "gfx.h"
#include "gfxlib.h"

Gfxlib aa_gfxlib = {
	/* header */
	{
		sizeof(Gfxlib),
		AA_GFXLIB, AA_GFXLIB_VERSION,
	},
	/* Stuff that goes to raster driver... */
	pj_alloc_bytemap,
	pj_alloc_bitmap,
	pj_open_temprast,
	pj_clipbox_make,
	pj_close_raster,
	pj_rast_free,
	pj_put_hseg,
	pj_get_hseg,
	pj_put_vseg,
	pj_get_vseg,
	_pj_put_rectpix,
	pj_get_rectpix,
	pj_set_hline,
	pj_set_vline,
	pj_clear_rast,
	pj_set_rect,
	pj_xor_rect,
	pj_mask1blit,
	pj_mask2blit,
	pj_xor_rast,
	pj__blitrect,
	pj_blitrect,
	pj__tblitrect,
	pj_tblitrect,
	pj__swaprect,
	pj_swaprect,
	pj_zoomblit,

	/* colormap stuff */

	pj_cmap_alloc,
	pj_cmap_free,
	pj_cmap_load,
	pj_cmap_copy,

	/* rcel routines */

	pj_rcel_bytemap_open,
	pj_rcel_bytemap_alloc,
	pj_rcel_make_virtual,

	pj_rcel_close,
	pj_rcel_free,
};

