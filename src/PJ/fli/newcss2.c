
/* ss2.c - Some C code that mixes with the assembler code in
   ss2line.asm to make up compressed pixel packets suitable
   for incorporation into a FLC file.  See also writefli.c

   07/05/91 (Ian)
	Rewritten to correspond to new module ss2line.asm.
*/

#define RASTCOMP_INTERNALS
#include "stdtypes.h"
#include "ptrmacro.h"
#include "rastcomp.h"
#include "fli.h"

extern void pj_ss2line_start(void *compbuf, int width);
extern void *pj_ss2line_finish(void);
extern void *pj_ss2line(void *oldline, void *newline);

void *pj_ss2_rects(Raster	*r1,
				   void 	*cbuf,
				   SHORT	x1,
				   SHORT	y1,
				   Raster	*r2,
				   SHORT	x2,
				   SHORT	y2,
				   USHORT	width,
				   USHORT	height)
/*****************************************************************************
 *
 ****************************************************************************/
{
	long	bytes_left; 	// used to prevent output buffer overflow and such.
	void	*end;			// ptr to end of cbuf, prevents buffer overflow.
	UBYTE	*line1;
	UBYTE	*line2;
	int 	bpr1;
	int 	bpr2;
	int 	linect = height;// need a signed version for loop control.

	// set up access to input rasters...

	bytes_left = ((width * linect) + 3) & ~3; // buf size w/pad for dword align

	if(r1->type == RT_BYTEMAP)
		{
		bpr1  = ((Bytemap *)r1)->bm.bpr;
		line1 = ((Bytemap *)r1)->bm.bp[0] + (((y1 - 1) * bpr1) + x1);
		}
	else
		{
		bpr1  = 0;
		line1 = FOPTR(cbuf,bytes_left);
		}

	if(r2->type == RT_BYTEMAP)
		{
		bpr2  = ((Bytemap *)r2)->bm.bpr;
		line2 = ((Bytemap *)r2)->bm.bp[0] + (((y2 - 1) * bpr2) + x2);
		}
	else
		{
		bpr2  = 0;
		line2 = FOPTR(cbuf,bytes_left+((width+3)&~3));
		}

	bytes_left -= bytes_left/16;	// for detect of lousy compression: when
	end = FOPTR(cbuf,bytes_left);	// deltas are almost the full raster size.

	// now build the compression packets for each line until all lines done...

	pj_ss2line_start(cbuf, width);

	while(--linect >= 0)
		{
		if(bpr1)
			line1 += bpr1;
		else
			pj_get_hseg(r1,line1,x1,y1++,width);

		if(bpr2)
			line2 += bpr2;
		else
			pj_get_hseg(r2,line2,x2,y2++,width);

		if (end < pj_ss2line(line1, line2))
			return NULL;		// oops, lousy compression, just punt.

		}

	end = pj_ss2line_finish();	// will return NULL if no deltas generated.

	if (end == NULL)
		return OPTR(cbuf,(EMPTY_DCOMP-sizeof(Chunk_id)));
	else
		return end;

}
