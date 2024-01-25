
#include "memory.h"
#include "errcodes.h"

Errcode ealloc(void **pt, long size)
	/* allocate cleared buffer of size into *pt.  
	 * Return Err_no_memory or Success */
{
if ((*pt = pj_zalloc(size)) == NULL)
	return(Err_no_memory);
else
	return(Success);
}

