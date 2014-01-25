#include "linklist.h"

/***********************************************************/
void add_head(Dlheader *list, Dlnode *node)
{
	node->prev = (Dlnode *)(&list->head);
	node->next = list->head;
	node->next->prev = list->head = node;
}
