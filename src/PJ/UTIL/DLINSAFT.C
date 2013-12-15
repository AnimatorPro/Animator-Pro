#include "linklist.h"

/***********************************************************/
void insert_after(node,lnode)

register Dlnode *node; /* node to insert after */
register Dlnode *lnode; /* attached node */
{
	node->prev = lnode;
	node->next = lnode->next;
	node->next->prev = lnode->next = node;
}
