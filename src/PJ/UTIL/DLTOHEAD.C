#include "linklist.h"

/**************************************************************/
/* moves adds all of the fromlists nodes to
 * the head of the tolist the nodes remain in the same order
 * as in the fromlist */

void list_tohead( register Dlheader *fromlist, register Dlheader *tolist)
{
Dlnode *fromtail;
extern Dlnode *see_tail();

	if(NULL == (fromtail = see_tail(fromlist))) /* nothing to move */
		return;

	/* link fromtail to tohead */
	fromtail->next = tolist->head;
	tolist->head->prev = fromtail;

	/* link fromhead onto tolist->head */
	tolist->head = fromlist->head;
	tolist->head->prev = (Dlnode *)&tolist->head;

	/* clear fromlist */
	init_list(fromlist);
}
