
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


/* some pointer manipulation routines for the 8086 */
unsigned
ptr_offset(offset, seg)
int offset, seg;
{
return(offset);
}

unsigned
ptr_seg(offset, seg)
int offset, seg;
{
return(seg);
}

long
make_long(l)
long l;
{
return(l);
}


void *
make_ptr(pt)
void *pt;
{
return(pt);
}
long 
pt_to_long(offset, seg)
unsigned offset, seg;
{
long result;

result = seg;
result <<= 4;
result += offset;
return(result);
}
