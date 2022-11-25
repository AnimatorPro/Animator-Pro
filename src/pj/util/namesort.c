#include <string.h>
#include "linklist.h"

static int cmp_names(void *l1, void *l2, void *cmpdat)
{
	(void)cmpdat;
	return strcmp(((Names *)l1)->name, ((Names *)l2)->name);
}

Names *sort_names(register Names *list)
{
	return((Names *)sort_slist((Slnode *)list, cmp_names, NULL));
}
