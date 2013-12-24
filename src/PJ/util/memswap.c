#include "stdtypes.h"

void swap_mem(void *srca, void *srcb, int count)
{
register UBYTE *a = srca;
register UBYTE *b = srcb; 
char swap;

	while (--count >= 0)
	{
		swap = *a;
		*a++ = *b;
		*b++ = swap;
	}
}
