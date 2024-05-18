/* brunline.c */

#include "memory.h"
#include "ptrmacro.h"
#include "rastcomp.h"

void *pj_brun_comp_line(BYTE *src, BYTE *cbuf, int count)
{
int wcount;
BYTE *c;
register int bcount;
int op_count;
BYTE *start_dif;
int dif_count;

	c = cbuf+1;
	op_count = 0;
	start_dif = src;
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
			if ((wcount = pj_bsame(src, bcount)) >= 3)
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
				*c++ = *src;
				op_count++;
				src += wcount;
				count -= wcount;
				start_dif = src;
			}
			else
			{
				dif_count++;
				src++;
				count -= 1;
			}
		}
	}
}

