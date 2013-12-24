#include "linklist.h"

void init_list(list)

Dlheader *list;
{
	list->head = (Dlnode *)&(list->tail);
	list->tail = NULL;
	list->tails_prev = (Dlnode *)list;
}
