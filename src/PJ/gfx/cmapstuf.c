#include "gfx.h"

void stuff_cmap(Cmap *cmap, Rgb3 *color)
{
Rgb3 *cmcolor;
Rgb3 *maxcolor;

	cmcolor = cmap->ctab;
	maxcolor = cmcolor + cmap->num_colors;

	while(cmcolor < maxcolor)
		*cmcolor++ = *color;
}
