#include "linklist.h"

/*************************************************************/
/* insert sorts a list given compare function and the list
 * list will be in ascending order based on compare() */

void isort_list(Dlheader *list, FUNC compare)

/* a function that is called given *node A and *node B
 * compare(Dlnode *nodea Dlnode *nodeb) returns 0 if
 * A == B, < 0 if A < B, > 0 if A > B */
{
Dlheader tlist; /* tempory list */
Dlnode *node;

	init_list(&tlist);
	list_tohead(list,&tlist);

	while(NULL != (node = get_head(&tlist)))
		insert_compare(node,list,compare);
}
