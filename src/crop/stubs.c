
/* stubs.c - misc functions ripped from various files of vpaint to get
   cropper to link */

#include "jimk.h"

color_dif(rgb,c)
BYTE *rgb, *c;
{
register int dc, dif;

dc = rgb[0]-c[0];
dif = dc*dc;
dc = rgb[1]-c[1];
dif += dc*dc;
dc = rgb[2]-c[2];
dif += dc*dc;
return(dif);
}

unconfig_ints()
{
}

upc_char(c)
UBYTE c;
{
if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
return(c);
}

void *
list_el(list, ix)
register Name_list *list;
int ix;
{
while (list && --ix>= 0)
	{
	list = list->next;
	}
return(list);
}
