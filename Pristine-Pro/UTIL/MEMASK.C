#include "memory.h"

void *pj_zalloc(long size)
/* same as laskmem bu returns it cleared "c" */
{
void *mem;

	if(NULL != (mem = pj_malloc(size)))
		zero_structure(mem,size);
	return(mem);
}
void *pj_zalloc(unsigned int size)
{
	return(pj_zalloc((long)size));
}
