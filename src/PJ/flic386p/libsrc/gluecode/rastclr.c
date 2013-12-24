/*****************************************************************************
 * RASTCLR.C - Clear a raster (set data area to all zeroes).
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_raster_clear(FlicRaster *prast)
/*****************************************************************************
 * clear the data area within a raster.
 ****************************************************************************/
{
	if (NULL == prast)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);
	pj_set_rast(prast,0);
	return Success;
}
