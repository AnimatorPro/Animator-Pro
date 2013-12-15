#include "linklist.h"

void *reverse_slist(void *l)
{
Names *newl, *pt, *next;

newl = NULL;
pt = l;
while (pt != NULL)
	{
	next = pt->next;
	pt->next = newl;
	newl = pt;
	pt = next;
	}
return(newl);
}
