#include "memory.h"

int nonzero_bytes(const UBYTE *c, int array_size)
/* count number of non-zero elements in a byte array */
{
const UBYTE *maxc;
int acc;

	maxc = c + array_size;
	acc = 0;
	while(c < maxc)
	{
		if(*c++)
			++acc;
	}
	return(acc);
}

