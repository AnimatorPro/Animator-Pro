#include "rastcall.ih"

void set_rast(Raster *r,Pixel color)

/* sets entire raster to a color fast */
{
	(r->lib->set_rast)((Raster *)r,(Pixel)color);
}
