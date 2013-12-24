/*****************************************************************************
 * CMAPUPDT.C - Update cmap attached to raster, and hardware palette if it
 *				is a hardware raster.
 ****************************************************************************/

#include "flicglue.h"
#include "gfx.h"

static char *modulename = __FILE__;

Errcode pj_cmap_update(FlicRaster *prast, PjCmap *cmap)
/*****************************************************************************
 * update the cmap in raster from the specified cmap.
 * - if NULL is passed as the cmap pointer, the raster's cmap is used.
 * - if the specified cmap is not the cmap attached to the raster, its
 *	 contents are copied into the raster's cmap.
 * - if the raster is a hardware raster, the hardware palette is updated.
 ****************************************************************************/
{

	if (NULL == prast)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (NULL == cmap)
		cmap = prast->cmap;

	if (prast->cmap != cmap)
		pj_cmap_copy((Cmap *)cmap, (Cmap *)prast->cmap);

	pj_cmap_load(prast, (Cmap *)prast->cmap);

	return Success;
}
