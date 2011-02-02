#include "rastgfx.ih"

void hscale_uncopy(Raster *f, void *csource, int scale, int lines)
/* isnt this really a vertical scaler ? */
{
Pixel *s;
long y;
long modit;

	if(lines > f->height)
		lines = f->height;

	s = csource;
	modit = f->width*scale;
	for (y=0; y<lines; ++y)
	{
		PUT_HSEG(f, s, 0, y, f->width);
		s += modit;
	}
}
