#include "linklist.h"

/**********************************************************/
Dlnode *get_tail(register Dlheader *list)
{
register Dlnode *tail;

	if((Dlnode *)list == (tail = list->tails_prev))
		return NULL;

	list->tails_prev = tail->prev;
	list->tails_prev->next = (Dlnode *)(&list->tail);
#ifdef DLL_SAFETY  
	tail->next = NULL;
#endif /* DLL_SAFETY */
	return(tail);
}
