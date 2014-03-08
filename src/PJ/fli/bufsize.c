#include "cmap.h"
#include "errcodes.h"
#include "fli.h"

LONG pj_fli_cbuf_size(USHORT width,USHORT height, LONG num_colors)

/* returns maximum size needed for a compression buffer to compress contents
 * of a screen of a given size */
{
	return((width * height)      	   /* one full screen */
			+ Max(2*width,2*PSTAMP_W)  /* 2-line buffer */
			+ (num_colors * 3)   /* cmap size */
			+ 256 + 16           /* jims extra stuff */
			+ sizeof(Raster) 	 /* a raster ? */
			+ (PSTAMP_W*PSTAMP_H) ); /* a full pstamp record */ 
}
LONG pj_fli_cel_cbuf_size(Rcel *cel)
/* given pointer to an Rcel screen returns max compression buffer size 
 * needed */
{
	return(pj_fli_cbuf_size(cel->width,cel->height,cel->cmap->num_colors));
}
