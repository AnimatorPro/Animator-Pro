#ifdef SLUFFED
#include "gfx.h"

void blit_image(Image *i,Pixel *colors, Raster *rast, Coor x,Coor y)
{
	if(i->type != ITYPE_BITPLANES)
		return;
	if(i->depth != 1)
		return;

	pj_mask2blit(i->image, Bitmap_bpr(i->width),0,0,
			  rast,x,y,i->width,i->height,colors[1],colors[0]);
}
#endif /* SLUFFED */
