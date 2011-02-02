#include "linklist.h"

/**********************************************************/
Dlnode *get_head(register Dlheader *list)
{
register Dlnode *head;

	if((Dlnode *)list == list->tails_prev)
		return(NULL);

	head = list->head;
	list->head = head->next;
	list->head->prev = (Dlnode *)(&list->head);
#ifdef DLL_SAFETY  
	head->next = NULL;
#endif /* DLL_SAFETY */
	return(head);
}
