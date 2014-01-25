#include "linklist.h"

/* Function: insert_before
 *
 *  node - node to insert before.
 *  lnode - attached node.
 */
void insert_before(Dlnode *node, Dlnode *lnode)
{
	node->next = lnode;
	node->prev = lnode->prev;
	node->prev->next = lnode->prev = node;
}
