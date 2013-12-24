#define RASTCOMP_INTERNALS
#include "ptrmacro.h"
#include "rastcomp.h"



/* structure for cline called by decompressed line maker */

typedef struct brun_dat {
	Raster *drast;
	BYTE *src;     
	LONG bpr;
	Pixel *dline;   /* destination line buffer or plane */
	SHORT *xtable; /* x scaling table */
	Coor dx;
	Ucoor dw, sw;
	SHORT last_sy;
} Brundat;


static void scale_ubrun_line(int sy, int dy, Brundat *brd)
{
	if(brd->last_sy != sy)
	{
		while(++brd->last_sy < sy)
			brd->src = pj_unbrun_skip_line(brd->src, brd->sw);

		brd->src = pj_unbrun_scale_line(brd->src,brd->sw,
									    (BYTE *)(brd->dline),brd->xtable);
	}
	pj__put_hseg(brd->drast,brd->dline,brd->dx,dy,brd->dw);
}
static void to_bym_scale_ubrun_line(int sy, int dy, Brundat *brd)

{
	if(brd->last_sy != sy)
	{
		while(++brd->last_sy < sy)
			brd->src = pj_unbrun_skip_line(brd->src, brd->sw);

		brd->src = pj_unbrun_scale_line(brd->src,brd->sw,
									    (BYTE *)(brd->dline),brd->xtable);
	}
	else
	{
		pj_copy_bytes(brd->dline - brd->bpr,brd->dline,brd->dw);
	}
	brd->dline += brd->bpr;
}
void pj_unbrun_scale_rect(Raster *dst,void *ucbuf, USHORT sw, USHORT sh, 
						   SHORT dx, SHORT dy, USHORT dw, USHORT dh)

/* at the moment the maximum dest width is 340 that's it, source width 
 * is ok to 32000 */
{
char sbuf[SBUF_SIZE];
Brundat brd;
void (*vincfunc)(int sx,int dx,Brundat *brd);

	/* load up the data, we'll have a few bytes of fun. */

	brd.drast = dst;
	brd.src = ucbuf;
	brd.last_sy = -1;
	brd.dx = dx;
	brd.dw = dw;
	brd.sw = sw;

	if(dst->type == RT_BYTEMAP)
	{
		vincfunc = to_bym_scale_ubrun_line;
		brd.xtable = (SHORT *)(sbuf);
		brd.bpr = (((Bytemap *)dst)->bm.bpr);
		brd.dline = (((Bytemap *)dst)->bm.bp[0])+(dx+(brd.bpr*dy));
	}
	else
	{
		vincfunc = scale_ubrun_line;
		brd.dline = (Pixel *)sbuf;
		brd.xtable = (SHORT *)(brd.dline + dw);
	}


	pj_make_scale_table(sw,dw,brd.xtable);
	brd.xtable[dw] = -1; /* the end for pj_scale_ubrun_line() */
	/* de compress lines */

	pj_do_linscale(0,sh,dy,dh,vincfunc,&brd);
}
