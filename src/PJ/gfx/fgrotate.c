#include "gfx.h"
#include "floatgfx.h"
#include "imath.h"

void frotate_points2d(double theta,
					  Short_xy *cent,
					  Short_xy *spt,
					  Short_xy *dpt,int count)
{
Short_xy c;
double x,y;
double sint, cost;
double fx,fy;

	if(cent)
		c = *cent;
	else
		*((LONG *)&c) = 0;

	sint = sin(theta);
	cost = cos(theta);

	while (--count >= 0)
	{

#ifdef NO_FLOAT_ROUNDING
		x = spt->x - c.x;
		y = spt->y - c.y;

		dpt->x = (SHORT)(cost*x - sint*y) + c.x;
		dpt->y = (SHORT)(sint*x + cost*y) + c.y;
#else
		x = spt->x - c.x;
		y = spt->y - c.y;

		fx = cost*x - sint*y;
		fy = sint*x + cost*y;

		dpt->x = (SHORT)(fx + (fx<0?-0.5:+0.5)) + c.x;
		dpt->y = (SHORT)(fy + (fy<0?-0.5:+0.5)) + c.y;
#endif
		++spt;
		++dpt;
	}
}
