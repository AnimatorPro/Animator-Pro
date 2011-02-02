#include "rastcall.ih"

void pj_set_hline(Raster *v, Pixel color, Coor x, Coor y, Ucoor width)
/************************************************************************* 
 * Draw a horizontal line in a solid color.  This is clipped, so it's
 * ok for the line to partially or entirely outside the raster.
 *
 * Parameters:
 *		Raster	*v;		What we draw on.
 *		Pixel	color;	Color of horizontal line.
 *		Coor	x;		Left end of line.
 *		Coor	y;		Vertical position of line.
 *		Ucoor	width;	Width of line in pixels.
 *************************************************************************/
{
	if(((Ucoor)y) >= v->height)
		return;
	if(x >= (Coor)(v->width))
		return;
	if(x < 0)
	{
		width += x;
		x = 0;
	}
	if(((Coor)width) <= 0)
		return;
	width += x;
	if (width > v->width)
		width = v->width;
	SET_HLINE(v,color,x,y,width - x);
}
