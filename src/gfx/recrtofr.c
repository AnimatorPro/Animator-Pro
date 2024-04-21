#include "rectang.h"

void crect_tofrect(register Cliprect *cr,register Fullrect *fr)

/* copys and converts a cliprect to a fullrect destination can be sources
 * cliprect */
{
	if((Cliprect *)&(fr->CRECTSTART) != cr)
		*((Cliprect *)&(fr->CRECTSTART)) = *cr;
	fr->width = fr->MaxX - fr->x;
	fr->height = fr->MaxY - fr->y;
}
