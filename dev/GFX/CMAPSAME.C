#include "cmap.h"

Boolean cmaps_same(Cmap *s1, Cmap *s2)
{
USHORT cmpsize;

	if(s1->num_colors != s2->num_colors)
		return(0);

	cmpsize = (s1->num_colors * sizeof(Rgb3));
	return(pj_bcompare(s1->ctab, s2->ctab, cmpsize) == cmpsize);
}
