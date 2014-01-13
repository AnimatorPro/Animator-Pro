/* bhash.c
 *
 * Stuff for 'bhash', which is a system for speeding up finding the
 * closest color to a given true color.  It builds up a hash table, so that
 * colors previously looked up are likely to be in the hash and need not
 * be recomputed.  Typical hit rate is 85% or better, but varies a lot from
 * picture to picture.
 *
 *
 * int bclosest_col(Rgb3 *rgb, int count, SHORT dither)
 *
 *   this routine applies optional error diffusion dithering to the input
 *   rgb value, then looks up the color using a cached lookup.	if the color
 *   is not found in the cache, the closestc() routine is called, and the
 *   color it finds is stored into the cache.  if dithering is requested,
 *   the weighted difference between the requested and found colors is saved
 *   for use on the next call.	the cache tends to get hits in the 65-100%
 *   range, generally on the high end of that range.  (95% is typical except
 *   when dithering is used.  dithering can generate tens of thousands of
 *   unique colors, which sometimes flushes the cache too quickly.)
 *
 *   the dithering error from the last call is rolled into the rgb values of
 *   the current call using the following (conceptual) formula:
 *	  temp = rgb->r + rederr;
 *	  if (temp < 0)
 *	    temp = 0;
 *	  if (temp > 255)
 *	    temp = 255;
 *	  drgb.r = temp;
 *   the same logic is applied to each of the r,g,b components, of course.
 *   after the error-corrected values are obtained, the rgb pointer parameter
 *   is modified to point at 'drgb' so that the lookup and post-lookup-
 *   dithering will use the error-corrected values.
 *
 *   the post-lookup-dithering takes the weighted difference between the
 *   closest color we found and the color we were trying to find, as follows:
 *	 rederr = 3*(drgb.r - foundrgb->r)/4;
 *   this formula is applied to each of the components, and the values are
 *   saved for use in the next call.
 *
 *   the cached lookup reduces the 24-bit rgb value to an 18-bit value used
 *   for the actual lookup (ie, we punt some significance for speed), and a
 *   12-bit hash key to use in the cache data lookup.  the hash key is made
 *   up of the four lowest bits of the 6-bit reduced rgb components.  (ie,
 *   from the original 8-bit components, the hash keys are the bits xxKKKKxx).
 *   the twelve bit hash key then indexes into the table of 4096 cache
 *   domains.  each domain contains two slots, and each slot occupies four
 *   bytes, giving a cache data area of 32k.  within each cache slot, the
 *   four bytes are mapped out such that the first byte is the index of the
 *   closest color found to match the rgb value, and the next 3 bytes hold
 *   the rgb value itself (scaled down to 64-level/6-bit values, remember).
 *   thus, when loaded as a dword, the closest color index ends up in the low
 *   byte of the register and the rgb ends up high-aligned in the upper 18
 *   bits of the register in bgr order.  this doesn't really matter since the
 *   values in the cache data area are visible only to us in this routine,
 *   and we always deal with them packed into this particular sequence. (see
 *   the comments on the barrel-shifter-torture-test code, below).
 *
 *   once the 12 bit key is built, it is multiplied by 8 to index to the
 *   correct domain within the cache data area, and the value from the first
 *   slot in the domain is loaded.  the rgb value in the slot is compared
 *   to the 18-bit rgb we're looking for.  if they match, the closest index
 *   number pulled from that cache slot is returned.  if the rgb value in the
 *   first cache slot is not a match, we check the other slot.	if that rgb
 *   value matches, we return that slot's closest index.  if the second slot's
 *   rgb value doesn't match either, we have a total cache miss.  in this
 *   case, the data from the first cache slot is transferred to the second
 *   slot, and the closestc() routine is used to find the color in the color
 *   table.  the index returned by closestc() is stored into the first cache
 *   slot.  this shuffling of entries on a cache miss implements a very simple
 *   (and cheap in terms of cycles) LRU flushing mechanism.
 *
 *   there is no special 'valid' field within the cache to mark slots that
 *   are used; the cache data is sort of self-validating, for lack of a
 *   better term.  the cache starts out initialized to zeros.  when we
 *   compare a requested color to the value in a slot that has never been
 *   used, the values would match only if the color being searched for is
 *   also all zeros.  this, of course, can only happen for domain zero; in any
 *   other slot that has been used, the rgb values cannot be all zeros or they
 *   wouldn't have hashed to that slot.  domain zero is therefore handled as a
 *   special case.  (if it weren't, a lookup for rgb {0,0,0} could end up
 *   returning index 0 from the cache, and there's no guarantee that the
 *   first palette slot holds {0,0,0} really).	the special case handling is
 *   simple:  the rgb values occupy the upper 18 bits of the dword, and the
 *   index occupies the lower 8 bits, leaving 6 bits inbetween that will
 *   always be zero because of the way we pack 24 bits down to 18.  those bits
 *   are included in the compare of the requested value against the value in
 *   the cache data, but since it is the requested values that eventually
 *   end up getting stored as we makes entries in the cache, those bits of
 *   each dword always remain zeros.  so, when we init the cache (in bhash.c)
 *   we just set the first two cache slots to values that have one of those
 *   bits on, which means that the first time a request is made to find color
 *   {0,0,0}, the compare against what's in the cache slots will be unequal,
 *   forcing a lookup.	once the lookup has filled in a real value, {0,0,0}
 *   lookups will work fine after that.
 *
 *   well, now the comments are longer than the code, so on with it...
 *
 *   oh yeah -- the order of things in the code below is tuned towards the
 *   idea of fastest performance when dithering is turned off and with the
 *   assumption that the first cache slot will hold a hit most of the time.
 *   it looks a bit disordered, but the idea is to take the fall-thru case
 *   of most branches for the conditions we expect to happen the most.
 */

#include "errcodes.h"
#include "pjbasics.h"
#include "bhash.h"

/* See comments above. */
#define SPECIAL_CASE_INIT_VALUE 0x00000100

typedef union bhash_slot {
	struct {
		unsigned index : 8;
		int r : 6;
		int g : 6;
		int b : 6;
		unsigned x : 6;
	} s;
	uint32_t all;
} BhashSlot;
STATIC_ASSERT(bhash, sizeof(BhashSlot) == 4);

typedef struct bhash_domain {
	BhashSlot slot[2];
} BhashDomain;

typedef struct bhash_control {
	BhashDomain *cachedata;
	Rgb3   *ctab;
	int    rederr;
	int    grnerr;
	int    bluerr;
#ifdef SHOW_STATS
	int calls, hits1, hits2, fhits, misses;
#endif
} BhashCtl;

/* 32kb hash table: 4096 domains of 2 slots of 4 bytes */
#define BSIZ (16*16*16*sizeof(BhashDomain))

static BhashCtl bhashctl;

static int
clamp(int a, int b, int c)
{
	return Max(a, Min(b, c));
}

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
	free_bhash();							// make sure it's gone.
	clear_mem(&bhashctl, sizeof(bhashctl)); // clean out dithering, etc.

	if(NULL == (bhashctl.cachedata = pj_malloc(BSIZ)))
		return(Err_no_memory);

	clear_mem(bhashctl.cachedata, BSIZ);	// init cache area to zeros.

	/* Init the first two slots in the cache to a special value. */
	bhashctl.cachedata[0].slot[0].all = SPECIAL_CASE_INIT_VALUE;
	bhashctl.cachedata[0].slot[1].all = SPECIAL_CASE_INIT_VALUE;

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
BhashDomain *h;
int i;
int r,g,b;
int closest;
Rgb3 drgb;	/* rgb after dither adjustments */
BhashSlot srgb;

	/* Add the dithering error from the last call into the current call. */
	if (dither)
	{
		r = clamp(0, rgb->r + bhashctl.rederr, RGB_MAX - 1);
		g = clamp(0, rgb->g + bhashctl.grnerr, RGB_MAX - 1);
		b = clamp(0, rgb->b + bhashctl.bluerr, RGB_MAX - 1);

		drgb.r = r;
		drgb.g = g;
		drgb.b = b;

		rgb = &drgb;
	}

	/* Look for a cache hit. */
	srgb.all = 0;
	srgb.s.r = rgb->r>>2;
	srgb.s.g = rgb->g>>2;
	srgb.s.b = rgb->b>>2;

	i = ((((srgb.s.r&0xf)<<8) + ((srgb.s.g&0xf)<<4) + ((srgb.s.b&0xf))));
	h = bhashctl.cachedata + i;

	/* Check first slot. */
	srgb.s.index = h->slot[0].s.index;
	if (srgb.all == h->slot[0].all)
		goto GOTIT;

	/* Check second slot. */
	srgb.s.index = h->slot[1].s.index;
	if (srgb.all == h->slot[1].all)
		goto GOTIT;

	/* Total cache miss. */
	srgb.s.index = closestc(rgb, bhashctl.ctab, count);
	h->slot[1].all = h->slot[0].all;
	h->slot[0].all = srgb.all;

GOTIT:

	/* Save the dithering error for use in the next call. */
	closest = srgb.s.index;
	if (dither)
	{
		rgb = bhashctl.ctab + closest;

		/* Note: the original asm used an arithmetic shift here which
		 * produces different results when the error is negative.
		 */
		bhashctl.rederr = 3*(r - rgb->r)/4;
		bhashctl.grnerr = 3*(g - rgb->g)/4;
		bhashctl.bluerr = 3*(b - rgb->b)/4;
	}
	return(closest);
}
