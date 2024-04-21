#include "errcodes.h"
#include "rastcall.h"
#include "memory.h"

/**** some routines to get temporary buffers of whatever size is available
 **** for processing rasters *****/

static Ucoor lines_in32k(Coor bpr, Ucoor height)
{
	Ucoor lines;

	/* try to allocate about 32K worth of buffer */
	lines = ((LONG)(32*1024))/bpr; 
	if (lines <= 0)
		return(1);
	if(lines > height)
		return(height);
	return(lines);
}
Errcode pj_get_rectbuf(Coor bpr, long *pheight, UBYTE **lbuf)

/* gets buffers for use with get/put rectpix, pheight initialized to 
 * max height needed */
{
	Ucoor lines;

	lines = lines_in32k(bpr,*pheight);
	for(;;)
	{
		if(!lines)
			return(Err_no_memory);
		if((*lbuf = pj_malloc((LONG)(lines * bpr))) != NULL)
		{
			*pheight = lines;
			return(0);
		}
		lines = (lines+1)>>1;	/* try again half size */
	}
}
Errcode pj_open_temprast(Bytemap *rr,SHORT width,SHORT height,
					  SHORT pdepth)

/*** gets raster with as many lines as can get under 32k ***/
{
Errcode err;
Rasthdr spec;
long bpr;
bool is_bitmap;

	if(pdepth == 1)
	{
		bpr = Bitmap_bpr(width);
		is_bitmap = true;
	}
	else
	{
		pdepth = 8;
		bpr = Bytemap_bpr(width);
		is_bitmap = false;
	}

	spec.pdepth = pdepth;
	spec.width = width;
	spec.height = lines_in32k(bpr,height);
	spec.aspect_dx = spec.aspect_dy = 1;

	/* if height <= 0 will error */
	for(;;)
	{
		if (is_bitmap) {
			err = pj_open_bitmap(&spec, (Bitmap *)rr);
		}
		else {
			err = pj_open_bytemap(&spec, rr);
		}

		if (err >= Success)
			break;

		spec.height = (spec.height+1) >> 1; /* try half as much */
	}
	return(err);
}

