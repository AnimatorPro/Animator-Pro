#include "linklist.h"

void
sort_indarray(void **array, LONG count,
		int (*cmp)(void *a, void *b, void *cmpdat), void *cmpdat)
/* a little shell on an array of indirect pointers to things.
 * takes a function that is like strcmp() to compare things pointed to */
{
register void **pt1, **pt2;
register void *swap;
register LONG swaps;
register LONG space, ct;

	if (count < 2)  /*very short arrays are already sorted*/
		return;

	space = count/2;
	--count; /* since look at two elements at once...*/
	for (;;)
	{
		swaps = 1;
		while (swaps)
		{
			pt1 = array;
			pt2 = array + space;
			ct = count - space + 1;
			swaps = 0;
			while (--ct >= 0)
			{
				if ((*cmp)(*pt1, *pt2, cmpdat) < 0)
				{
					swaps = 1;
					swap = *pt1;
					*pt1 = *pt2;
					*pt2 = swap;
				}
				pt1++;
				pt2++;
			}
		}
		if ( (space /= 2) == 0)
			break;
	}
}
