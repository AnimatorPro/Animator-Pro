#include "cmap.h"

void set_color_rgb(Rgb3 *rgb, USHORT cnum, Cmap *cmap)

/* sets a color in a colormap */
{
	cmap->ctab[cnum] = *rgb;
}
