/* grcflird.c - Generic library calls for fli decompressing only and a call to
 * load them into a library */

#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "rastcall.h"
#include "rastlib.h"

static void _grc_get_hseg(const Raster *v, Pixel *pixbuf, 
	Ucoor x, Ucoor y, Ucoor width)
/* Move pixels from a horizontal line of source raster to memory buffer. */
/* (Unclipped) */
{
while (width-- > 0)
	*pixbuf++ = GET_DOT(v, x++, y);
}

static void _grc_get_rectpix(const Raster *v,Pixel *pixbuf,
	Ucoor x,Ucoor y,Ucoor width,Ucoor height)
/* Move a rectangular area of raster into a memory buffer. (Much like
   a blit, but assumes destination is memory and that the width and height
   of the move are the same size as the memory buffer. */
/* (Clipped.) */
{
while(height-- > 0)
	{
	GET_HSEG(v,pixbuf,x,y++,width);
	pixbuf = OPTR(pixbuf,width);
	}
}

void pj_grc_load_compcalls(struct rastlib *lib)
/* loads compressor relevant subset of generic calls into a library */
{
	pj_grc_load_commcalls(lib);
	lib->get_hseg = _grc_get_hseg;
	lib->get_rectpix = _grc_get_rectpix;
}
