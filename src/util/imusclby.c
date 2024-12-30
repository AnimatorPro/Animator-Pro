#include "imath.h"


/* return(x * p/q) done to avoid rounding error */
int pj_uscale_by(USHORT x, USHORT p, USHORT q)
{
	int l = x;
	l *= p;
	l /= q;
	return l;
}
