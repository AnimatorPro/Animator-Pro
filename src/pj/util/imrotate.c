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
