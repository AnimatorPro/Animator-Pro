#include "linklist.h"

/* Function: insert_after
 *
 *  node - node to insert after.
 *  lnode - attached node.
 */
void insert_after(Dlnode *node, Dlnode *lnode)
{
	node->prev = lnode;
	node->next = lnode->next;
	node->next->prev = lnode->next = node;
}
