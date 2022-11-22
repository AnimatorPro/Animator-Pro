#define RASTCALL_INTERNALS
#include "rastgfx.ih"

void pj_clear_rast(Raster *r)

/* clears an entire raster fast */
{
	pj_set_rast(r,0);
}
