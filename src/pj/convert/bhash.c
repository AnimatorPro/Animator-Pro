/* bhash.c - a color caching algorithm to speed up finding the closest
 * color in the current color map to an RGB value.    It works by
 * building up a hash value of the lower 4 significant bits of the
 * RGB request, and using this as in index into a table.   If the
 * table entry has already been hit then it checks to see if the
 * last value stored there is the same as the requested value, and
 * if so returns it.  Otherwise it scans the color map for the
 * closest color and stores it in the table.  The typical hit rates
 * for this are about 58%, but it varies a lot depending on the picture.
 * In particular on all grey pictures this does not do as well as the
 * simpler 'ghash' algorithm.   
 *
 * This is much slower than the bbhash64.c method,  but consumes
 * less memory, and preserves all 8 bits of the RGB components.
 */
#include "errcodes.h"
#include "pjbasics.h"
#include "bhash.h"
#include "ccache.h"

#define BSIZ (1024*4*sizeof(struct bhash) )

static Errcode c_make(Ccache *cc)
{
if((cc->data = pj_zalloc(BSIZ)) == NULL)
	return(Err_no_memory);
return(Success);
}

static void c_free(Ccache *cc)
{
pj_freez(&cc->data);
}


static int c_closest(Ccache *cc, Rgb3 *rgb, Cmap *cmap)
/* find closest color in color map to a true color value,
   using a cashe.  Has some problems with sign extension on Microsoft
   C.  */
{
#define c_hash ((struct bhash *)(cc->data))
register struct bhash *h;

/* first look for a hash hit */
h = c_hash+((((rgb->r&0xf)<<8) + ((rgb->g&0xf)<<4) + ((rgb->b&0xf))));
if (!h->valid || h->rgb.r != rgb->r  || h->rgb.g != rgb->g 
			 || h->rgb.b != rgb->b )
	{
	++(cc->misses);
	h->closest = closestc(rgb,cmap->ctab,cmap->num_colors);
	h->rgb = *rgb;
	h->valid = 1;
	}
return(h->closest);
}

Ccache cc_bhash =
	{
	"bhash",
	c_make,
	c_free,
	c_closest,
	};

