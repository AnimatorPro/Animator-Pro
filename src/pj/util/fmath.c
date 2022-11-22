#include "fpmath.h"


int roundtoint(double in)
{
	return((int)(in+(in<0?-0.5:0.5)));
}
#ifndef roundtolong
long roundtolong(double in)
{
	return((long)(in+(in<0?-0.5:0.5)));
}
#endif
