#include "rastcall.ih"

void pj_set_rect(Raster *r, Pixel color,
			  Coor x, Coor y, Ucoor w, Ucoor h )
/************************************************************************* 
 * Draw a solid color rectangle.  Rectangle is clipped to fit raster.
 *
 * Parameters:
 *		Raster 	*r;		What get's drawn on.
 *		Pixel 	color;	Color to draw with
 *		Coor 	x;		Left edge of rectangle
 *		Coor	y;		Top edge of rectangle
 *		Ucoor	w;		Width
 *		Ucoor	y;		Height
 *************************************************************************/
{
	cliprect(r,x,y,w,h);
	SET_RECT(r,color,x,y,w,h);
}
