#include "stdio.h"
#include "null.h"

Errcode null_close_raster(NullRast *r)
{
printf("Raster closing after following calls:\n"
	   "\tget_dot    %6d\n"
	   "\tput_dot    %6d\n"
	   "\tset_colors  %6d\n",
	   r->hw.nm.gets, r->hw.nm.puts, r->hw.nm.colors);
return(Success);
}

void null_put_dot(NullRast *r, Pixel color, Coor x, Coor y)
{
++r->hw.nm.puts;
}

Pixel null_get_dot(NullRast *r, Coor x, Coor y)
{
++r->hw.nm.gets;
return(0);			/* that's a null dot for you... */
}

void null_set_colors(NullRast *r, LONG start, LONG count, unsigned char *table)
{
++r->hw.nm.colors;
}

static struct rastlib null_raster_library;

struct rastlib *null_get_rlib(Vdevice *dev, int mode, NullRast *r)
{
static got_lib = 0;

if (!got_lib)
	{
	null_raster_library.close_raster = null_close_raster;
	null_raster_library.put_dot = null_put_dot;
	null_raster_library.get_dot = null_get_dot;
	null_raster_library.set_colors = null_set_colors;
	got_lib = 1;
	}
return(&null_raster_library);
}

