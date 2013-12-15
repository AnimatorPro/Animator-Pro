#include "rastcall.h"
#include "cmap.h"

void pj_cmap_load(void *raster, Cmap *cmap)

/* loads entire cmap into raster display hardware if it has hardware that needs
 * setting otherwise does nothing */
{
	pj_set_colors(raster, 0, cmap->num_colors, (UBYTE *)(cmap->ctab));
}
