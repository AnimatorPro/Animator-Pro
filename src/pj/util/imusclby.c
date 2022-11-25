#include "imath.h"


int pj_uscale_by(USHORT x, USHORT p, USHORT q)

/* return(x * p/q) done to avoid rounding error */
{
LONG l;

	l = x;
	l *= p;
	l /= q;
	return((int)l);
}
