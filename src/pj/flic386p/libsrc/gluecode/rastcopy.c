/*****************************************************************************
 * RASTCOPY.C - Copy data & color between rasters.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

extern void pj_rcel_copy(Rcel *source, Rcel *dest);
extern void pj_cmap_load(Rcel *source, Cmap *dest);

Errcode pj_raster_copy(FlicRaster *source, FlicRaster *dest)
/*************************************************************************
 * copy data and color map from one raster to another.
 * the rasters must be the same size.
 *************************************************************************/
{
	Boolean cmaps_same;

	if (NULL == source || NULL == dest)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);
	if (source->width != dest->width || source->height != dest->height)
		return Err_wrong_res;
	cmaps_same = pj_cmaps_same(source->cmap, dest->cmap);
	pj_rcel_copy((Rcel *)source, (Rcel *)dest);
	if (!cmaps_same)
		/* update hardware color map if needed */
		pj_cmap_load((Rcel *)dest, (Cmap *)dest->cmap); 
	return Success;
}
