#include "gfx.h"
#include "memory.h"

int pixsame(void *pixbuf, USHORT numpix, Pixel first_color)

/* gives number of contiguous pixels in pixbuf equal to first_color.
 * note: does not check for numpix <= 0 
 * only works for pixsize = 1 for now */
{
	if(*(Pixel *)pixbuf != first_color)
		return(0);
	return(pj_bsame(pixbuf,numpix));
}

