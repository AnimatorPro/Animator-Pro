#include "linklist.h"

/***********************************************************/
int slist_ix(Slnode *list, Slnode *el)

/* given pointer to element and pointer to first in list returns
 * "index" or count away from list element is -1 if not found */
{
int ix = 0;

	while(list != NULL)
	{
		if (list == el)
			return(ix);
		++ix;
		list = list->next;
	}
	return(-1);
}
