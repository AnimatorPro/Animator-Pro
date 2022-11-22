#include <string.h>
#include "flicglue.h"

Boolean pj_cmaps_same(PjCmap *s1, PjCmap *s2)
{
USHORT cmpsize;

	if(s1->num_colors != s2->num_colors)
		return(0);

	cmpsize = (s1->num_colors * sizeof(Rgb3));
	return(pj_bcompare(s1->ctab, s2->ctab, cmpsize) == cmpsize);
}
