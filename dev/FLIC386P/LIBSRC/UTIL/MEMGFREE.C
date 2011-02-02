#include "memory.h"

void pj_gentle_free(void *pt)
{
	if (pt != NULL)
		pj_free(pt);
}
