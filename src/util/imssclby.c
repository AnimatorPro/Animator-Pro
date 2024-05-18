#include "imath.h"

int sscale_by(int x, int p, int q)
{
LONG l;

	l = x;
	l *= p;
	l /= q;
	return((int)l);
}
