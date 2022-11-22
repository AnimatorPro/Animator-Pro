/*****************************************************************************
 * RASTONLY.C - Create a special read-only raster good only for flic creation.
 *
 *	This module allows creation of an extremely limited type of raster that
 *	can be used only as the input to the pj_flic_write_first/next/finish()
 *	routines.  Flic creation routines are guaranteed to call only a get_hseg()
 *	function to obtain their input, so the raster created herein implements
 *	only that function (as provided by the client code), and stubs out all
 *	the rest of the raster I/O library.
 ****************************************************************************/

#include "flicglue.h"
#include "rastlib.h"

static char *modulename = __FILE__;

static Errcode dummy_routine(void)
/*****************************************************************************
 * stub routine that does nothing, plugged into custom raster I/O library.
 ****************************************************************************/
{
	return Err_unimpl;
}

Errcode pj_raster_make_compress_only(FlicRaster **pprast,
					int width, int height, void *get_hseg_func)
/*************************************************************************
 * Attach client-provided function and stub library to a custom raster.
 *************************************************************************/
{
	Errcode 	err;
	FlicRaster	*prast;
	Rastlib 	*rlib;

	if (NULL == pprast || NULL == get_hseg_func)
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

	pj_stuff_pointers(dummy_routine, rlib, sizeof(Rastlib)/sizeof(void*));
	rlib->get_hseg = get_hseg_func;

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

Errcode pj_raster_free_compress_only(FlicRaster **pprast)
/*****************************************************************************
 * free raster and resources allocated above.
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
