#include "linklist.h"

/****************************************************************/
Boolean isin_list(register Dlnode *testnode,Dlheader *list)

/* returns 1 if node found in list 0 if not */
{
register Dlnode *node;

	node = list->head;
	while(node->next != NULL)
	{
		if(testnode == node)
			return(1);
		node = node->next;
	}
	return(0);
}
