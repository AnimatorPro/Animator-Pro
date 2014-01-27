#define RASTCOMP_INTERNALS
#include "ptrmacro.h"
#include "rastcomp.h"

void *pj_unbrun_scale_line(BYTE *src, Coor sw, BYTE *dst, SHORT *xtable)

/* Uncompress one line of unbrun data in to a shrunken line specified
 * by the xtable input. sw must be the width of the source line.
 * returns pointer to next "chunk". The last entry in the xtable should
 * be at xtable[dw] and should be -1 */
{
int x;
int psize;
BYTE dchar;

	++src;	/* skip over obsolete opcount byte */

	x = 0;
	psize = 0;

	while(x < sw)
	{
		psize = (int)(*src++);
		if(psize >= 0)
		{
			x += psize;
			dchar = *src++;

			while(((USHORT)(*xtable)) < x)
			{
				++xtable;
				*dst++ = dchar;
			}
		}
		else
		{
			src -= x;
			x -= psize;
			while(((USHORT)(*xtable)) < x)
				*dst++ = src[*xtable++];
			src += x;
		}
	}
	return(src);
}
