#include "stdtypes.h"

void intel_swap(void *pt, int count)
{
register UBYTE *rpt = pt;
register UBYTE swap;

while (--count >= 0)
	{
	swap = rpt[1];
	rpt[1] = rpt[0];
	rpt[0] = swap;
	rpt += 2;
	}
}
