#include "cmap.h"
#include "errcodes.h"
#include "rcel.h"
#include "vdevcall.h"

Errcode open_vd_rcel(Vdevice *vd, Rasthdr *spec, Rcel *cel,
					 LONG num_colors, UBYTE displayable)

/* opens an Rcel for given specs given pointer to a Vdevice a NULL vdriver
 * will open a Bytemap cel */
{
Errcode err;

	if((err = pj_vd_open_raster(vd,spec,(Raster *)cel,displayable)) < Success)
		goto error;
	if((err = pj_cmap_alloc(&cel->cmap,num_colors)) < Success)
		goto error;
	return(Success);
error:
	pj_close_raster(cel);
	return(err);
}
