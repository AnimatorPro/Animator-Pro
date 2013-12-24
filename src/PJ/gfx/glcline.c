
/* pj_cline.c - just your basic stepping line algorithm */

#define GFX_INTERNALS
#include "gfx.h"

void pj_cline(SHORT x1, SHORT y1, SHORT x2, SHORT y2, 
		   dotout_type dotout,
		   void *dotdat)

/* note that if x1 < x2 dots will be output from x1 to x2 and y1 to y2
 * and the reverse if x2 < x1 Some code is dependent on this */
{
register SHORT   duty_cycle;
SHORT incy;
register SHORT delta_x, delta_y;
register SHORT dots;

	delta_y = y2-y1;
	delta_x = x2-x1;
	if (delta_y < 0) 
	{
		delta_y = -delta_y;
		incy = -1;
	}
	else
	{
		incy = 1;
	}
	if ((delta_x) < 0) 
	{
		delta_x = -delta_x;
		incy = -incy;
		x1 = x2;
		y1 = y2;
	}
	duty_cycle = (delta_x - delta_y)/2;

	if (delta_x >= delta_y)
	{
		dots = ++delta_x;
		while (--dots >= 0)
		{
			(*dotout)(x1,y1,dotdat);
			duty_cycle -= delta_y;
			++x1;
			if (duty_cycle < 0)
			{
				duty_cycle += delta_x;	  /* update duty cycle */
				y1+=incy;
			}
		}
	}
	else /* dy > dx */
	{
		dots = ++delta_y;
		while (--dots >= 0)
		{
			(*dotout)(x1,y1,dotdat);
			duty_cycle += delta_x;
			y1+=incy;
			if (duty_cycle > 0)
			{
				duty_cycle -= delta_y;	  /* update duty cycle */
				++x1;
			}
		}
	}
}
