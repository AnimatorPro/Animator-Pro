#include "linklist.h"

/***********************************************************/
void add_tail(list,node)

Dlheader *list;
Dlnode *node;
{
	node->next = (Dlnode *)(&list->tail);
	node->prev = list->tails_prev;
	node->prev->next = list->tails_prev = node;
}
