#define GFX_INTERNALS
#define RASTGFX_INTERNALS
#include "ptrmacro.h"
#include "memory.h"
#include "gfx.h"

#include <string.h>


typedef struct sblit_dat {
	Raster *src;
	Raster *dst;
	Coor sx, dx; 
	Ucoor sw, dw;
	int last_sy;
	Pixel *sbuf;
	Pixel *dbuf;
	SHORT *xtable;
	SHORT mode;
	Pixel *xlat;
	Pixel tcolor;
} Sblit_dat;

#define SBD_TCXL 1

static void line_inc(int sy, int dy, Sblit_dat *sbd)
{
    Pixel *spix;
    Pixel *dpix;
    Pixel *maxpix;
    SHORT *xtable;
    Pixel p;

	spix = sbd->sbuf;
	dpix = sbd->dbuf;
	if(sbd->last_sy != sy)
	{
		sbd->last_sy = sy;
		pj_get_hseg(sbd->src,spix,sbd->sx,sy,sbd->sw);

		xtable = sbd->xtable;
		maxpix = dpix + sbd->dw;

		if(sbd->mode == SBD_TCXL)
		{
		Pixel *xlat = sbd->xlat;

			pj_get_hseg(sbd->dst,sbd->dbuf,sbd->dx,dy,sbd->dw);
			while(dpix < maxpix)
			{
				if((p = spix[*xtable++]) != sbd->tcolor)
					*dpix = xlat[p];
				++dpix;
			}
		}
		else
		{
			while(dpix < maxpix)
				*dpix++ = spix[*xtable++];
		}
	}
	pj_put_hseg(sbd->dst,sbd->dbuf,sbd->dx,dy,sbd->dw);
}


/* This was pulled from linscale.c.
 *
 * Weird thing: before, it was passing void* around and casting to Sblit_dat
 * inside the different functions. line_inc() was passed as a function pointer.
 * Somewhere in all that, on Apple Silicon it corrupts the ->src pointer of the
 * Sblit_dat by setting it to 0x80004, every time.  Making a custom function here
 * for this particular use case and calling line_inc directly without void
 * pointers seems to fix the issue.
 * */
static void pj_do_linscale_sbd(int sx, int sw, int dx, int dw, Sblit_dat* sbd)
{
	int xerr;
	int dmax;

	xerr = dw - (sw>>1);
	dmax = dx + dw;

	for(;dx < dmax;++dx)
	{
		line_inc(sx,dx,sbd);
		if((xerr -= sw) <= 0)
		{
			for(;;)
			{
				++sx;
				if((xerr += dw) > 0)
					break;
			}
		}
	}
}


void pj_scale_blit(Raster *src, Coor sx, Coor sy, Ucoor sw, Ucoor sh,
           		   Raster *dst, Coor dx, Coor dy, Ucoor dw, Ucoor dh,
				   Tcolxldat *tcxl)
{
	Sblit_dat sbd;
	memset(&sbd, 0, sizeof(Sblit_dat));
	Pixel sbuf[SBUF_SIZE];
	unsigned int bsize;

	bsize = (sw+dw)*sizeof(Pixel)+dw*sizeof(SHORT);

	if(bsize <= sizeof(sbuf)) {
		sbd.sbuf = sbuf;
	}
	else if ((sbd.sbuf = (Pixel *)pj_malloc(bsize)) == NULL) {
		return;
	}
	sbd.dbuf = sbd.sbuf + sw;
	sbd.xtable = (SHORT *)(sbd.dbuf + dw);

	sbd.dx = dx;
	sbd.sx = sx;
	sbd.sw = sw;
	sbd.dw = dw;
	sbd.last_sy = sy-1;
	sbd.src = src;
	sbd.dst = dst;
	sbd.mode = 0;

	if(tcxl != NULL)
	{
		sbd.mode = SBD_TCXL;
		sbd.tcolor = tcxl->tcolor;
		sbd.xlat = tcxl->xlat;
	}
	pj_make_scale_table(sw,dw,sbd.xtable);

	pj_do_linscale_sbd(sy,sh,dy,dh,&sbd);

	if(sbd.sbuf != sbuf)
		pj_free(sbd.sbuf);
}
