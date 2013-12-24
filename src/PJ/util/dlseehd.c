#include "linklist.h"

/**********************************************************/
Dlnode *see_head(register Dlheader *list)
{
	if((Dlnode *)list == list->tails_prev)
		return(NULL);
	return(list->head);
}
