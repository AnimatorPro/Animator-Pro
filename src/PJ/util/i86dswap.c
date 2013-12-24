#include "stdtypes.h"

void intel_dswap(void *v, int count)
/* swap a 32 bit number from a Motorola style proc */
{
UBYTE *bb = v;
UBYTE swap;


while (--count >= 0)
	{
	swap = bb[0];
	bb[0] = bb[3];
	bb[3] = swap;
	swap = bb[1];
	bb[1] = bb[2];
	bb[2] = swap;
	bb += 4;
	}
}

