#define VDEV_INTERNALS
#include "errcodes.h"
#include "rastcall.h"
#include "vdevcall.h"

Errcode pj_vd_open_screen(Vdevice *vd, Raster *r,
					   USHORT width, USHORT height, USHORT mode)

/* verifys driver and opens primary display screen on a video device 
 * use pj_close_raster() to close this screen */
{
Errcode err;

	if (vd->lib->open_graphics == NULL)
		goto bad_protocol;
	if((err = (*vd->lib->open_graphics)(vd, r, width, height, mode)) < Success)
		return(err);
	if(r->type < vd->first_rtype
	    || r->type >= vd->first_rtype + vd->num_rtypes)
	{
		pj_close_raster(r);
		goto bad_protocol;
	}
	pj_set_grc_calls(r->lib);
	return(Success);
bad_protocol:
	return(Err_driver_protocol);
}
