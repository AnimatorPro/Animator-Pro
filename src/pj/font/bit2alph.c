/*
 * bit2alph.c - this contains stuff to manipulate alpha-channels which I'm
 * defining here as a two dimensional byte-array used to represent various
 * levels of transparency.  0 bytes represent full transparency and 255
 * bytes represent full opaqueness.
 */

#include "rastext.h"

/*****************************************************************************
 * code to reduce a bit mask to an alpha channel...
 ****************************************************************************/
/*----------------------------------------------------------------------------
 * counts of how many bits are ones in a given byte value.
 *--------------------------------------------------------------------------*/

static UBYTE bitcounts[256] = {
	0, 1, 1, 2, 1, 2, 2, 3,  1, 2, 2, 3, 2, 3, 3, 4,	 /*   0 - 15  */
	1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 5,	 /*  16 - 31  */
	1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 5,	 /*  32 - 47  */
	2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 6,	 /*  48 - 63  */
	1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 5,	 /*  64 - 79  */
	2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 6,	 /*  80 - 95  */
	2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 6,	 /*  96 - 111 */
	3, 4, 4, 5, 4, 5, 5, 6,  4, 5, 5, 6, 5, 6, 6, 7,	 /* 112 - 127 */
	1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 5,	 /* 128 - 143 */
	2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 6,	 /* 144 - 159 */
	2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 6,	 /* 160 - 175 */
	3, 4, 4, 5, 4, 5, 5, 6,  4, 5, 5, 6, 5, 6, 6, 7,	 /* 176 - 191 */
	2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 6,	 /* 192 - 207 */
	3, 4, 4, 5, 4, 5, 5, 6,  4, 5, 5, 6, 5, 6, 6, 7,	 /* 208 - 223 */
	3, 4, 4, 5, 4, 5, 5, 6,  4, 5, 5, 6, 5, 6, 6, 7,	 /* 224 - 239 */
	4, 5, 5, 6, 5, 6, 6, 7,  5, 6, 6, 7, 6, 7, 7, 8 	 /* 240 - 255 */
};

/*----------------------------------------------------------------------------
 * masks to isolate bits in the current byte.
 *
 *	in the mask2 table, indicies 0, 2, 4, 6 access bit pairs normally, and
 *	indicies 1,3,5,7 are used for clipped access.
 *
 *	in the mask4 table, indicies 0, 4 access bit nybbles normally, and
 *	indicies, 1,2,3,5,6,7 are used for clipped access.
 *
 *	in the mask8 table, index 0 accesses the byte normally, and all others
 *	are used for clipped access.
 *
 *	clipped access is obtained by figuring the bit offset needed (x&7) and
 *	then adding the difference between the ending x and the available width.
 *	eg, if width is 6 and we're going after nybbles starting at bit 4, then
 *	x&7 is 4, plus endx-width (8-6) is 2, so we use the sixth element
 *	of the mask4 table (0x0C), which masks off the 2 unwanted low-order bits.
 *--------------------------------------------------------------------------*/

static UBYTE mask2[8] = {0xC0, 0x80, 0x30, 0x20, 0x0C, 0x08, 0x03, 0x02};
static UBYTE mask4[8] = {0xF0, 0xE0, 0xC0, 0x80, 0x0F, 0x0E, 0x0C, 0x08};
static UBYTE mask8[8] = {0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};

void bitmask_to_alpha_channel(Pixel *dest, int shrinker, UBYTE *bitplane
, int w, int h, int bpr
, int y_fraction)
/*****************************************************************************
 * convert a bitplane to a transparency mask by averaging pixel matricies.
 * can handle width or height not a multiple of shrinker.
 * output is bytes in the range of 0-255, indicating degree of transpancy
 * based on how many bits in the input matrix were on.
 * destination buffer size is ceiling(w/shrinker)*ceiling(h/shrinker) bytes.
 * shrinker must be 2, 4, or 8.
 ****************************************************************************/
{
int 	sum;		/* sum of bits in matrix currently being looked at */
int 	x;			/* current x in bitplane */
int 	y;			/* current y in bitplane */
int 	endx;		/* ending x in current matrix; used for clipping */
int		endy;		/* ending y in current matrix; used for clipping */
int 	ycount; 	/* count for lines to be processed in current matrix */
int		ylines;		/* Number of lines to shrink down for a given y. */
int		iheight;	/* Initial height of matrix. */
int 	shiftamt;	/* amount to upshift sum to make 0-255 range */
UBYTE	mask;		/* mask to grab rights bits from current byte */
UBYTE	*pmasktab;	/* -> table of masks for requested matrix size */
UBYTE	*pcurbyte;	/* -> current byte in bitplane */

switch (shrinker) 
  {
  case 2:
	pmasktab  = mask2;
	shiftamt = 6;
	break;
  case 4:
	pmasktab  = mask4;
	shiftamt = 4;
	break;
  case 8:
	pmasktab  = mask8;
	shiftamt = 2;
	break;
  default:
	return; 			/* ooops, illegal matrix size */
  }

iheight = shrinker - y_fraction;	/* Pretend we've a couple lines of zero
									 * first. */
for (y = 0; y < h;) 
	{
	endy = y + iheight;
	ylines = (endy < h ? iheight : h - y);
	for (x = 0; x < w; x += shrinker) 
		{
		sum 	 = 0;
		endx	 = x + shrinker;
		pcurbyte = bitplane + (x >> 3);
		if (endx <= w) 
			{						/* if we're not gonna go off the edge */
			mask = pmasktab[x & 7]; /* use the normal mask, else use a	 */
			} 
		else 
			{						/* mask that will clip off-edge bits.*/
			mask = pmasktab[(x & 7) + (endx - w)];
			}

		//mask = pmasktab[x&7];	/* DEBUG */

		/* Count up bits one line at a time. */
		ycount = ylines;
		while (ycount--) 
			{
			sum += bitcounts[(*pcurbyte) & mask];
			pcurbyte += bpr;
			}
		sum <<= shiftamt;
		if (sum > 255) 
			{
			sum = 255;
			}
		*dest++ = sum;
		}
	bitplane += (bpr * iheight);
	y += iheight;
	iheight = shrinker;		/* only first line may be short. */
	}
}


