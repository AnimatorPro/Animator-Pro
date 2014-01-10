/* complcc.c Some C code that mixes with the assembler code in
   comp.asm and skip.asm to make up compressed pixel packets suitable
   for incorporation into an animator 1.0  FLI file. will only compress
   a 320 X 200 screen results are unpredictable with other sizes */

#define RASTCOMP_INTERNALS
#include "memory.h"
#include "ptrmacro.h"
#include "rastcomp.h"

static char *sbrc_line(char *s1, char *s2, char *cbuf, int count)
{
register int wcount;
register char *c;
int op_count;
int next_match;
int bcount;

	op_count = 0;
	c = cbuf+1;
	for (;;)
	{
		if (count <= 0)
		{
			goto OUT;
		}
		/* first find out how many bytes to skip... */
		wcount = pj_bcompare(s1, s2, count);
		if ((count -= wcount) <= 0)
			goto OUT;	/* same until the end... */
		/* if skip is longer than 255 have to break it up into smaller ops */
		while (wcount > 255)
		{
			s1 += 255+1;
			s2 += 255;
			wcount -= 255+1;
			/* make dummy copy 1 op */
			*c++ = (char)255;
			*c++ = 1;
			*c++ = *s2++;
			op_count++;
		}
		/* save initial skip and move screen pointer to 1st different byte */
		*c++ = wcount;
		s1 += wcount;
		s2 += wcount;
		op_count++;

		/* if have skipped to near the end do a literal copy... */
		if (count <= INERTIA)
		{
			*c++ = count;
			pj_copy_bytes(s2,c,count);
			c += count;
			goto OUT;
		}

		/* now look for a run of same... */
		bcount = count;
		if(bcount > MAX_RUN)
			bcount = MAX_RUN;

		wcount = pj_bsame(s2, bcount);
		if (wcount >= INERTIA)	/* it's worth doing a same thing thing */
		{
			next_match = pj_til_next_skip(s1, s2, wcount, INERTIA);

			/* if it's in our space and a decent size */
			if (next_match < wcount)
			{
				/* we'll cut short same run for the skip */
				wcount = next_match;
			}
			*c++ = -wcount;
			*c++ = *s2;
			s1 += wcount;
			s2 += wcount;
			count -= wcount;
		}
		else	/* doing a literal copy.  What can we do to make it short? */
		{
			/* figure out how long until the next worthwhile "skip" */
			/* Have wcount of stuff we can't skip through. */
			wcount = pj_tnsame(s2,
					pj_til_next_skip(s1, s2, bcount, INERTIA-1),
					INERTIA);
			/* Say copy positive count as lit copy op, and put bytes to copy
			   into the compression buffer */
			*c++ = wcount;
			pj_copy_bytes(s2,c,wcount);
			s1 += wcount;
			s2 += wcount;
			c += wcount;
			count -= wcount;
		}
	}
OUT:
	*cbuf = op_count;
	return(norm_pointer(c));
}

void *pj_lccomp_rects(Raster *r1, void *cbuf, 
				   SHORT x1, SHORT y1, 
				   Raster *r2, 
				   SHORT x2, SHORT y2, USHORT width, USHORT height)

/* does a load count skip count compression of differences between 
 * two rectangles in rasters note may be out of the same raster */
{
SHORT skip_count, j;
char *c;
char *oc;
USHORT acc;
LONG bytes_left;
USHORT last_real;
SHORT bpr1;
SHORT bpr2;
UBYTE *line1;
UBYTE *line2;

	bytes_left = width * height;

	/* if we can get'um directly do it fast way */
	if(r1->type == RT_BYTEMAP) 
	{
		bpr1 = ((Bytemap *)r1)->bm.bpr;
		line1 = ((Bytemap *)r1)->bm.bp[0] + ((y1 - 1) * bpr1);
	}
	else
	{
		bpr1 = 0;
		line1 = FOPTR(cbuf,bytes_left);
	}
	if(r2->type == RT_BYTEMAP) /* if we can get'um directly do it fast way */
	{
		bpr2 = ((Bytemap *)r2)->bm.bpr;
		line2 = ((Bytemap *)r2)->bm.bp[0] + ((y2 - 1) * bpr2);
	}
	else
	{
		bpr2 = 0;
		line2 = FOPTR(cbuf,bytes_left+width);
	}

	bytes_left -= bytes_left/16;

	/* find out how many lines of s1 and s2 are the same 
	 * at the moment will not handle an odd width */

	acc = (width>>1);	/* SHORTS in line for fcompare */

	skip_count = 0;

	/* skip over blank lines at top */
	for(;;)
	{
		/* If all same do special case for empty frame*/
		if(height == 0)
			return(OPTR(cbuf,2));

		if(bpr1)
			line1 += bpr1;
		else
			pj_get_hseg(r1,line1,x1,y1++,width);

		if(bpr2)
			line2 += bpr2;
		else
			pj_get_hseg(r2,line2,x2,y2++,width);

		/* note this will leave line2/2 loaded or incremented to the 
		 * line with difference for loop below */

		if((USHORT)pj_fcompare(line1, line2, acc) != acc)
			break;
		--height;
		++skip_count;
	}


	/* store offset of 1st real line and set up for main line-at-a-time loop */

	*(SHORT *)cbuf = skip_count;
	cbuf = OPTR(cbuf,sizeof(SHORT));
	c = OPTR(cbuf,sizeof(SHORT));

	last_real = 0;	/* keep track of last moving line */

	for (j=1; j<=height;j++)
	{
		oc = c;

		/* if whole line is the same note waste: first time this isn't true */

		if ((USHORT)pj_fcompare(line1,line2,acc) == acc)
		{
			*c++ = 0;	/* set op count to 0 */
		}
		else	/* compress line */
		{
			c = sbrc_line(line1,line2,c,width);
			last_real = j;
		}

		bytes_left -= SIZE(oc,c);

		if (bytes_left <= 0) 
			return(NULL);

		/* get next lines */

		if(bpr1)
			line1 += bpr1;
		else
			pj_get_hseg(r1,line1,x1,y1++,width);

		if(bpr2)
			line2 += bpr2;
		else
			pj_get_hseg(r2,line2,x2,y2++,width);
	}

	/* set # of lines in compression to last real, removing empty bottom lines
	 * from buffer */

	*(USHORT *)cbuf = last_real;
	c -= height-last_real;
	return(pj_enorm_pointer(c));
}
