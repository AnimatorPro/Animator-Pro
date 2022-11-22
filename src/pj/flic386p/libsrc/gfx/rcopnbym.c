#include "rcel.h"
#include "memory.h"
#include "errcodes.h"
//#include "vdevcall.h"

Errcode pj_rcel_bytemap_open(Rasthdr *spec,Rcel *cel,LONG num_colors)

/* opens an Rcel for given specs given pointer to a Vdevice a NULL vdriver
 * will open a Bytemap cel */
{
Errcode err;

	if((err = pj_open_bytemap(spec,(Bytemap *)cel)) < Success)
		goto error;
	if((err = pj_cmap_alloc(&cel->cmap,num_colors)) < Success)
		goto error;
	return(Success);
error:
	pj_close_raster(cel);
	return(err);
}
