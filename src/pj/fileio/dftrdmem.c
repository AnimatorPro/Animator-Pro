#include "dfile.ih"
#include "memory.h"

void *trd_laskmem(long size)
{
	return(pj_malloc(size));
}
void trd_freemem(void *p)
{
	pj_free(p);
}
