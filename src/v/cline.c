
/* cline.c - just your basic stepping line algorithm */

#include "jimk.h"

cline(x1, y1, x2, y2, dotout)
int x1,y1,x2,y2;
Vector dotout;
{
register WORD   duty_cycle;
WORD incy;
register WORD   delta_x, delta_y;
register WORD dots;

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
	dots = delta_x+1;
	while (--dots >= 0)
		{
		(*dotout)(x1,y1);
		duty_cycle -= delta_y;
		x1 += 1;
		if (duty_cycle < 0)
			{
			duty_cycle += delta_x;	  /* update duty cycle */
			y1+=incy;
			}
		}
	}
else
	{
	dots = delta_y+1;
	while (--dots >= 0)
		{
		(*dotout)(x1,y1);
		duty_cycle += delta_x;
		y1+=incy;
		if (duty_cycle > 0)
			{
			duty_cycle -= delta_y;	  /* update duty cycle */
			x1 += 1;
			}
		}
	}
}

