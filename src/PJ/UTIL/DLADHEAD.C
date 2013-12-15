#include "linklist.h"

/***********************************************************/
void add_head(list,node)

Dlheader *list;
Dlnode *node;
{
	node->prev = (Dlnode *)(&list->head);
	node->next = list->head;
	node->next->prev = list->head = node;
}
