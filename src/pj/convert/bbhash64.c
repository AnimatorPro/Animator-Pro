/* bbhash64.c - a color caching algorithm to speed up finding the closest
 * color in the current color map to an RGB value.   This one ignores the
 * lower two bits of the RGB components,  which is fine on VGA displays
 * where they are only six bits instead of 8 anyways.  It works by
 * building up a hash value of the lower 5 significant bits of the
 * RGB request, and using this as in index into a table.   If the
 * table entry has already been hit then it checks to see if the
 * last value stored there is the same as the requested value, and
 * if so returns it.  Otherwise it scans the color map for the
 * closest color and stores it in the table.  The typical hit rates
 * for this are about 88%, but it varies a lot depending on the picture.
 * In particular on all grey pictures this does not do as well as the
 * simpler 'ghash' algorithm.   
 *
 * If all 8 bits of the color components are significant, you must
 * use the much slower (only about 58% average hash hits) bhash.c
 * routines.
 */

#include "errcodes.h"
#include "pjbasics.h"
#include "bhash.h"
#include "ccache.h"


#define BSIZ (1024*4*8*sizeof(struct bhash) )

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
Rgb3 r64;

r64.r = rgb->r>>2;
r64.g = rgb->g>>2;
r64.b = rgb->b>>2;
/* first look for a hash hit */
h = c_hash+((((r64.r&0x1f)<<10) + ((r64.g&0x1f)<<5) + ((r64.b&0x1f))));
if (!h->valid || h->rgb.r != r64.r  || h->rgb.g != r64.g 
			 || h->rgb.b != r64.b )
	{
	++(cc->misses);
	h->closest = closestc(rgb,cmap->ctab,cmap->num_colors);
	h->rgb = r64;
	h->valid = 1;
	}
return(h->closest);
}

Ccache cc_bbhash64 =
	{
	"bbhash64",
	c_make,
	c_free,
	c_closest,
	};

