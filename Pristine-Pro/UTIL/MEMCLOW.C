#include "errcodes.h"
#include "memory.h"
#include "ptrmacro.h"

extern void *malloc();



/* note that size must immediately precede memory !!!! */

typedef struct cmem_chunk {
	LONG size;
	char mem[1];
} Lochunk;

long mem_free = 0x7FFFFFFF;
long init_mem_free = 0x7FFFFFFF;

Errcode init_mem(long max_mem)
/* this is actually optional in the cmem library but the mem_free vars will
 * not be accurate and there will be no system overhead cushion */
{
register long size;
char *pool;

	if (max_mem <= 0)
		{
		init_mem_free = size = dos_mem_free();	/*see what's available */
		size -= 32*1024;   		/*leave some for dos etc. */
		}
	else
		{
		init_mem_free = size = max_mem;
		}
	mem_free = size;
	return(1);
}

void *c_askmem(long nbytes)
{
Lochunk *pt;

	nbytes += OFFSET(Lochunk,mem);

	if( nbytes < mem_free 
		&& (pt = malloc( nbytes )) != NULL)
	{
		pt->size = nbytes;
		mem_free -= nbytes;
		return(&(pt->mem));
	}
	return(NULL);
}
long c_freemem(void *pt)
/* returns size freed keeps track of mem_free */
{
Lochunk *lc;
long size;

	lc = TOSTRUCT(Lochunk,mem,pt);
	size = (lc->size - OFFSET(Lochunk,mem));
	mem_free += lc->size;
	free(lc);
	return(size);
}
