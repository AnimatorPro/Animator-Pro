#include "imath.h"

int itmult(SHORT trig,SHORT x)
{
long result;

	result = trig;
	result *= x;
#ifdef LATER
	if (result >= 0)
		result += 1<<13;
	else
		result -= 1<<13;
#endif /* LATER */
	return(result/(1<<14));
}

void polar(short theta,short rad,short *xy)
{
int c;
	xy[1] = itmult(isincos(theta,&c),rad);
	xy[0] = itmult(c,rad);
}

void rotate_points(int theta,int cx,int cy,short *spt,short *dpt,int count)
{
int s,c,x,y;

	s = isincos(theta,&c);
	while (--count >= 0)
	{
		x = *spt++ - cx;
		y = *spt++ - cy;
		*dpt++ = itmult(c,x) - itmult(s,y) + cx;
		*dpt++ = itmult(s,x) + itmult(c,y) + cy;
	}
}

