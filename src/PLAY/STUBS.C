
#include "jimk.h"
#include "stubs.str"

int blocks_out;


void *
begmem(amount)
unsigned amount;
{
void *pt;

if ((pt = askmem(amount)) == NULL)
	outta_memory();
return(pt);
}

outta_memory()
{
continu_line(stubs_100 /* "Out of memory!" */);
}

gentle_freemem(pt)
void *pt;
{
if (pt != NULL)
	freemem(pt);
}

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



/* magic numbers tagged at beginning and end of allocated memory blocks */
#define START_COOKIE (0x41f3)
#define END_COOKIE (0x1599)
#define _lalloc  malloc




void *
laskmem(size)
long size;
{
WORD *endcookie;
WORD *pt;
long psize;

if (size > 0xffff0)
	return(0);
psize = ((size+6+15)>>4);
if ((pt = _lalloc(psize)) != NULL)
	{
#ifdef CHECKIT
	if (!check_mem_magic(pt, psize))
		{
		old_video();
		puts("Someone's stomping free memory!");
		unconfig_ints();
		exit(0);	/* okok... */
		}
#endif CHECKIT
	blocks_out++;
	*pt++ = psize;
	*pt++ = START_COOKIE;
	endcookie = long_to_pt(pt_to_long(pt-2)+(psize<<4L)-2);
	*endcookie = END_COOKIE;
	}
return(pt);
}


