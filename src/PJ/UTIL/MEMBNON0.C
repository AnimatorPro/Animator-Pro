#include "stdtypes.h"

int nonzero_bytes(register UBYTE *c, int array_size)
/* count number of non-zero elements in a byte array */
{
register UBYTE *maxc;
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

