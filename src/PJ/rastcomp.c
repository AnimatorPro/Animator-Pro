/* rastcomp.c - Some C code that mixes with the assembler code in
   comp.asm and skip.asm to make up compressed pixel packets suitable
   for incorporation into a FLI file.  See also writefli.c */

#define RASTCOMP_C
#include "ptrmacro.h"
#include "rastcomp.h"

#define MAX_RUN 127

extern USHORT pj_bcompare(), pj_fcompare(), pj_bsame();

static char *brun_comp_line(char *s1, char *cbuf, int count)
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

#define INERTIA 4

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
			next_match = pj_tnskip(s1, s2, wcount,INERTIA);

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
			wcount = pj_tnsame(s2,pj_tnskip(s1,s2,bcount,INERTIA-1),INERTIA);
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
char *c, *cmax;
SFUNC xcompare;
USHORT acc;
USHORT last_real;
SHORT bpr1;
SHORT bpr2;
UBYTE *line1;
UBYTE *line2;

	c = cbuf;
	cmax = c + (width * height);

	/* if we can get'um directly do it fast way */
	if(r1->type == RT_BYTEMAP) 
	{
		bpr1 = ((Bytemap *)r1)->bm.bpr;
		line1 = ((Bytemap *)r1)->bm.bp[0] + ((y1 - 1) * bpr1);
	}
	else
	{
		bpr1 = 0;
		line1 = (UBYTE *)cmax;
	}
	if(r2->type == RT_BYTEMAP) /* if we can get'um directly do it fast way */
	{
		bpr2 = ((Bytemap *)r2)->bm.bpr;
		line2 = ((Bytemap *)r2)->bm.bp[0] + ((y2 - 1) * bpr2);
	}
	else
	{
		bpr2 = 0;
		line2 = ((UBYTE *)cmax) + width;
	}

	/* find out how many lines of s1 and s2 are the same 
	 * at the moment will not handle an odd width */

	if(width & 1) /* odd width */
	{
		acc = width;
		xcompare = (SFUNC)pj_bcompare;
	}
	else
	{
		acc = (width>>1);	/* SHORTS in line for fcompare */
		xcompare = (SFUNC)pj_fcompare;
	}

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

		if((USHORT)(*xcompare)(line1, line2, acc) != acc)
			break;
		--height;
		++skip_count;
	}


	/* store offset of 1st real line and set up for main line-at-a-time loop */

	*(SHORT *)cbuf = skip_count;
	cbuf = OPTR(cbuf,sizeof(SHORT));
	c = cbuf;

	last_real = 0;	/* keep track of last moving line */

	for (j=1; j<=height;j++)
	{
		/* if whole line is the same note waste: first time this isn't true */

		if ((USHORT)(*xcompare)(line1,line2,acc) == acc)
		{
			*c++ = 0;	/* set op count to 0 */
		}
		else	/* compress line */
		{
			c = sbrc_line(line1,line2,c,width);
			last_real = j;
		}

		if(c >= cmax)
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
	return(c);
}


void *pj_brun_rect(Raster *r,void *cbuf,
				SHORT x,SHORT y,USHORT width,USHORT height)

/* brun compresses all pixels in a raster rectangle and puts them in cbuf
 * it returns length of buffer used in cbuf 0 if overflow */
{
register char *c;
register int bpr;
char *cmax;
UBYTE *lbuf;

	c = cbuf;
	cmax = c + (width * height);

	if(r->type == RT_BYTEMAP) /* if we can get'um directly do it the fast way */
	{
		bpr = ((Bytemap *)r)->bm.bpr;
		/* note (y - 1): pre-increment below */
		lbuf = ((Bytemap *)r)->bm.bp[0] + (bpr * (y - 1)); 
	}
	else
	{
		bpr = 0;
		lbuf = (UBYTE *)cmax;
	}

	while(height--)
	{
		if(bpr)
			lbuf += bpr;
		else
			pj_get_hseg(r,lbuf,x,y++,width);

		c = brun_comp_line(lbuf,c,width);
		if(c >= cmax)
			return(NULL);
	}
	return(c);
}

