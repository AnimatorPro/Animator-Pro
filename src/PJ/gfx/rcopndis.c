#include "cmap.h"
#include "errcodes.h"
#include "memory.h"
#include "rcel.h"
#include "vdevcall.h"

Errcode open_display_rcel(Vdevice *vd, Rcel *cel,
						  USHORT width, USHORT height, SHORT mode)
{
Errcode err;

	if((err = pj_vd_open_screen(vd,(Raster *)cel,width,height,mode)) < 0)
		goto error;
	if((err = pj_cmap_alloc(&cel->cmap,COLORS)) < Success)
		goto error;
	return(Success);
error:
	pj_close_raster(cel);
	return(err);
}
