#include "cmap.h"

ULONG cmap_crcsum(Cmap *cmap)
{
	return(mem_crcsum(cmap->ctab,cmap->num_colors * sizeof(Rgb3)));
}
