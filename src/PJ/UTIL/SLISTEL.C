#include "linklist.h"

/*********************************************/
void *slist_el(register Slnode *list, int ix)

/* given pointer to first element of a list and index returns pointer to 
 * element. NULL if not there */
{
	while(list && --ix >= 0)
		list = list->next;
	return(list);
}
