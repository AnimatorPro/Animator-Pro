#include "gfx.h"

void max_line(Raster *r, Short_xy *ends, dotout_func dotout, void *dotdat)
/* given two end points draws a line that spans whole screen going through the
 * end points assuming points are within screen */
{
#define clip_line(r,x1,y1,x2,y2,do,dd) pj_cline(x1,y1,x2,y2,do,dd)
SHORT dx,dy,absdx,absdy;
SHORT mult, x1,y1,x2,y2;

	x1 = ends->x;
	y1 = ends->y;
	x2 = (++ends)->x;
	y2 = ends->y;

	dx = x2 - x1;
	dy = y2 - y1;

	if(dx == 0)
	{
		pj_cline(x1,0,x1,r->height-1,dotout,dotdat);
		return;
	}
	else if(dy == 0)
	{
		pj_cline(0,y1,r->width-1,y2,dotout,dotdat);
		return;
	}

	absdx = Absval(dx);
	absdy = Absval(dy);

	if(absdx > absdy)
		mult = (r->width/absdx)+1; 
	else
		mult = (r->height/absdy)+1; 

	dx *= mult;
	dy *= mult;

	clip_line(r,x1-dx,y1-dy,x2+dx,y2+dy,dotout,dotdat);

#undef clip_line
}
