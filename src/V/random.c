
#include "jimk.h"
/* random.c - quick and dirty 12 bit random number generator */

int
v_random(void)
{
if ((vs.randseed+=vs.randseed) >= 0)
	vs.randseed ^= 0x2b41;
return(vs.randseed);
}

