#include "memory.h"

void *pj_zalloc(unsigned size)
/* same as laskmem bu returns it cleared "c" */
{
void *mem;

	if(NULL != (mem = pj_malloc(size)))
		clear_mem(mem,size);
	return(mem);
}
