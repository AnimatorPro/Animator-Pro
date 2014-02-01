#include "gfx.h"
#include "rcel.h"

void pj_rcel_copy(Rcel *s, Rcel *d)
/* only works if both screens are depth 1 and the same dimensions */
{
	pj_blitrect(s,0,0,d,0,0,d->width,d->height);
	pj_cmap_copy(s->cmap, d->cmap);
}

