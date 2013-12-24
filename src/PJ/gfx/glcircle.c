#include "gfx.ih"

void circle(void *r,Pixel color, Coor centx, 
			Coor centy, Ucoor diameter, Boolean filled)

/* for simple one color circles */
{
Sdat sd;

	sd.rast = r;
	sd.color = color;
	dcircle(centx, centy, diameter, sdot, &sd, (EFUNC)shline, &sd, filled);
}
