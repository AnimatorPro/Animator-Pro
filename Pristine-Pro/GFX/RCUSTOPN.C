#include "rcel.h"
#include "memory.h"
#include "errcodes.h"
#include "vdevcall.h"

Errcode pj_rcel_custom_open(Rasthdr *spec, Rcel *cel,
							Grctype grc_lib_type, LONG num_colors)

/* opens an Rcel for given specs given pointer to a Vdevice a NULL vdriver
 * will open a Bytemap cel */
{
Errcode err;

	if((err = pj_rast_custom_open(spec, (Raster *)cel, grc_lib_type)) < Success)
		goto error;
	if((err = pj_cmap_alloc(&cel->cmap,num_colors)) < Success)
		goto error;
	return(Success);
error:
	pj_close_raster(cel);
	return(err);
}
