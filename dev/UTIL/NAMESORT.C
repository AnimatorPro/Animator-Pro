#include "linklist.h"

static int cmp_names(Names *l1, Names *l2)
{
	return(strcmp(l1->name, l2->name) );
}

Names *sort_names(register Names *list)
{
	return((Names *)sort_slist((Slnode *)list, cmp_names));
}
