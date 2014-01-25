#include "linklist.h"

/*********************************************/
void *slist_last(Slnode *list)
/* given pointer to first element of a list and index returns pointer to 
 * last element.  (NULL if list is empty). */
{
	Slnode *last = list;

	while (list)
		{
		last = list;
		list = list->next;
		}
	return(last);
}
