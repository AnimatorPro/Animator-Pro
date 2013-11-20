
/* lccomp.c - Some C code that mixes with the assembler code in
   comp.asm and skip.asm to make up compressed pixel packets suitable
   for incorporation into a FLI file.  See also writefli.c */


#include "lccomp.h"
#include "peekpok_.h"
#include "ptr.h"

#define MAX_RUN 127

static char *
brun_comp_line(const char *s1, char *cbuf, int count)
{
int wcount;
register char *c;
register int bcount;
int op_count;
const char *start_dif;
int dif_count;
int same_count;

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
		if ((wcount = bsame(s1, bcount)) >= 3)
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

static char *
sbrc_line(const char *s1, const char *s2, char *cbuf, int count)
{
register int wcount;
int i;
register char *c;
int op_count;
int same_count;
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
	wcount = bcompare(s1, s2, count);
	if ((count -= wcount) <= 0)
		goto OUT;	/* same until the end... */
	/* if skip is longer than 255 have to break it up into smaller ops */
	while (wcount > 255)
		{
		s1 += 255+1;
		s2 += 255;
		wcount -= 255+1;
		/* make dummy copy 1 op */
		*c++ = 255;
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
		copy_bytes(s2,c,count);
		c += count;
		goto OUT;
		}

	/* now look for a run of same... */
	bcount = count;
	if (bcount > MAX_RUN)
		bcount = MAX_RUN;

	wcount = bsame(s2, bcount);
	if (wcount >= INERTIA)	/* it's worth doing a same thing thing */
		{
		next_match = til_next_skip(s1, s2, wcount,INERTIA);

		if (next_match < wcount) /* if it's in our space and a decent size */
			{			/* we'll cut short same run for the skip */
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
		/* figure out how long until the next worthwhile 'skip' */
		/* Have wcount of stuff we can't skip through. */
		wcount = til_next_same(s2,
				til_next_skip(s1, s2, bcount, INERTIA-1),
				INERTIA);
		/* Say copy positive count as lit copy op, and put bytes to copy
		   into the compression buffer */
		*c++ = wcount;
		copy_bytes(s2,c,wcount);
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

unsigned int *
lccomp(const char *s1, const char *s2, unsigned int *cbuf,
		unsigned int width, unsigned int height)
{
int skip_count, lcount, j;
char *c;
char *oc;
unsigned acc;
long total;
unsigned last_real;

/* find out how many lines of s1 and s2 are the same */
acc = (width>>1);	/* WORDS in line */
j = height;
skip_count = 0;
total = 0;
while (--j >= 0)
	{
	if (fcompare(s1, s2, acc) != acc)
		break;
	s1 += width;
	s2 += width;
	skip_count++;
	}

/* If all same do special case for empty frame*/
if (skip_count == height)	
	return(cbuf+1);

/* store offset of 1st real line and set up for main line-at-a-time loop */
*cbuf++ = skip_count;
height -= skip_count;
c = (char *)(cbuf+1);
last_real = 0;	/* keep track of last moving line */
for (j=1; j<=height;j++)
	{
	oc = c;
	if (fcompare(s1,s2,acc) == acc)	/* whole line is the same */
		{
		*c++ = 0;	/* set op count to 0 */
		}
	else	/* compress line */
		{
		c = sbrc_line(s1,s2,c,width);
		last_real = j;
		}
	total += pt_to_long(c) - pt_to_long(oc);
	if (total >= 60000L)
		return(NULL);
	s1 += width;
	s2 += width;
	}
/* set # of lines in compression to last real, removing empty bottom lines
   from buffer */
*cbuf = last_real;
c -= height-last_real;
return(enorm_pointer(c));
}

unsigned int *
brun(const char *s1, const char *s2, int *cbuf,
		int width, int height)
{
register char *c;
char *oc;
long total = 0;

/* store offset of 1st real line and set up for main line-at-a-time loop */
c = (char *)(cbuf);
while (--height >= 0)
	{
	oc = c;
	c = brun_comp_line(s1,c,width);
	total += pt_to_long(c) - pt_to_long(oc);
	if (total >= 60000L)
		return(NULL);
	s1 += width;
	}
return(enorm_pointer(c));
}
