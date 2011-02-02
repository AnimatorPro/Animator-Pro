#include "linklist.h"

/**********************************************************/
void insert_before(node,lnode)

register Dlnode *node; /* node to insert before */
register Dlnode *lnode; /* attached node */
{
	node->next = lnode;
	node->prev = lnode->prev;
	node->prev->next = lnode->prev = node;
}
