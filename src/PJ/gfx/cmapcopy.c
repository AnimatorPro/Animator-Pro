#include "cmap.h"
#include "memory.h"

void pj_cmap_copy(Cmap *s,Cmap *d)

/* copys color palette from s to d */
{
USHORT csize;

	if((csize = s->num_colors) > d->num_colors)
		csize = d->num_colors;
	csize *= 3;
	pj_copy_bytes(s->ctab, d->ctab, csize);
}
