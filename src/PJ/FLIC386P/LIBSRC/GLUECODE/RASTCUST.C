/*****************************************************************************
 * RASTCUST.C - Bind a set of client-specified routines into a raster.
 *
 *	This allows the client to build a mini-driver from a set of custom
 *	access routines, with fliclib generic raster I/O routines filling in
 *	the bulk of the driver.  This might be used to support some custom video
 *	hardware not supported by the fliclib without the trouble of implementing
 *	a complete device driver. It might also be used to implement EMS rasters,
 *	and other esoteric storage schemes.
 *
 *	The client routines absolutely must supply put_dot and get_dot functions.
 *	If video hardware is being supported, set_colors is also mandatory.
 *	Supplying the other routines, especially get/put_hseg and set_hline, will
 *	vastly speed access to the raster, as the generics will make repeated
 *	calls to get/put_dot if the seg/line functions are missing.
 ****************************************************************************/

#include "flicglue.h"
#include "rastlib.h"

static char *modulename = __FILE__;

Errcode pj_raster_make_custom(FlicRaster **pprast,
								int width, int height, RastlibCustom *lib)
/*************************************************************************
 * allocate a raster and color map, attach client-supplied I/O routines.
 ************************************************************************/
{
	Errcode 	err;
	Rastlib 	*rlib;
	FlicRaster	*prast;

	if (NULL == pprast || NULL == lib)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (NULL == (prast = pj_zalloc(sizeof(FlicRaster))))
		return Err_no_memory;

	if (NULL == (prast->lib = rlib = pj_zalloc(sizeof(Rastlib)))) {
		pj_free(prast);
		return Err_no_memory;
	}

	if (Success > (err = pj_cmap_alloc((Cmap **)(&prast->cmap), COLORS))) {
		pj_freez(&prast->lib);
		pj_free(prast);
		return err;
	}

	rlib->put_dot	 = (rl_type_put_dot)lib->put_dot;
	rlib->get_dot	 = (rl_type_get_dot)lib->get_dot;
	rlib->set_hline  = (rl_type_set_hline)lib->set_hline;
	rlib->put_hseg	 = (rl_type_put_hseg)lib->put_hseg;
	rlib->get_hseg	 = (rl_type_get_hseg)lib->get_hseg;
	rlib->set_colors = (rl_type_set_colors)lib->set_colors;
	rlib->wait_vsync = (rl_type_wait_vsync)lib->wait_vsync;
	pj_set_grc_calls(rlib);

	prast->type 	 = RT_FIRST_CUSTOM;
	prast->width	 = width;
	prast->height	 = height;
	prast->pdepth	 = 8;
	prast->aspect_dx = 1;
	prast->aspect_dy = 1;

	pj_get_default_cmap((Cmap *)(prast->cmap));

	*pprast = prast;
	return Success;
}

Errcode pj_raster_free_custom(FlicRaster **pprast)
/*****************************************************************************
 * free raster and resources allocate above.
 ****************************************************************************/
{
	FlicRaster *prast;

	if (NULL == pprast)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);
	prast = *pprast;
	pj_freez(&prast->lib);
	pj_freez(&prast->cmap);
	pj_freez(pprast);
	return Success;
}
