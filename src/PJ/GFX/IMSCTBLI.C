#define RASTGFX_INTERNALS
#include "gfx.h"


void image_scale_tblit(Image *i,Pixel *colors,
						Raster *dst,Coor dx, Coor dy, Ucoor dw, Ucoor dh)
{
Raster irast;
Tcolxldat tcx;

	if(image_to_rast(i, &irast, dst) < Success)
		return;

	tcx.tcolor = 0;
	tcx.xlat = colors;
	pj_scale_blit(&irast,0,0,irast.width,irast.height,dst,dx,dy,dw,dh,
				  &tcx);
}

