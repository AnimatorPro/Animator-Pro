#include "gfx.h"

void tblit_image(Image *i,Pixel *colors, Raster *rast, Coor x,Coor y)
/* color 0 is tcolor ( colors[0] ignored never accessed) */
{
	if(i->type != ITYPE_BITPLANES)
		return;
	if(i->depth != 1)
		return;

	pj_mask1blit(i->image, Bitmap_bpr(i->width),0,0,
			  rast,x,y,i->width,i->height, colors[1] );
}
