#include "imath.h"

int rscale_by(int x,int p,int q)
{
long l;
SHORT sign;

	sign = 1;
	if (x < 0)
		sign = -sign;
	if (p < 0)
		sign = -sign;
	l = x;
	l *= p;
	if (sign > 0)
		l += (q/2);
	else
		l -= (q/2);
	l /= q;
	return((int)l);
}
