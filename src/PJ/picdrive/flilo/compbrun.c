/* rastcomp.c - Some C code that mixes with the assembler code in
   comp.asm and skip.asm to make up compressed pixel packets suitable
   for incorporation into a FLI file.  See also writefli.c */

#define RASTCOMP_INTERNALS
#include "flilo.h"
#include "memory.h"
#include "ptrmacro.h"
#include "rastcomp.h"

static char *flow_brun_comp_line(char *s1, char *cbuf, int count)
{
int wcount;
register char *c;
register int bcount;
int op_count;
char *start_dif;
int dif_count;

	c = cbuf+1;
	op_count = 0;
	start_dif = s1;
	dif_count = 0;
	for (;;)
	{
		if (count < 3)
		{
			dif_count += count;
			while (dif_count > 0)
			{
				bcount = (dif_count < MAX_RUN ? dif_count : MAX_RUN );
				*c++ = -bcount;
				dif_count -= bcount;
				while (--bcount >= 0)
					*c++ = *start_dif++;
				op_count++;
			}
			*cbuf = op_count;
			return(norm_pointer(c));
		}
		else
		{
			bcount = (count < MAX_RUN ? count : MAX_RUN );
			if ((wcount = pj_bsame(s1, bcount)) >= 3)
			{
				while (dif_count > 0)
				{
					bcount = (dif_count < MAX_RUN ? dif_count : MAX_RUN );
					*c++ = -bcount;
					dif_count -= bcount;
					while (--bcount >= 0)
						*c++ = *start_dif++;
					op_count++;
				}
				*c++ = wcount;
				*c++ = *s1;
				op_count++;
				s1 += wcount;
				count -= wcount;
				start_dif = s1;
			}
			else
			{
				dif_count++;
				s1++;
				count -= 1;
			}
		}
	}
}
void *flow_brun_rect(Raster *r,void *cbuf,
				SHORT x,SHORT y,USHORT width,USHORT height)

/* brun compresses all pixels in a raster rectangle and puts them in cbuf
 * it returns length of buffer used in cbuf 0 if overflow */
{
register char *c;
char *cmax;
UBYTE *lbuf;

	c = cbuf;
	cmax = c + (width * height);
	lbuf = (UBYTE *)cmax;

	while(height--)
	{
		pj_get_hseg(r,lbuf,x,y++,width);
		c = flow_brun_comp_line(lbuf,c,width);
		if(c >= cmax)
			return(NULL);
	}
	return(c);
}
