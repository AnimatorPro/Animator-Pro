#include "linklist.h"
#include "memory.h"

void free_slist(void *list)

/* frees all nodes in a singly linked list assuming the pointer to the node
 * is an allocated element */
{
Slnode *lst = list;
Slnode *next;

while (lst)
	{
	next = lst->next;
	pj_free(lst);
	lst = next;
	}
}
