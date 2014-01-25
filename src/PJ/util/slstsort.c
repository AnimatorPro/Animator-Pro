#include "memory.h"
#include "linklist.h"

/***********************************************************/
void *sort_slist(register Slnode *list, FUNC cmp, void *cmpdat)
{
register void **array, **array_pt;
register Slnode *pt;
register int elements, i;

	elements = slist_len(list);
	if (elements <= 1)
		return(list);	/* length 0 or 1 lists already sorted */

	array = pj_malloc( elements * sizeof(void *));
	if (array)
	{
		pt = list;
		array_pt = array;
		while ( pt )
		{
			*array_pt++ = pt;
			pt = pt->next;
		}
		sort_indarray(array, elements, cmp, cmpdat);
		array_pt = array;
		list = NULL;
		i = elements;
		while (--i >= 0)
		{
			pt = *array_pt++;
			pt->next = list;
			list = pt;
		}
		pj_free( array );
	}
	return(list);
}
