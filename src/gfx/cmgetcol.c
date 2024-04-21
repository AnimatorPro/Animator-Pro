#include "cmap.h"

void get_color_rgb(USHORT cnum, Cmap *cmap, Rgb3 *rgb)
/* sets a color in a colormap */
{
	*rgb = cmap->ctab[cnum];
}
