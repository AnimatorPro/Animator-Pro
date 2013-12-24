/*****************************************************************************
 * RASTALLO.C - Allocate/free a ram-based raster, data area, and color map.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_raster_make_ram(FlicRaster **pprast, int width, int height)
/*****************************************************************************
 * allocate a ram-based raster.  creates raster, data area, and color map.
 *
 * this routine does sanity checking on the caller's parms, then sets up the
 * 'spec' raster based on caller's width/height and calls the internal routine
 * to allocate the parts and bind them together into a raster.
 ****************************************************************************/
{
	Rasthdr protocel;
	Errcode err;

	if (NULL == pprast)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (0 >= width || 0 >= height)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	protocel.width		= width;
	protocel.height 	= height;
	protocel.pdepth 	= 8;
	protocel.aspect_dx	= 1;
	protocel.aspect_dy	= 1;

	if (Success <= (err =  pj_rcel_bytemap_alloc(&protocel, (Rcel **)pprast, COLORS))) {
		pj_get_default_cmap((Cmap *)((*pprast)->cmap));
	}
	return(err);

}

Errcode pj_raster_free_ram(FlicRaster **pprast)
/*****************************************************************************
 * free a raster allocated via above routine.
 ****************************************************************************/
{
	if (NULL == pprast || NULL == *pprast)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);
	pj_rcel_free((Rcel *)*pprast);
	*pprast = NULL;
	return Success;
}
