#include "fli.h"
#include "flicomp.h"
#include "rastcomp.h"


static void shift_copy_cmap(UBYTE *src, UBYTE *dst, int size)
{
UBYTE *max_dst = dst + size;

	while(dst < max_dst)
		*dst++ = *src++ >> 2; 
}
static char *full_cmap(Rgb3 *ctab,void *cbuf,int num_colors)
{
register UBYTE *bbuf = cbuf;

	*bbuf++ = 1;
	*bbuf++ = 0;
	*bbuf++ = 0;
	*bbuf++ = 0;
	shift_copy_cmap((UBYTE *)ctab,(UBYTE *)bbuf,COLORS*3);
	return(norm_pointer(bbuf+COLORS*3));
}
static void *comp_cmap(Rgb3 *last_ctab, Rgb3 *this_ctab, 
					   void *cbuf,int num_colors)
{
UBYTE *last, *this;
extern void *pj_fccomp();

	last = ((UBYTE*)cbuf) + (COLORS*6);
	this = last + (COLORS*6);
	shift_copy_cmap((UBYTE *)last_ctab,last,COLORS*3);
	shift_copy_cmap((UBYTE *)this_ctab,this,COLORS*3);
	return(pj_fccomp(last,this,cbuf,COLORS));
}
static LONG make_no_pstamp(Rcel *screen, void *cbuf,
					 	   SHORT x,SHORT y,USHORT width,USHORT height)
{
	return(0);
}

static struct complib aa_complib = {
	FLI_BRUN,
	FLI_COLOR,
	pj_brun_rect,
	full_cmap,
	make_no_pstamp,
	FLI_LC,
	FLI_COLOR,
	pj_lccomp_rects,
	comp_cmap,
};

const Flicomp pj_fli_comp_aa = &aa_complib;


