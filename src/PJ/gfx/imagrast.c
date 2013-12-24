#define RASTGFX_INTERNALS
#include "gfx.h"
#include "errcodes.h"

Errcode image_to_rast(Image *i, Raster *rast, Raster *tomatch)
/* loads an empty raster with data from image to make a valid raster 
 * if tomatch is non null it will load other spec fields from tomatch */
{
	if(tomatch)
		*(Rasthdr *)rast = *(Rasthdr *)tomatch; 

	rast->width = i->width;
	rast->height = i->height;
	rast->pdepth = i->depth;

	if(i->type == ITYPE_BITPLANES)
		return(pj_build_bitmap((Rasthdr *)rast,rast,i->image));
	return(Err_bad_input);
}
