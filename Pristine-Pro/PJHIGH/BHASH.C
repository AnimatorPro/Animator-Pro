/*****
 Stuff for 'bhash', which is a system for speeding up finding the
 closest color to a given true color.  It builds up a hash table, so that
 colors previously looked up are likely to be in the hash and need not
 be recomputed.  Typical hit rate is 85% or better, but varies a lot from
 picture to picture.

 See also BHASHLKP.ASM which contains the dithering and lookup code.

 *******/

#include "errcodes.h"
#include "pjbasics.h"
#include "bhash.h"

#define BSIZ (16*16*16*2*sizeof(ULONG)) // ie, 32kbytes,
										//	 4096 domains of 2 slots of 4 bytes

#define SPECIAL_CASE_INIT_VALUE 0x00000100 // see comments in bhashlkp.asm

//static struct bhash *bhash;

typedef struct bhash_control {
	void   *cachedata;
	Rgb3   *ctab;
	int    rederr;
	int    grnerr;
	int    bluerr;
	ULONG  drgb;	  // temp var used by dithering, blackbox to us here
#ifdef SHOW_STATS
	int calls, hits1, hits2, fhits, misses;
#endif
	} BhashCtl;

BhashCtl bhashctl;			// global so assembler code can see it.

void free_bhash(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (bhashctl.cachedata != NULL) {
		pj_freez(&bhashctl.cachedata);
#ifdef SHOW_STATS
		continu_box("Hash stats:\n"
					"  Calls:      %d\n"
					"  Hits 1:     %d\n"
					"  Hits 2:     %d\n"
					"  False hits: %d\n"
					"  Misses:     %d\n"
						bhashctl.calls, bhashctl.hits1, bhashctl.hits2,
						bhashctl.fhits, bhashctl.misses);
#endif
	}
}

Errcode make_bhash(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	ULONG *pwrk;

	free_bhash();							// make sure it's gone.
	clear_mem(&bhashctl, sizeof(bhashctl)); // clean out dithering, etc.

	if(NULL == (bhashctl.cachedata = pj_malloc(BSIZ)))
		return(Err_no_memory);

	clear_mem(bhashctl.cachedata, BSIZ);	// init cache area to zeros.
	pwrk = bhashctl.cachedata;				// init the first two slots in
	pwrk[0] = SPECIAL_CASE_INIT_VALUE;		// the cache to a special value.
	pwrk[1] = SPECIAL_CASE_INIT_VALUE;		// see comments in bhashlkp.asm.

	bhashctl.ctab = vb.pencel->cmap->ctab;	// do all the dereferencing once.

	return(Success);
}

Boolean is_bhash(void)	/* used by GEL tool to see if bhash already in place */
/*****************************************************************************
 *
 ****************************************************************************/
{
	return (bhashctl.cachedata != NULL);
}

#ifdef CCODE	/* this routine moved to bhashlkp.c */

int bclosest_col(register Rgb3 *rgb,int count,SHORT dither)
/*****************************************************************************
 * find closest color in color map to a true color value,
 * using a cache.  This for speed will truncate the RGB values
 * to 0-63 effectively.  I don't like it, but the speed difference
 * is about a factor of 3, and right now that's the way all the
 * video boards are.  (Except the HGSC). In a perfect world we'd have a
 * flag in the device driver to tell us how to do this.  (<-Yeah!)
 ****************************************************************************/
{
register struct bhash *h;
int i;
int r,g,b;
int closest;
Rgb3 drgb;	/* rgb after dither adjustments */
Rgb3 srgb;	/* scaled down to 0-63 */

	bhashctl.prgb = rgb;
	bhashctl.ctabcount = count;

	if (dither) {
		closest = bhash_dithered_lookup();
	} else {
		closest = bhash_lookup();
	}
	return closest;

	if (dither)
	{
	register int temp;

		temp = rgb->r + rerr;
		if (temp < 0)
			temp = 0;
		if (temp > RGB_MAX-1)
			temp = RGB_MAX-1;
		drgb.r = r = temp;
		temp = rgb->g + gerr;
		if (temp < 0)
			temp = 0;
		if (temp > RGB_MAX-1)
			temp = RGB_MAX-1;
		drgb.g = g = temp;
		temp = rgb->b + berr;
		if (temp < 0)
			temp = 0;
		if (temp > RGB_MAX-1)
			temp = RGB_MAX-1;
		drgb.b = b = temp;
		rgb = &drgb;
	}

	srgb.r = rgb->r>>2;
	srgb.g = rgb->g>>2;
	srgb.b = rgb->b>>2;

	/* first look for a hash hit */
	i = ((((srgb.r&0xf)<<8) + ((srgb.g&0xf)<<4) + ((srgb.b&0xf))));
	h = bhash+i;
	closest = h->closest;
	if (closest != 0) {
		if (h->rgb.r == srgb.r &&  h->rgb.g == srgb.g && h->rgb.b == srgb.b) {
			goto GOTIT;
		}
	}
	closest = h->closest = closestc(rgb,vb.pencel->cmap->ctab,count);
	h->rgb = srgb;
GOTIT:
	if (dither)
	{
		rgb = vb.pencel->cmap->ctab + closest;
		rerr = 3*(r - rgb->r)/4;
		gerr = 3*(g - rgb->g)/4;
		berr = 3*(b - rgb->b)/4;
	}
	return(closest);
}
#endif /* CCODE */

