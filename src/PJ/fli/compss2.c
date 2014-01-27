
/* ss2.c - Some C code that mixes with the assembler code in
   comp.asm and skip.asm to make up compressed pixel packets suitable
   for incorporation into a FLI file.  See also writefli.c */


#define RASTCOMP_INTERNALS
#include "stdtypes.h"
#include "asm.h"
#include "fli.h"
#include "memory.h"
#include "ptrmacro.h"
#include "rastcomp.h"

#define INERT 3


#ifdef DEBUG
static SHORT *fat_ss2_line(SHORT *s1, SHORT *s2, SHORT *cbuf, 
					   int count, int skipct) 
/* makes an essentially uncompressed line (just one big copy op) */
{
BYTE *packet;

*cbuf++ = 1;
packet = (BYTE *)cbuf;
packet[0] = 0;
packet[1] = count;
cbuf += 1;
pj_copy_words(s2, cbuf, count);
return(cbuf+count);
}
#endif /* DEBUG */

/* This is a "ss2" Autodesk Animator Pro style compression routine
 * done up as a state machine.   It's been tested a little bit now,
 * but hasn't been put into PJ yet.
 */
#define MAX_RUN 127		/* Largest RUN */
#define MAX_COPY 127	/* Largest COPY */
#define MAX_SKIPS 127	/* Largest SKIP */

#define COPYBREAK 2		/* Break off COPY when have SKIP this big */
#define RUNBREAK 3		/* Break off RUN when have SKIP this big */
#define RUN_IN_COPY_BREAK 4	/* Break off COPY when have RUN this big */

typedef struct
	{
	unsigned char skip;
	char count;
	} Pack;

typedef union 
	{
	short data;
	Pack pack;
	} Data_pack;

static SHORT line_op_count;			/* # of packets */

static short *save_copy(short *cpt, int skips, short *data, int count)
/* add a copy-ops packet to compression stream */
{
#define p ((Data_pack *)cpt)
int lct;

if (count <= MAX_COPY)	/* do small block directly */
	{
	++line_op_count;
	p->pack.skip = (skips<<1);	/* skips is # of bytes to skip */
	p->pack.count = count;		/* but count is # of words to copy */
	cpt += 1;
	pj_copy_words(data,cpt,count);
	return(cpt+count);
	}
while (count > 0)	   /* chop up long blocks into pieces */
	{
	++line_op_count;
	if (count > MAX_COPY)
		lct = MAX_COPY;
	else
		lct = count;
	p->pack.skip = (skips<<1);
	p->pack.count = lct;
	cpt += 1;
	pj_copy_words(data,cpt,lct);
	cpt += lct;
	data += lct;
	count -= lct;
	skips = 0;
	}
return(cpt);
#undef p
}

static short *save_run(short *cpt, int skips, short data, int count)
/* add a run-ops packet to compression stream */
{
#define p ((Data_pack *)cpt)
int lct;

if (count <= MAX_RUN)	/* do small runs directly */
	{
	++line_op_count;
	p->pack.skip = (skips<<1);
	p->pack.count = -count;
	cpt[1] = data;
	return(cpt+2);
	}
while (count > 0)	   /* chop up long runs into pieces */
	{
	++line_op_count;
	if (count > MAX_RUN)
		lct = MAX_RUN;
	else
		lct = count;
	p->pack.skip = (skips<<1);
	p->pack.count = -lct;
	cpt[1] = data;
	cpt += 2;
	count -= lct;
	skips = 0;
	}
return(cpt);
#undef p
}


static SHORT ss2_line(
	SHORT *old, 		/* buffer for previous frame */
	SHORT *new, 		/* buffer for current frame */
	SHORT **pcomp,  	/* pointer to compression buffer, returns 
						 * next empty position of compression buffer */
	int word_count, 			/* number of words in line */
	int skipped_already) 		/* number of words that initially match */
{
SHORT *comp = *pcomp+1;		/* open position of compresion buf */
SHORT *end = new + word_count;	/* keep track of when to stop */
SHORT n, o;					/* new[0] and old[0] most of the time */
SHORT lastn;				/* new[-1] most of the time */
SHORT init_skips; 			/* # of skip words in finished op */
SHORT skip_count;			/* current length of skip */
SHORT run_count;			/* current length of run */
SHORT copy_count;			/* current length of copy */

line_op_count = 0;
while (skipped_already > MAX_SKIPS)
	{
	comp = save_copy(comp, MAX_SKIPS, new+MAX_SKIPS, 1);
	new += MAX_SKIPS+1;
	old += MAX_SKIPS+1;
	skipped_already -= (MAX_SKIPS+1);
	}
skip_count = init_skips = skipped_already; /* Initialize skip from parameter */
new = new + skipped_already - 1;	/* so loop below can be predecrement */
old = old + skipped_already - 1;	/* so loop below can be predecrement */
	{
	SKIPPING:
		/* This is the initial state and the state during long skips */
		{
		if (++new >= end)
			goto FINISH;
		if (*(++old) == (n = *new))
			{
			++skip_count;
			goto SKIPPING;
			}
		}
	DIF1:	/* This state is the first word after termination of
				 * a skip */
		{
		if (++new >= end)
			{
			while ((init_skips = skip_count) > MAX_SKIPS)
				{
				comp = save_copy(comp, MAX_SKIPS, 
					new-skip_count+MAX_SKIPS-1, 1);
				skip_count -= (MAX_SKIPS+1);
				}
			comp = save_copy(comp, skip_count, new-1, 1);
			goto FINISH;
			}
		while ((init_skips = skip_count) > MAX_SKIPS)
			{
			comp = save_copy(comp, MAX_SKIPS, 
				new-skip_count+MAX_SKIPS-1, 1);
			skip_count -= (MAX_SKIPS+1);
			}
		skip_count = 0;
		lastn = n;
			/* Have to break up really long skips here */
		if (*(++old) == (n = *new))
			skip_count = 1;		/* one word different, next the same */
		if (n == lastn)
			goto RUN2;
		else
			goto COPY2;
		}
	COPY2: 	/* State if have 2 differing words */
		{
		if (++new >= end)
			{
			comp = save_copy(comp, init_skips, new-2, 2-skip_count);
			goto FINISH;
			}
		lastn = n;
		if (*(++old) == (n = *new))
			{
			if (++skip_count >= COPYBREAK)	/* look for embedded skips */
				{
				comp = save_copy(comp, init_skips, new-2, 1);
				/* Write out accumulated copy minus current skips */
				goto SKIPPING;
				}
			}
		else
			skip_count = 0;
		if (n == lastn)
			run_count = 2;
		else
			run_count = 1;
		copy_count = 3;
		}
	COPYING:		/* Here have a copy 3 long or more */
		{
		if (++new >= end)
			{
			comp = save_copy(comp, init_skips, new-copy_count, 
				copy_count-skip_count);
			goto FINISH;
			}
		lastn = n;
		++copy_count;
		if (*(++old) == (n = *new))
			{
			if (++skip_count >= COPYBREAK)	/* look for embedded skips */
				{
				comp = save_copy(comp, init_skips, new+1-copy_count, 
					copy_count-skip_count);
				/* Write out accumulated copy minus current skips */
				goto SKIPPING;
				}
			}
		else
			skip_count = 0;
		if (n == lastn)
			{
			if (++run_count >= RUN_IN_COPY_BREAK)
				{
				comp = save_copy(comp, init_skips, new+1-copy_count, 
					copy_count-run_count);
				init_skips = 0;
				goto RUNNING;
				}
			}
		else
			{
			run_count = 1;
			}
		goto COPYING;
		}
	RUN2:	/* State if have 2 words that are the same */
		{
		if (++new >= end)
			{
			comp = save_run(comp, init_skips, lastn, 2-skip_count);
			goto FINISH;
			}
		lastn = n;
		if (*(++old) == (n = *new))
			++skip_count;
		else
			skip_count = 0;
		if (n == lastn)
			{
			run_count = 3;
			goto RUNNING;
			}
		else if (skip_count > 0)
			{
			if (skip_count == 2)
				{
				comp = save_copy(comp, init_skips, new-2, 1);
				goto SKIPPING;
				}
			else if (skip_count == 1)
				{
				comp = save_run(comp, init_skips, lastn, 2);
				goto SKIPPING;
				}
			}
		else
			{
			run_count = 1;
			copy_count = 3;
			goto COPYING;
			}
		}
	RUNNING:	/* Here have a run at least 3 long */
		{
		if (++new >= end)
			{
			comp = save_run(comp, init_skips, lastn, run_count-skip_count);
			goto FINISH;
			}
		lastn = n;
		n = *new;
		o = *(++old);
		if (n == lastn)
			++run_count;
		else	/* Break run */
			{
			comp = save_run(comp, init_skips,
				lastn, run_count - skip_count);
			/* Write out run minus the skip part to the output */
			if (o == n)	/* If run-breaking byte part continues skip... */
				{
				++skip_count;
				goto SKIPPING;
				}
			else
				{
				goto DIF1;
				}
			}
		if (o == n)
			{
			if (++skip_count >= RUNBREAK)
			/* Test to see if have a worthwhile skip in the middle
			 * of the run, and maybe break out of it. */
				{
				comp = save_run(comp, init_skips,
					lastn, run_count - skip_count);
				/* Write out run minus the skip part to the output */
				goto SKIPPING;
				/* And pick up skip where we are. */
				}
			}
		else
			skip_count = 0;
		goto RUNNING;
		}
	}
FINISH:
**pcomp = line_op_count;
*pcomp = comp;
return(line_op_count);
}

void *pj_ss2_rects(Raster *r1, void *cbuf, 
				   SHORT x1, SHORT y1, 
				   Raster *r2, 
				   SHORT x2, SHORT y2, USHORT width, USHORT height)
{
int dpr = (width>>2);	/* doubles per row */
int bafter_doubles = (width & 0x0003); /* bytes after doubles */
int wpr = (width>>1);	/* words per row */
int row;
int gotline;
int lskip;
int wskip, bskip;
long op_count = 0;
SHORT *cpt = OPTR(cbuf,2);
UBYTE *line1;
UBYTE *line2;
SHORT bpr1;
SHORT bpr2;
ULONG bytes_left;
int did_last = 0;

	bytes_left = width * height;

	/* if we can get'um directly do it fast way */
	if(r1->type == RT_BYTEMAP) 
	{
		bpr1 = ((Bytemap *)r1)->bm.bpr;
		line1 = ((Bytemap *)r1)->bm.bp[0] + (((y1 - 1) * bpr1) + x1);
	}
	else
	{
		bpr1 = 0;
		line1 = FOPTR(cbuf,bytes_left);
	}
	if(r2->type == RT_BYTEMAP) /* if we can get'um directly do it fast way */
	{
		bpr2 = ((Bytemap *)r2)->bm.bpr;
		line2 = ((Bytemap *)r2)->bm.bp[0] + (((y2 - 1) * bpr2) + x2);
	}
	else
	{
		bpr2 = 0;
		line2 = FOPTR(cbuf,bytes_left+width);
	}

	bytes_left -= bytes_left/16;

	row = 0;
	lskip = 0;
	while(row < height)
	{
		if(SIZE(cbuf,cpt) > bytes_left)
			return(NULL);

		gotline = 0;
		while (row < height)
		{
			++row;
			if(bpr1)
				line1 += bpr1;
			else
				pj_get_hseg(r1,line1,x1,y1++,width);

			if(bpr2)
				line2 += bpr2;
			else
				pj_get_hseg(r2,line2,x2,y2++,width);

			if((wskip = pj_dcompare(line1, line2, dpr)) != dpr)
			{
				wskip <<= 1;
				if(*(((SHORT *)line1) + wskip) == *(((SHORT *)line2) + wskip))
					++wskip; 
				gotline = 1;
				break;
			}
			if(bafter_doubles) /* a few bytes left !! */
			{
				bskip = dpr<<2;
				if((bskip = pj_bcompare(line1 + bskip,line2 + bskip,
							         bafter_doubles)) != bafter_doubles)
				{
					wskip <<= 1;
					wskip += bskip>>1;
					gotline = 1;
					break;
				}
			}
			++lskip;
		}
		if (gotline)
		{
			if (lskip > 0)
				*cpt++ = -lskip;

			if(width & 1) /* if odd width see if last pixel has changed */
			{  
				/* note: if a last pixel is written gotline == 0 */

				--width;
				if(line1[width] != line2[width])
				{
					*cpt++ = (SHORT)((USHORT)0x8000 | (USHORT)(line2[width]));
					did_last = 1;
				}
				else
					did_last = 0;

				++width;
			}
			switch(width)
			{
				case 3:
				case 2:
					if(*((SHORT *)line1) == *((SHORT *)line2))
						goto empty_line;

					/* write one word !! */

					*cpt++ = 1; /* one op */
					((BYTE *)cpt)[0] = 0;	/* skip 0 */
					((BYTE *)cpt)[1] = 1;	/* copy 1 */
					++cpt;
					*cpt++ = *((SHORT *)line2); /* one to copy */
					break;
				default: /* bigger than 3 bytes */
				{
					if(ss2_line((SHORT *)line1, (SHORT *)line2, 
								&cpt, wpr, wskip))
					{
						break;
					}
					--cpt; /* back off ops and fall through to empty line */ 
				}
				case 1:
				empty_line:
					if(!did_last)
					{
						if(lskip > 0)
							--cpt; /* cancel skip and continue adding */
						++lskip;   /* skip this */
						continue;  /* continue loop */
					}
					else
						*cpt++ = 0; /* no ops */

					break;
			}
			++op_count;
			lskip = 0;
		}
	}
	if (op_count == 0)
		return(OPTR(cbuf,(EMPTY_DCOMP-sizeof(Chunk_id)))); 
	else
	{
		*((SHORT *)cbuf) = op_count;
		return(cpt);
	}
}


