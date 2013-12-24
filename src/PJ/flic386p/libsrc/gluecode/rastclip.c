/*****************************************************************************
 * RASTCLIP.C - Create a clipped virtual raster, either centered or offset.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

static FlicRaster	cached_raster;
static UBYTE		cached_raster_in_use = FALSE;

Errcode pj_raster_make_offset(FlicRaster **ppvirt,
							  FlicRaster *proot,
							  Flic		 *pflic,
							  int x, int y)
/*************************************************************************
 * This makes a raster that is a rectangular piece of a potentially larger
 * raster.	It also forces all drawing and fli-playing routines to clip
 * to the bounds of the root raster.
 *************************************************************************/
{
	FlicRaster	*pvirt;
	Boolean 	clipped_out;

	if (NULL == ppvirt || NULL == proot || NULL == pflic) {
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);
	}

	if (cached_raster_in_use) {
		if (NULL == (pvirt = pj_zalloc(sizeof(FlicRaster))))
			return Err_no_memory;
	} else {
		cached_raster_in_use = TRUE;
		pvirt = &cached_raster;
	}

	*pvirt = *proot;
	*ppvirt = pvirt;

	clipped_out = !pj_clipbox_make((Clipbox *)pvirt, (Raster *)proot,
					x, y, pflic->flifile->hdr.width, pflic->flifile->hdr.height);

	if (clipped_out)
		return 1;
	else
		return Success;
}

Errcode pj_raster_free_offset(FlicRaster **pprast)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (NULL == pprast || NULL == *pprast)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (*pprast != &cached_raster) {
		pj_free(*pprast);
	} else {
		cached_raster_in_use = FALSE;
	}

	*pprast = NULL;
	return Success;
}

Errcode pj_raster_make_centered(FlicRaster **ppvirt,
							  FlicRaster *proot,
							  Flic		 *pflic)
/*************************************************************************
 *
 *************************************************************************/
{
	if (NULL == proot || NULL == pflic) {
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);
	}

	return pj_raster_make_offset(ppvirt, proot, pflic,
				((proot->width	- pflic->flifile->hdr.width)  / 2),
				((proot->height - pflic->flifile->hdr.height) / 2));
}

Errcode pj_raster_free_centered(FlicRaster **pprast)
/*****************************************************************************
 *
 ****************************************************************************/
{
	return pj_raster_free_offset(pprast);
}

