#include "linklist.h"

/***********************************************************/
/* returns header node is attached to 
 * (if DLL_SAFETY returns NULL if not attached) */

Dlheader *find_header(register Dlnode *node)
{
#ifdef DLL_SAFETY
	if(node->next == NULL)
		return(NULL);
#endif /* DLL_SAFETY */

	while(node->prev != NULL)
		node = node->prev;
	return((Dlheader *)node);
}
