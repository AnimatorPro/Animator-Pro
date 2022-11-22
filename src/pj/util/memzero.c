#include "memory.h"
#include "ptrmacro.h"

void zero_lots(void *pt, LONG size)
{
int lsize;

size >>=1;	/* convert to word count */
while (size > 0)
	{
	if (size > 32000)
		lsize = 32000;
	else
		lsize = size;
	pj_stuff_words(0, pt, lsize);
	pt = norm_pointer(OPTR(pt,lsize));
	pt = norm_pointer(OPTR(pt,lsize));
	size -= lsize;
	}
}
