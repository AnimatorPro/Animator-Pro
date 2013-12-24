/*****************************************************************************
 *
 ****************************************************************************/

#define REXLIB_INTERNALS

#include "pocolib.h"
#include "syslib.h"

void pj_free(void *block)
{
	free(block);
}

void *pj_malloc(size_t amount)
{
	return malloc(amount);
}

void *pj_zalloc(size_t amount)
{
	return zalloc(amount);
}

unsigned long pj_time(unsigned long *ptime)
{
	if (ptime)
		*ptime = 0;
	return 0;		/* ::sigh:: this is the best we can do */
}

