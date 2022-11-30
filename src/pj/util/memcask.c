#include "memory.h"

void *pj_zalloc(size_t size)
/* same as laskmem but returns it cleared "c" */
{
void *mem;

	if(NULL != (mem = pj_malloc(size)))
		clear_mem(mem,size);
	return(mem);
}
