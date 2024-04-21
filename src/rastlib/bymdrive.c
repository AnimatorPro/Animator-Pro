#include "raster.h"

/* video driver interface to ramrast functions */

#ifdef DOESNT_WORK_YET
static Errcode vd_init_ramrast(Vdriver *vd, Raster *toclone, 
							   RamRast *rr, SHORT mode )
{
	init_ramrast(toclone,rr,mode);
	return(0);
}
static Errcode vd_open_ramrast(Vdriver *vd, RamRast *rr)
{
	return(open_ramrast(rr));
}
static int rr_open = 0;
static rr_close_vdriver(Vdriver *vd)
{
	rr_open = 0;
}
Errcode rr_load_vdriver(Vdriver *vd)
{
static int loaded = 0;
static void *rr_vdlib[VDL_NUMCALLS];

	if(rr_open)
		return(Err_isopen);

	if(!loaded)
	{
		/* Work around bug in 3D Studio REX loader where static
		 * data isn't set to zero. */
		clear_mem(rr_vdlib,sizeof(rr_vdlib)); 

		rr_vdlib[VDL_CLOSE] = rr_close_vdriver;
		rr_vdlib[VDL_GET_MODES] = pj_vdo_nutin;
		rr_vdlib[VDL_INIT_RAST] = vd_init_ramrast;
		rr_vdlib[VDL_OPEN_RAST] = vd_open_ramrast;
		rr_vdlib[VDL_SHOW_RAST] = pj_vdo_nutin;
		loaded = 1;
	}
	vd->lib = rr_vdlib;
	vd->first_rtype = RT_BITMAP; 
	vd->last_rtype = RT_BYTEMAP; 
	rr_open = 1;
	return(0);
}
#endif /* DOESNT_WORK_YET */
