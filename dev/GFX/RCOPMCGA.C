#include "rcel.h"
#include "memory.h"
#include "errcodes.h"

Errcode pj_open_mcga_rcel(Rcel *cel)

/* opens an Rcel for a mcga raster only 320 X 200 cels */
{
Errcode err;

	if((err = pj_open_mcga_raster((Raster *)cel)) < Success)
		goto error;
	if((err = pj_cmap_alloc(&cel->cmap,256)) < Success)
		goto error;
	goto done;
error:
	pj_close_raster(cel);
done:
	return(err);
}
