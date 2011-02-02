#include "cmap.h"

Cmap *clone_cmap(Cmap *toclone)
{
Cmap *clone;

	if(pj_cmap_alloc(&clone,toclone->num_colors) >= Success)
		pj_cmap_copy(toclone,clone);
	return(clone);
}
