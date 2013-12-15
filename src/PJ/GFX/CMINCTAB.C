#include "cmap.h"

Boolean in_ctable(Rgb3 *rgb, Rgb3 *ctab, int count)
{
	while (--count >= 0)
	{
		if (rgb->r == ctab->r && rgb->g == ctab->g && rgb->b == ctab->b)
			return(1);
		++ctab;
	}
	return(0);
}
