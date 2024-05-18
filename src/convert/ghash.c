/* ghash.c - a color caching algorithm to speed up finding the closest
 * color in the current color map to an RGB value.    This works by
 * using the green component of the RGB request as an index into a
 * table.  If a color is already present in the table and it matches
 * the request then we don't have to scan the whole color map.
 *
 * On arbitrary pictures the hit rate for this is only about 56%, so
 * usually we'll use more effective bbhash64 method.  However this
 * one works with nearly 100% efficiency on greyscale images.
 */
#include "errcodes.h"
#include "memory.h"
#include "ccache.h"


typedef struct ghash
	{
	UBYTE valid;
	UBYTE ix;
	Rgb3 rgb;
	} Ghash;


static Errcode c_make(Ccache *cc)
{
if ((cc->data = pj_zalloc(RGB_MAX*sizeof(Ghash))) == NULL)
	return(Err_no_memory);
return(Success);
}

static void c_free(Ccache *cc)
{
pj_freez(&cc->data);
}


static int c_closest(Ccache *cc, Rgb3 *rgb, Cmap *cmap)
{
Ghash *gh;
#define c_hash ((Ghash *)(cc->data))

/* first look for a hash hit */
gh = c_hash+rgb->g;
if (!gh->valid || rgb->r != gh->rgb.r || rgb->b != gh->rgb.b)
	{
	++(cc->misses);
	gh->valid = TRUE;
	gh->ix = closestc(rgb,cmap->ctab,cmap->num_colors);
	gh->rgb = *rgb;
	}
return(gh->ix);
}


Ccache cc_ghash =
	{
	"ghash",
	c_make,
	c_free,
	c_closest,
	};
