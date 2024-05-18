#include <string.h>
#include "errcodes.h"
#include "memory.h"
#include "picdrive.h"
#include "rif.h"

static void bits_to_bytes(UBYTE *in, UBYTE *out, int w, UBYTE out_mask)
/*****************************************************************************
 * Subroutine to help convert from bit-plane to byte-a-pixel representation
 * or the out_mask into out byte-plane wherever a bit in in bit-plane is set.
 ****************************************************************************/
{
int k;
UBYTE imask;
UBYTE c;

while (w > 0)
	{
	imask = 0x80;
	k = 8;
	if (k > w)
		k = w;
	c = *in++;
	while (--k >= 0)
		{
		if (c&imask)
			*out |= out_mask;
		out += 1;
		imask >>= 1;
		}
	w -= 8;
	}
}

Errcode conv_bitmaps(UBYTE *oplanes[], int pcount, 
					 int bpr, int width, int height, Rcel *screen)
/*****************************************************************************
 * Convert a bit-plane pixel representation to a byte-a-pixel representation.
 * (From Amiga style to Mode 13H VGA/MCGA style.)
 ****************************************************************************/
{
int i,j;
UBYTE *pix_buf;
UBYTE omask;
UBYTE *planes[8];

if (pcount > 8)
	return(Err_pdepth_not_avail);
memcpy(planes, oplanes, pcount*sizeof(UBYTE *));
if ((pix_buf = pj_malloc(width)) == NULL)
	return(Err_no_memory);
for (i=0; i<height; ++i)
	{
	memset(pix_buf, 0, width);		
	omask = 1;
	for (j=0; j<pcount; ++j)
		{
		bits_to_bytes(planes[j], pix_buf, width, omask);
		omask <<= 1;
		planes[j] += bpr;
		}
	pj_put_hseg(screen, pix_buf, 0, i, width);
	}
pj_free(pix_buf);
return(Success);
}
