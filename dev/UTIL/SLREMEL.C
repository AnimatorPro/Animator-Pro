#include "linklist.h"

void *remove_el(void *list, void *el)
{
Slnode tnode;
Slnode *last;
register Slnode *next;

	/* back up one with dummy node to handle NULL case and case where el
	 * is first in list */

	tnode.next = list;
	next = &tnode;

	for(;;)
	{
		last = next;
		if((next = next->next) == NULL)
			break;
		if(next == el)
		{
			last->next = next->next;
			break;
		}
	}
	return(tnode.next);
}
