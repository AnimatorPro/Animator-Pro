/* random.c - random number generator */
#include "jimk.h"
#include "util.h"

void pj_srandom(int seed)
{
	vs.randseed = seed;
}
int pj_random(void)
{
 	return((((ULONG)(vs.randseed=
				((vs.randseed * 0x41c64e6d)+0x3039)))>>10)&0x7fff);
}
