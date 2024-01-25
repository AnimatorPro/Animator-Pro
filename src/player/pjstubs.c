/* pjstubs.c - This is some functions that in fact will never be
 * called.  They're just needed to link.  
 */

#include "render.h"

/* The font system wants these four render routines... */
void render_mask_blit(UBYTE *mplane, SHORT mbpr,
					  SHORT mx, SHORT my,
					  void *drast, /* currently ignored uses vb.pencel */
					  SHORT rx, SHORT ry, USHORT width, USHORT height, ... )
{
}

void render_mask_alpha_blit(UBYTE *alpha, int abpr, int x, int y, int w, int h
, Rcel *r, Pixel oncolor)
{
}

Errcode render_hline(register SHORT y,register SHORT x0,SHORT x1,Raster *r)
{
return Success;
}

Errcode render_opoly(Poly *p, Boolean closed)
{
return Success;
}
