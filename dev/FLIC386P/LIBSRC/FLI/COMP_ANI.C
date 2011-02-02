#include "memory.h"
#include "fli.h"
#include "flicomp.h"
#include "rastcomp.h"

static char *full_cmap(Rgb3 *ctab,void *cbuf,int num_colors)
{
register UBYTE *bbuf = cbuf;

	*bbuf++ = 1;
	*bbuf++ = 0;
	*bbuf++ = 0;
	*bbuf++ = 0;
	pj_copy_bytes(ctab, bbuf, COLORS*3);
	return(norm_pointer(bbuf+COLORS*3));
}

extern LONG pj_build_rect_pstamp(Rcel *screen, void *cbuf,
					 	         SHORT x,SHORT y,USHORT width,USHORT height);

extern void *pj_fccomp();

static struct complib ani_complib = {
	FLI_BRUN,
	FLI_COLOR256,
	pj_brun_rect,
	full_cmap,
	pj_build_rect_pstamp,
	FLI_SS2,
	FLI_COLOR256,
	pj_ss2_rects,
	pj_fccomp,
};

const Flicomp pj_fli_comp_ani = &ani_complib;


