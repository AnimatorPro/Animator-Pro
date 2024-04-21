#include <string.h>
#include "ptrmacro.h"
#include "rastlib.h"
#include "memory.h"
#define WNDO_INTERNALS
#include "wndo.h"

/**************************************************************/

static void wr1_put_dot(Raster *wndo, Pixel c, Coor x, Coor y)
{
	Wndo *w = (Wndo *)wndo;
	CPUT_DOT(w->rasts[w->onerast],c,x,y);
}

static void _wr1_put_dot(Raster *wndo, Pixel c, Coor x, Coor y)
{
	Wndo *w = (Wndo *)wndo;
	PUT_DOT(w->rasts[w->onerast],c,x,y);
}

/**** the offset calls are this way ****/

static void wr1os_put_dot(Raster *wndo, Pixel c, Coor x, Coor y)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	CPUT_DOT(r,c,x + w->behind.x - r->x,y + w->behind.y - r->y);
}

static void _wr1os_put_dot(Raster *wndo, Pixel c, Coor x, Coor y)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	PUT_DOT(r,c,x + w->behind.x - r->x,y + w->behind.y - r->y);
}

static Pixel wr1_get_dot(Raster *wndo, Coor x, Coor y)
{
	Wndo *w = (Wndo *)wndo;
	return CGET_DOT(w->rasts[w->onerast], x, y);
}

static Pixel _wr1_get_dot(Raster *wndo, Coor x, Coor y)
{
	Wndo *w = (Wndo *)wndo;
	return GET_DOT(w->rasts[w->onerast], x, y);
}

static Pixel wr1os_get_dot(Raster *wndo, Coor x, Coor y)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	return CGET_DOT(r, x + w->behind.x - r->x, y + w->behind.y - r->y);
}

static Pixel _wr1os_get_dot(Raster *wndo, Coor x, Coor y)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	return GET_DOT(r, x + w->behind.x - r->x, y + w->behind.y - r->y);
}

static void
_wr1_put_hseg(Raster *wndo, Pixel *pbuf, Coor x, Coor y, Ucoor width)
{
	Wndo *w = (Wndo *)wndo;
	PUT_HSEG(w->rasts[w->onerast],pbuf,x,y,width);
}
static void
_wr1os_put_hseg(Raster *wndo, Pixel *pbuf, Coor x, Coor y, Ucoor width)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	PUT_HSEG(r,pbuf,x + w->behind.x - r->x,y + w->behind.y - r->y,width);
}
static void
_wr1_get_hseg(Raster *wndo, Pixel *pbuf, Coor x, Coor y, Ucoor width)
{
	Wndo *w = (Wndo *)wndo;
	GET_HSEG(w->rasts[w->onerast],pbuf,x,y,width);
}
static void
_wr1os_get_hseg(Raster *wndo, Pixel *pbuf, Coor x, Coor y, Ucoor width)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	GET_HSEG(r,pbuf,x + w->behind.x - r->x,y + w->behind.y - r->y,width);
}
static void
_wr1_put_vseg(Raster *wndo, Pixel *pbuf, Coor x, Coor y, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	PUT_VSEG(w->rasts[w->onerast],pbuf,x,y,height);
}
static void
_wr1os_put_vseg(Raster *wndo, Pixel *pbuf, Coor x, Coor y, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	PUT_VSEG(r,pbuf,x + w->behind.x - r->x,y + w->behind.y - r->y,height);
}
static void
_wr1_get_vseg(Raster *wndo, Pixel *pbuf, Coor x, Coor y, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	GET_VSEG(w->rasts[w->onerast],pbuf,x,y,height);
}
static void
_wr1os_get_vseg(Raster *wndo, Pixel *pbuf, Coor x, Coor y, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	GET_VSEG(r,pbuf,x + w->behind.x - r->x,y + w->behind.y - r->y,height);
}
static void
_wr1os_set_hline(Raster *wndo, Pixel color, Coor x, Coor y, Ucoor width)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	SET_HLINE(r,color,x + w->behind.x - r->x,y + w->behind.y - r->y,width);
}
static void
_wr1os_set_vline(Raster *wndo, Pixel color, Coor x, Coor y, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];
	SET_VLINE(r,color,x + w->behind.x - r->x,y + w->behind.y - r->y,height);
}
static void
_wr1os_put_rectpix(Raster *wndo, Pixel *pixbuf,
		Coor x, Coor y, Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

  	x += w->behind.x - r->x;
	y += w->behind.y - r->y;

	while(height-- > 0)
	{
		PUT_HSEG(r,pixbuf,x,y++,width);
		pixbuf = OPTR(pixbuf,width);
	}
}
static void
_wr1os_get_rectpix(Raster *wndo, Pixel *pixbuf,
		Coor x, Coor y, Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

  	x += w->behind.x - r->x;
	y += w->behind.y - r->y;

	while(height-- > 0)
	{
		GET_HSEG(r,pixbuf,x,y++,width);
		pixbuf = OPTR(pixbuf,width);
	}
}
static void
_wr1os_set_rect(Raster *wndo, Pixel color,
		Coor x, Coor y, Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	SET_RECT(r,color,
			  x + w->behind.x - r->x,y + w->behind.y - r->y,
			  width,height);
}
static void
_wr1os_xor_rect(Raster *wndo, Pixel color,
		Coor x, Coor y, Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	XOR_RECT(r,color,
			  x + w->behind.x - r->x,y + w->behind.y - r->y,
			  width,height);
}
static void
_wr1os_mask1blit(UBYTE *mbytes, unsigned int mbpr, Coor mx, Coor my,
		Raster *wndo, Coor x, Coor y, Ucoor width, Ucoor height,
		Pixel oncolor)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	MASK1BLIT(mbytes, mbpr,mx,my, r,
			  x + w->behind.x - r->x,y + w->behind.y - r->y,
			  width,height,oncolor);
}
static void
_wr1os_mask2blit(UBYTE *mbytes, unsigned int mbpr, Coor mx, Coor my,
		Raster *wndo, Coor x, Coor y, Ucoor width, Ucoor height,
		Pixel oncolor, Pixel offcolor)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	MASK2BLIT(mbytes, mbpr,mx,my, r,
			  x + w->behind.x - r->x,y + w->behind.y - r->y,
			  width,height,oncolor,offcolor);
}

/* decompressors */

static void
wr1os_unbrun_rect(Raster *wndo, void *ucbuf, LONG pixsize,
		Coor x, Coor y, Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	(r->lib->unbrun_rect)(r,ucbuf, pixsize,
			  		x + w->behind.x - r->x,y + w->behind.y - r->y,
					width,height);
}
static void
wr1os_unlccomp_rect(Raster *wndo, void *ucbuf, LONG pixsize,
		Coor x, Coor y, Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	(r->lib->unlccomp_rect)(r,ucbuf, pixsize,
			  		x + w->behind.x - r->x,y + w->behind.y - r->y,
					width,height);
}
static Errcode
wr1os_unss2_rect(Raster *wndo, void *ucbuf, LONG pixsize,
		Coor x, Coor y, Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	return (r->lib->unss2_rect)(r, ucbuf, pixsize,
			  		x + w->behind.x - r->x,y + w->behind.y - r->y,
					width,height);
}

/***** binary calls *******/

static Errcode
wr1os_blitrect(Raster *wndo, Coor wx, Coor wy,
		Raster *dest, Coor dest_x, Coor dest_y,
		Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	return pj__blitrect(r, wx + w->behind.x - r->x, wy + w->behind.y - r->y,
			 dest, dest_x, dest_y,width, height);
}
static Errcode
wr1os_f_blitrect(Raster *source, Coor src_x, Coor src_y,
		Raster *wndo, Coor wx, Coor wy,
		Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	return (r->lib->blitrect[RL_FROM_BYTEMAP])(source, src_x, src_y,
			 r, wx + w->behind.x - r->x,wy + w->behind.y - r->y,
			 width, height);
}

static void
wr1os_swaprect(Raster *wndo, Coor wx, Coor wy,
		Raster *rastb, Coor rastb_x, Coor rastb_y,
		Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	pj__swaprect(r, wx + w->behind.x - r->x,wy + w->behind.y - r->y,
			  rastb, rastb_x, rastb_y, width, height);
}
static void
wr1os_f_swaprect(Raster *rasta, Coor rasta_x, Coor rasta_y,
		Raster *wndo, Coor wx, Coor wy,
		Ucoor width, Ucoor height)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	(r->lib->swaprect[RL_FROM_BYTEMAP])(rasta, rasta_x, rasta_y,
			 r, wx + w->behind.x - r->x,wy + w->behind.y - r->y,
			 width, height);
}

static Errcode
wr1os_tblitrect(Raster *wndo, Coor wx, Coor wy,
		Raster *dest, Coor dest_x, Coor dest_y,
		Ucoor width, Ucoor height, Pixel tcolor)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	return pj__tblitrect(r, wx + w->behind.x - r->x, wy + w->behind.y - r->y,
			  dest, dest_x, dest_y,width, height, tcolor);
}
static Errcode
wr1os_f_tblitrect(Raster *source, Coor src_x, Coor src_y,
		Raster *wndo, Coor wx, Coor wy,
		Ucoor width, Ucoor height, Pixel tcolor)
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	return (r->lib->tblitrect[RL_FROM_BYTEMAP])(source, src_x, src_y,
			  r, wx + w->behind.x - r->x,wy + w->behind.y - r->y,
			  width, height, tcolor);
}

static Errcode
wr1os_zoomblit(Raster *wndo, Coor sx, Coor sy,
		Raster *dst, Coor dx, Coor dy,
		Ucoor dw, Ucoor dh, LONG zxs, LONG zys)
/* this type is to handle the FROM_BYTEMAP case since the lib will be
 * called from the destination raster *****/
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	return pj_zoomblit(r, sx + w->behind.x - r->x,
			   sy + w->behind.y - r->y, dst, dx, dy, dw, dh, zxs, zys);
}
static Errcode
wr1os_f_zoomblit(Raster *src, Coor sx, Coor sy,
		Raster *wndo, Coor dx, Coor dy,
		Ucoor dw, Ucoor dh, LONG zxs, LONG zys)
/* this type is to handle the FROM_BYTEMAP case since the lib will be
 * called from the destination raster *****/
{
	Wndo *w = (Wndo *)wndo;
	Raster *r = w->rasts[w->onerast];

	return (r->lib->zoomblit[RL_FROM_BYTEMAP])
				(src,sx,sy,r,dx + w->behind.x - r->x,
				 dy + w->behind.y - r->y, dw, dh, zxs, zys);
}



Rastlib *get_wndo_r1oslib(void)
{
static int loaded = 0;
static Rastlib oslib;

	if(!loaded)
	{
		loaded = 1;
		copy_mem(get_window_lib(),&oslib,sizeof(oslib));

		oslib.cput_dot = wr1os_put_dot;
		oslib.put_dot = _wr1os_put_dot;
		oslib.cget_dot = wr1os_get_dot;
		oslib.get_dot = _wr1os_get_dot;

		oslib.put_hseg = _wr1os_put_hseg;
		oslib.get_hseg = _wr1os_get_hseg;
		oslib.put_vseg = _wr1os_put_vseg;
		oslib.get_vseg = _wr1os_get_vseg;

		oslib.put_rectpix = _wr1os_put_rectpix;
		oslib.get_rectpix = _wr1os_get_rectpix;

		oslib.set_hline = _wr1os_set_hline;
		oslib.set_vline = _wr1os_set_vline;
		oslib.set_rect = _wr1os_set_rect;

		/* oslib.set_rast = NULL; let be funneled through set rect
		 * 'cause window is a rectangle */

		oslib.xor_rect = _wr1os_xor_rect;

		oslib.unbrun_rect = wr1os_unbrun_rect;
		oslib.unlccomp_rect = wr1os_unlccomp_rect;
		oslib.unss2_rect = wr1os_unss2_rect;

		oslib.mask1blit = _wr1os_mask1blit;
		oslib.mask2blit = _wr1os_mask2blit;

		/* binary calls */

		oslib.blitrect[RL_TO_SAME] = wr1os_blitrect;
		oslib.blitrect[RL_TO_BYTEMAP] = wr1os_blitrect;
		oslib.blitrect[RL_FROM_BYTEMAP] = wr1os_f_blitrect;
		oslib.blitrect[RL_TO_OTHER] = wr1os_blitrect;

		oslib.swaprect[RL_TO_SAME] = wr1os_swaprect;
		oslib.swaprect[RL_TO_BYTEMAP] = wr1os_swaprect;
		oslib.swaprect[RL_FROM_BYTEMAP] = wr1os_f_swaprect;
		oslib.swaprect[RL_TO_OTHER] = wr1os_swaprect;

		oslib.tblitrect[RL_TO_SAME] = wr1os_tblitrect;
		oslib.tblitrect[RL_TO_BYTEMAP] = wr1os_tblitrect;
		oslib.tblitrect[RL_FROM_BYTEMAP] = wr1os_f_tblitrect;
		oslib.tblitrect[RL_TO_OTHER] = wr1os_tblitrect;

#ifdef  NOTYET
		oslib.xor_rast[RL_TO_SAME] = wr1os_xor_rast;
		oslib.xor_rast[RL_TO_BYTEMAP] = wr1os_xor_rast;
		oslib.xor_rast[RL_FROM_BYTEMAP] = wr1os_f_xor_rast;
		oslib.xor_rast[RL_TO_OTHER] = wr1os_xor_rast;
#endif /* NOTYET */

		oslib.zoomblit[RL_TO_SAME] = wr1os_zoomblit;
		oslib.zoomblit[RL_TO_BYTEMAP] = wr1os_zoomblit;
		oslib.zoomblit[RL_FROM_BYTEMAP] = wr1os_f_zoomblit;
		oslib.zoomblit[RL_TO_OTHER] = wr1os_zoomblit;
	}
	return(&oslib);
}

Rastlib *get_wndo_r1lib(void)
{
static int loaded = 0;
static Rastlib r1lib;

	if(!loaded)
	{
		loaded = 1;
		copy_mem(get_wndo_r1oslib(),&r1lib,sizeof(r1lib));
		r1lib.cput_dot = wr1_put_dot;
		r1lib.put_dot = _wr1_put_dot;
		r1lib.cget_dot = wr1_get_dot;
		r1lib.get_dot = _wr1_get_dot;

		r1lib.put_hseg = _wr1_put_hseg;
		r1lib.get_hseg = _wr1_get_hseg;
		r1lib.put_vseg = _wr1_put_vseg;
		r1lib.get_vseg = _wr1_get_vseg;
		pj_set_grc_calls(&r1lib);
	}
	return(&r1lib);
}

