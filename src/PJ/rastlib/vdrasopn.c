#define VDEV_INTERNALS
#include "errcodes.h"
#include "rastcall.h"
#include "vdevcall.h"

Errcode pj_vd_open_raster(Vdevice *vd, Rasthdr *spec, Raster *r,UBYTE displayable)

/* opens a secondary (currently) un-displayed screen (raster) 
 * on a video device to match the specs in the input "spec" raster 
 * header use close_raster() to close this screen. it takes 
 * width, height, pdepth, aspect_dx, and aspect_dy from the spec Raster 
 * struct provided.  All other fields in the raster struct are ignored.
 * no fields in the spec raster struct are altered.  If the open fails 
 * because the spec is invalid ie: aspect ratio un-displayable or pixel
 * depth is unavailable the correct values for the mode will be loaded
 * in the (now closed) raster the fields X and Y are always set to 0 
 * The spec may be the header of the raster to be opened */
{
Errcode err;
Rasthdr sspec;

	if (vd->lib->open_cel == NULL)
		return(Err_unimpl);

	if(spec == (Rasthdr *)r)
	{
		sspec = *spec;
		spec = &sspec;
	}

	if ((err = (*vd->lib->open_cel)(vd,r,spec->width,spec->height,
									spec->pdepth, displayable)) >= Success)
	{
		if(r->type < vd->first_rtype
		    || r->type >= vd->first_rtype + vd->num_rtypes)
		{
			err = Err_driver_protocol;
			goto error;
		}
		pj_set_grc_calls(r->lib);
	}

	/* if it is a displayable raster and the displayed aspect ratio is
	 * not available return error code otherwise they get whatever 
	 * they ask for */

	if(displayable)
	{
		if (r->aspect_dx != spec->aspect_dx || r->aspect_dy != spec->aspect_dy)
		{
			err = Err_aspect_not_disp;
			goto error;
		}
	}
	else
	{
		r->aspect_dx = spec->aspect_dx;
		r->aspect_dy = spec->aspect_dy;
	}

	goto done;
error:
	pj_close_raster(r);
done:
	return(err);
}
