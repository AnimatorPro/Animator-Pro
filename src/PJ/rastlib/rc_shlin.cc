#include "rastcall.ih"

void _set_hline( Raster *rp, Pixel color,
			  Coor minx, Coor y,  /* origin of line */
			  Ucoor width )          /* length of line */
/* draw in rp a horizontal line of color */
{
	SET_HLINE(rp,color,minx,y,width);
}
