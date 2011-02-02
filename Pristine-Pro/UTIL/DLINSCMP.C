#include "linklist.h"

/****************************************************************/
/* inserts a node in a list given compare function and the list
 * only works if list is in sorted order using this compare function */

void insert_compare(register Dlnode *node,
					Dlheader *list,
					FUNC compare)
/* FUNC a function that is called given *node A and *node B
 * compare(Dlnode *nodea Dlnode *nodeb) returns 0 if
 * A == B, < 0 if A < B, > 0 if A > B */
{
register Dlnode *lnode; /* current list node */
register Dlnode *nextnode;

	for(lnode = list->tails_prev;
	    NULL != (nextnode = lnode->prev);
		lnode = nextnode)
	{
		if((*compare)(node,lnode) > 0)
			break;
	}
	insert_after(node,lnode);
}
