#include "rastcall.ih"

void _set_vline( Raster *r, Pixel color,
			  Coor x, Coor miny,  /* origin of line */
			  Ucoor height)         /* length of line */
/* draw in rp a vertical line of color */
{
	SET_VLINE(r,color,x,miny,height);
}
