#include <stdlib.h>
#include "memory.h"
#include "tfile.h"

void *trd_askmem(unsigned int count)
{
	return(lo_askmem(count));
}
void *trd_laskmem(long count)
{
	return(lo_askmem(count));
}
void *trd_askcmem(unsigned int count)
{
void *pt;

	if ((pt = lo_askmem(count)) != NULL)
		clear_mem(pt,count);
	return(pt);
}
long trd_freemem(void *pt)
{
	return(lo_freemem(pt));
}
void trd_freez(void **pt)
{
if (*pt != NULL)
	{
	trd_freemem(*pt);
	*pt = NULL;
	}
}
void *trd_flush_alloc(long size)
{
void *pt;

	if ((pt = lo_askmem(size)) == NULL)
	{
		if(trd_compact(size) >= Success);	/* try shrinking ram-disk */
			pt = trd_askmem(size);
	}
	return(pt);
}
