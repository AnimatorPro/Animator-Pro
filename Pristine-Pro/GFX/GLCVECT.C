
#define GFX_INTERNALS
#include "gfx.h"

void cvect(Short_xy *ends, VFUNC dotout, void *dotdat)
{
	pj_cline(ends[0].x,ends[0].y,ends[1].x,ends[1].y,dotout,dotdat);
}
