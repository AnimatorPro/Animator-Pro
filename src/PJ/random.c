/* random.c - random number generator */
#include "jimk.h"


void srandom(int seed)
{
	vs.randseed = seed;
}
int random(void)
{
 	return((((ULONG)(vs.randseed=
				((vs.randseed * 0x41c64e6d)+0x3039)))>>10)&0x7fff);
}
