/***************************************************************
Pict file pdr modules:

	Created by Peter Kennard.  Sept 29, 1991
		Implements non-pattern fill mode bitmap operations
		and transfer mode blit. Parses and ignores all other
		opcodes.
****************************************************************/
/* brunline.c */
#include "stdtypes.h"
#include "errcodes.h"
#include "memory.h"

#define MAX_RUN (0x0080) 

extern USHORT pj_bsame();

Errcode brun_unpack_3compbytes(BYTE *packline, UBYTE *buf, int len)
/* Unpack records that are byte run compressed so that one ends up with 
 * 4 contiguous buffers for r,g,b, and alpha each bpr/4 long.  We ignore 
 * any alpha info and just get the r g and b. */
{
	len -= len>>2;
	return(brun_unpack_line(packline,buf,len));
}
Errcode wrun_unpack_line(BYTE *packline, UBYTE *buf, int len)
/* Unpack a buffer using apple type 16 bit word run compression 
   and put into buf argument.  The decompressed length expected is in len
   bytes. */
{
union { SHORT *w; BYTE *b; } packed;
USHORT *p;
int count;
int lenleft;

	packed.b = packline;
	/* uncompress it into the buffer mon */
	p = (USHORT *)buf;
	lenleft = len >>= 1;  /* Apple sez it must be even. */

	while(lenleft > 0)
	{
		if ((count = *packed.b++) < 0)	/* it's a run */
		{
			count = 1-count;
			pj_stuff_words(*packed.w++,p,count);
			p += count;
			lenleft -= count;
		}
		else
		{
			++count;
			pj_copy_words(packed.w,p,count);
			p += count;
			packed.w += count;
			lenleft -= count;
		}
	}
	if(lenleft != 0)
		return(Err_format);

	intel_swap_words(buf,len);
	return(Success);
}
Errcode brun_unpack_line(BYTE *packline, UBYTE *buf, int len)
/* Unpack a buffer using apple type byte run compression 
   and put into buf argument.  The decompressed length expected is in len
   bytes. */
{
UBYTE *p;
int count;

	/* uncompress it into the buffer mon */
	p = buf;

	while (len > 0)
	{
		if ((count = *packline++) < 0)	/* it's a run */
		{
			count = 1-count;
			stuff_bytes(*packline++,p,count);
			p += count;
			len -= count;
		}
		else
		{
			++count;
			copy_bytes(packline,p,count);
			p += count;
			packline += count;
			len -= count;
		}
	}
	return((len == 0) ? Success : Err_format);
}
char *brun_pack_line(char *src, char *cbuf, int count)
/* Compresses a buffer using apple spec byte run compression.  Compresses src
 * into cbuf.  Count is the number of bytes of src to compress.
 * This function returns the pointer to the next available byte in cbuf. */
{
int same_count;
int bcount;
char *dif_start;
int dif_count;


	dif_start = src;
	dif_count = 0;

	while(count >= 3)
	{
		if((same_count = pj_bsame(src,Min(count,MAX_RUN))) >= 3)
		{
			while(dif_count > 0)
			{
				bcount = Min(dif_count,MAX_RUN);
				dif_count -= bcount;
				*cbuf++ = bcount-1;
				copy_bytes(dif_start,cbuf,bcount);
				dif_start += bcount;
				cbuf += bcount;
			}
			count -= same_count;
			*cbuf++ = 1-same_count;
			*cbuf++ = *src;
			src += same_count;
			dif_start = src;
		}
		else
		{
			--count;
			++dif_count;
			++src;
		}
	}

	dif_count += count;
	while(dif_count > 0)
	{
		bcount = Min(dif_count,MAX_RUN);
		dif_count -= bcount;
		*cbuf++ = bcount-1;
		copy_bytes(dif_start,cbuf,bcount);
		dif_start += bcount;
		cbuf += bcount;
	}
	return(cbuf);
}

