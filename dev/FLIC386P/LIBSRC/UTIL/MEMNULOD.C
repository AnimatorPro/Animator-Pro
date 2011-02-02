#include "stdtypes.h"

void pj_load_array_nulls(void **source, void **dest, int num_pointers)
/* sets pointers in dest that are NULL to pointers in source */
{
void **maxdest;

	maxdest = dest + num_pointers;
	while(dest < maxdest)
	{
		if(*dest == NULL)
			*dest = *source;
		++dest;
		++source;
	}
}
