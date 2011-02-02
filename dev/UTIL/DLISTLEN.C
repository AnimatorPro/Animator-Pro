#include "linklist.h"

/***********************************************************/
/* returns length of doubly linked list */

LONG listlen(Dlheader *list)
{
register Dlnode *node;
register Dlnode *next;
register LONG len;

	for(node = list->head, len = 0;
		NULL != (next = node->next);
		node = next)
	{
		++len;
	}
	return(len);
}
