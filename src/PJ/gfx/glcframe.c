
#define GFX_INTERNALS
#include "gfx.h"


void cline_frame(SHORT x0,SHORT y0,SHORT x1,SHORT y1,
				 VFUNC dotout, void *dotdat)
/* Draw a hollow rectangle through pj_cline routine */
{
	pj_cline( x0, y0, x1, y0, dotout, dotdat);
	pj_cline( x1, y0, x1, y1, dotout, dotdat);
	pj_cline( x1, y1, x0, y1, dotout, dotdat);
	pj_cline( x0, y1, x0, y0, dotout, dotdat);
}

