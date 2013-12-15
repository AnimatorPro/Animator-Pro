/*****************************************************************************
 * RASTBIND.C - Allocate/free raster & color map bound to client's data area.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_raster_bind_ram(FlicRaster **pprast, int width, int height, Pixel *pbuf)
/*****************************************************************************
 * bind a client-provided data area into a bytemap (ram) raster.
 ****************************************************************************/
{
	FlicRaster	*prast;
	Rasthdr 	protocel;
	Errcode 	err;

	if (NULL == pprast || NULL == pbuf)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (0 >= width || 0 >= height)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	if (NULL == (prast = pj_zalloc(sizeof(FlicRaster))))
		return Err_no_memory;

	protocel.width		= width;
	protocel.height 	= height;
	protocel.pdepth 	= 8;
	protocel.aspect_dx	= 1;
	protocel.aspect_dy	= 1;

	if (Success <= (err =  pj_build_bytemap(&protocel, (Raster *)prast, pbuf))) {
		if (Success <= (err = pj_cmap_alloc((Cmap **)(&prast->cmap), COLORS))) {
			pj_get_default_cmap((Cmap *)(prast->cmap));
		}
	}

	if (Success <= err)
		*pprast = prast;
	else
		pj_free(prast);

	return err;

}

Errcode pj_raster_unbind_ram(FlicRaster **pprast)
/*****************************************************************************
 * free resources for a raster allocated with above routine.
 * does NOT free client's data area, just the bits we allocated above.
 ****************************************************************************/
{
	FlicRaster *prast;

	if (NULL == pprast)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);
	prast = *pprast;
	pj_freez(&prast->cmap);
	pj_freez(pprast);
	return Success;
}
