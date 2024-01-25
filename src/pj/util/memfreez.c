#include "memory.h"

void pj_freez(void *v) 
/************************************************************************* 
 * This takes as input the address of a pointer.  It checks to see if the
 * pointer is NULL.  If not it frees the pointer and sets it to NULL.
 * In typical usage you might have:
 * 		char *pt;
 *		if ((pt = pj_malloc(size)) != NULL)
 *			{
 *			... do some processing ...
 *			pj_freez(&pt);
 *			}
 * Parameters:
 *		void	*v;		An address of a pointer to a piece of memory
 *						gotten from pj_malloc, pj_zalloc, etc.
 *************************************************************************/
/* really to be a pointer to the memory pointer but void for convenience */
{
	if (*(void **)v != NULL)
		pj_free(*(void **)v);
	*(void **)v = NULL;
}
