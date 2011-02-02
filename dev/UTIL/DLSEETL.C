#include "linklist.h"

/**********************************************************/
Dlnode *see_tail(register Dlheader *list)
{
	if((Dlnode *)list == list->tails_prev)
		return(NULL);
	return(list->tails_prev);
}
