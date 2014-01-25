#include "linklist.h"

/*************************************************************/
/* moves adds all of the fromlists nodes to
 * the tail of the tolist the nodes remain in the same order
 * as in the fromlist */

void list_totail(Dlheader *fromlist, Dlheader *tolist)
{
Dlnode *fromhead;

	if(NULL == (fromhead = see_head(fromlist))) /* nothing to move */
		return;

	/* link fromhead to totail */
	fromhead->prev = tolist->tails_prev;  /* point fromhead back to totail */
	fromhead->prev->next = fromhead;      /* point totail to fromhead */

	/* link fromtail to tolist->tail */
	tolist->tails_prev = fromlist->tails_prev;
	tolist->tails_prev->next = (Dlnode *)&tolist->tail; 	

	/* clear fromlist */
	init_list(fromlist);
}
