
/* rgbcmap.c - routines to compress an arbitrary # of colors to an
 * arbitrary smaller number of colors.	Works pretty cool except
 * if you have duplicate colors in the source it sometimes hangs.
 * This is a modification of the old cpack routines that is much
 * faster and just about as good.
 */

#include "errcodes.h"
#include "memory.h"
#include "aaconfig.h"
#include "wndo.h"

#include "convert.h"
#include "rgbcmap.h"


			/* A table of the count of bits set in a byte */
UBYTE bits_in_byte[256] = {
0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

/* A table converting a bit position to a byte with the bit set in that
 * position.
 */
unsigned char bitmasks[8] =
	{
	0x80, 0x40, 0x20, 0x10,
	0x8, 0x4, 0x2, 0x1,
	};

static long count_bits_set(UBYTE *h)
/*****************************************************************************
 * Count up the number of 1 bits in a table of bytes.
 ****************************************************************************/
{
	long acc;
	long size;

	size = (cs.colors_256) ? HIST256_SIZE : HIST64_SIZE;

	acc = 0;
	while (--size >= 0)
		acc += bits_in_byte[*h++];

	return(acc);
}

Errcode alloc_histogram(UBYTE **h)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (cs.colors_256)
		return ealloc(h, HIST256_SIZE);
	else
		return ealloc(h, HIST64_SIZE);

}

void freez_histogram(UBYTE **h)
/*****************************************************************************
 *
 ****************************************************************************/
{
	pj_freez(h);
}

void hist_set_bits(UBYTE *h,  UBYTE **rgb_bufs, int count)
/*****************************************************************************
 * Set bits in histogram corresponding to colors held in r/g/b.  The
 * input colors are assumed to be in 0-255 format.
 ****************************************************************************/
{
	long	cbit;
	int 	cbyte;
	UBYTE	*r = *rgb_bufs++;
	UBYTE	*g = *rgb_bufs++;
	UBYTE	*b = *rgb_bufs;

	if (cs.colors_256)
		{
		while (--count >= 0)
			{
			cbit = ((long)*r++<<16) + (*g++ <<8) + *b++;
			cbyte = cbit>>3;
			h[cbyte] |= bitmasks[cbit&7];
			}
		}
	else
		{
		while (--count >= 0)
			{
			cbit = ((long)(*r++&0xfc)<<10)
				 + ((*g++&0xfc)<<4)
				 + ((*b++&0xfc)>>2);
			cbyte = (cbit>>3);
			h[cbyte] |= bitmasks[cbit&7];
			}
		}
}

static void hist_to_ctab256(UBYTE *h, UBYTE *cm)
/*****************************************************************************
 * Given a histogram make up an RGB triple (each component 0-63)
 * for each bit in histogram that is set.	(You better count the bits
 * to make sure the cm buffer is big enough to hold the result.)
 ****************************************************************************/
{
	int r,g,b;
	int limit;
	UBYTE c;
	UBYTE mask;

	if (cs.colors_256)
		limit = 256;
	else
		limit = 64;

	for (r=0; r<limit; ++r)
		for (g=0; g<limit; ++g)
			for (b=0; b<limit; )
				{
				if ((c = *h++) == 0)
					b += 8;
				else
					{
					mask = 0x80;
					while (mask)
						{
						if (c&mask)
							{
							cm[0] = r;
							cm[1] = g;
							cm[2] = b;
							cm += 3;
							}
						++b;
						mask >>= 1;
						}
					}
				}
}

static void lshift_cmap(UBYTE *tab, int count)
/*****************************************************************************
 * Shift every element left by 2.  (Convert from 0-63 to 0-255
 * representation.)
 ****************************************************************************/
{
	while (--count >= 0)
		*tab++ <<= 2;
}



static Errcode find_newc(int usedc, int freec, Rgb3 *ctab, int threshold,
					 Rgb3 *lsource, long lscount, USHORT *curthresh)
/*****************************************************************************
 * Do a pack with a given threshold
 ****************************************************************************/
{
	int closestix;
	long new;
	int dif;
	Errcode err;

	soft_status_line("!%6ld%4d%4d", "ctop_cpack", 
					lscount-usedc, freec, threshold);

	if ((err = soft_abort("cpack_abort")) < Success)
		return(err);

	new = 0;
	while (--lscount >= 0)
		{
		if (*curthresh > threshold)
			{
			closestix = closestc(lsource,ctab,usedc);
			if ((dif = color_dif(ctab+closestix, lsource)) > threshold)
				{
				if (new >= freec)
					return(Err_overflow);
				new++;
				ctab[usedc] = *lsource;
				usedc++;
				*curthresh = 0;
				}
			else
				*curthresh = dif;
			}
		++curthresh;
		++lsource;
		}
	return(new);
}

static Errcode fpack_ctable(
		Rgb3 *source,	/* Source - array scount long of rgb triples */
		LONG scount,	 /* source size */
		Rgb3 *dest, /* Destination cmap  MUST be COLORS long even if
				  * dcount less.  Duplicate colors here sometimes hang
					up this routine */
		int dcount)  /* Dest size (will be packed to this many) */
/*****************************************************************************
 * Pack colors with adaptive threshold streaming algorithm.
 ****************************************************************************/
{
#ifdef DEBUG
	int have_reported = FALSE;
#endif

	int threshold;	/* how far away is closest color before put in this one? */
	int newused;			/* # of VGA colors we just ate... */
	int usedc;
	int hiwater;
	USHORT *curthresh = NULL;
	int ctsize = scount * sizeof(*curthresh);

	if (scount <= dcount)	/* if will fit in dest copy it */
		{
		copy_mem(source, dest, scount*sizeof(Rgb3));
		return Success;
		}

	if ((curthresh = pj_malloc(ctsize)) == NULL)
		return(Err_no_memory);

	*dest = *source;		/* copy in the first color */
	usedc = 1;
	threshold = 1024;
							/* Do the first threshold.	If have overflow
							 * set the threshold to 2x and do it again...*/
FINDHI:

	memset(curthresh,0xff,ctsize);
	*curthresh = 0;
	newused = find_newc(usedc, dcount-usedc, dest,
						threshold, source, scount,curthresh);
	if (newused == Err_overflow)
		{
		threshold *= 2;
		goto FINDHI;
		}
	else if (newused < Success)
		goto OUT;
	usedc += newused;

							/* Keep doing thresholds until we run out of
							 * colors. */
	hiwater = threshold;
	while (threshold >= 3)
		{
		if ((newused = find_newc(usedc, dcount-usedc, dest,
								 threshold, source, scount,curthresh))
			< Success)
			{
			if (newused != Err_overflow)
				goto OUT;
			else
				{
#ifdef DEBUG
  if (!have_reported)
	{
	have_reported = TRUE;
	continu_box("Original version would have stopped here.\n"
				"Colors used now = %d\n", usedc);
	}
#endif
				newused = Success;
				threshold += threshold / 3;
				if (threshold >= hiwater)
					goto OUT;
				}
			}
		else
			{
			hiwater = threshold;
			threshold /= 2;
			}
		usedc += newused;
		}

OUT:

#ifdef DEBUG
  continu_box("Used colors = %d",usedc);
#endif

	pj_gentle_free(curthresh);
	return newused;
}

Errcode hist_to_cmap(UBYTE **phist, Cmap *cmap)
/*****************************************************************************
 * This converts a histogram to a cmap of a fixed size.
 * Consumes *phist in the process.
 ****************************************************************************/
{
	Errcode err 	  = Success;
	UBYTE	*big_ctab = NULL;
	int 	ccount;

	ccount = count_bits_set(*phist);

	if ((big_ctab = pj_malloc(ccount*3)) == NULL)
		{
		err = Err_no_memory;
		goto OUT;
		}

	hist_to_ctab256(*phist, big_ctab);
	if (!cs.colors_256)
		lshift_cmap((UBYTE *)big_ctab, ccount*3);

	pj_freez(phist);

				/* copy in the menu colors first */
	memset(cmap->ctab,0,cmap->num_colors*sizeof(Rgb3));
	copy_mem(vconfg.mc_ideals,&cmap->ctab[FIRST_MUCOLOR],
			 NUM_MUCOLORS*sizeof(Rgb3));

	err = fpack_ctable((Rgb3 *)big_ctab,ccount,
		cmap->ctab,cmap->num_colors-NUM_MUCOLORS);

OUT:
	pj_gentle_free(big_ctab);
	return(err);
}

