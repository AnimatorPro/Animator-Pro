#include "jimk.h"
#include "inkdot.h"
#include "inks.h"
#include "memory.h"
#include "options.h"
#include "rastlib.h"
#include "render.h"
#include "rexlib.h"

Thash *gel_thash;
SHORT gel_factor;

static UBYTE 
	render_with_mask, render_fast, render_real_fast, 
	render_write_mask, render_read_mask, render_hlineable;
static int ink_type;
static Pixel (*ink_dot)(const Ink *inky,const SHORT x,const SHORT y);

extern Button dithergroup_sel, dtintgroup_sel, tintgroup_sel;
extern int bclosest_col(Rgb3 *rgb, int count, SHORT dither);

Aa_ink_data ink_aid = {
	0, 0, NULL, NULL, NULL, NULL, 
	COLORS, RGB_MAX, RGB_MAX, RGB_MAX,
	true_blend,
	closestc,
	make_ink_bhash,
	free_ink_bhash,
	bclosest_col,
};

set_render_fast()
{
ink_type = vl.ink->ot.id;
ink_dot = vl.ink->dot;
render_read_mask = (vs.use_mask != 0 && mask_rast != NULL);
render_write_mask = (vs.make_mask != 0 && mask_rast != NULL);
render_with_mask = (render_read_mask || render_write_mask);
render_hlineable = !(render_read_mask || gel_factor);
render_fast = (!render_with_mask && !gel_factor);
render_real_fast = (render_fast && ink_type == opq_INKID);
if(thecel != NULL)
	ink_aid.cel = thecel->rc;
else
	ink_aid.cel = NULL;
ink_aid.alt = vl.alt_cel;
ink_aid.screen = vb.pencel;
ink_aid.ccolor = vs.ccolor;
ink_aid.tcolor = vs.inks[0];
ink_aid.undo = undof;
}



/*********** Start of opaque ink stuff (real easy!) ********************/
Pixel opq_dot(const Ink *inky, const SHORT x, const SHORT y)
{
return(vs.ccolor);
}

void opq_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
SET_HLINE(vb.pencel,vs.ccolor,x0,y,width);
}
/*********** Start of gradient oriented ink stuff **********************/
static Pixel dfrom_range(long p,long q,long x,long y, Boolean dither)
{
SHORT color;

if (q == 0)
	return(0);
p *= vs.buns[vs.use_bun].bun_count-1;
if (dither)
	{
	color = p/q;
	if ((long)((p)%(q)*256L/(q)) > ((((x)&(y))*83+((x)+(y))*63+(x)*37)&255))
		color++;
	}
else
	color = (p+q/2)/q;
color = vs.buns[vs.use_bun].bundle[color];
return(color);
}

Pixel vsp_dot(const Ink *inky, const SHORT x, const SHORT y)
{
return(dfrom_range(y-rdta.rdy0,rdta.rheight,x,y,inky->dither));
}

void vsp_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
UBYTE short_buf[SBSIZE];
UBYTE *spt;
long p,q;
long fac1, fac2, fac3;
UBYTE *bun;
SHORT i;
UBYTE coff;
UBYTE color;
SHORT x;

if ((q = rdta.rheight) == 0)
	{
	SET_HLINE(vb.pencel,0,x0,y,width);
	return;
	}
p = (y - rdta.rdy0)*(vs.buns[vs.use_bun].bun_count-1);
bun = vs.buns[vs.use_bun].bundle;
if (!inky->dither)
	{
	SET_HLINE(vb.pencel,bun[(p+q/2)/q],x0,y,width);
	}
else
	{
	x = x0;
	color = p/q;
	fac1 = p%q*256L/q; 
	fac2 = (x+y)*63;
	fac3 = x*37;
	spt  =  short_buf;
	i = width;
	while (--i>=0)
		{
		coff = (fac1 > (((x&y)*83+fac2+fac3)&255));
		fac2 += 63;
		fac3 += 37;
		*spt++ = bun[color+coff];
		x++;
		}
	PUT_HSEG(vb.pencel,short_buf,x0,y,width);
	}
return;
}

Pixel hsp_dot(const Ink *inky, const SHORT x,const SHORT y)
{
return(dfrom_range(x-rdta.rdx0,rdta.rwidth,x,y,inky->dither));
}

void hsp_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
UBYTE short_buf[SBSIZE];
UBYTE *spt;
long p,q;
long bc;
UBYTE *bun;
SHORT x;
SHORT i;
UBYTE colo;
long fac2, fac3;

if ((q = rdta.rwidth) == 0)
	{
	SET_HLINE(vb.pencel,0,x0,y,width);
	return;
	}
bc = vs.buns[vs.use_bun].bun_count-1;
p = (x0 - rdta.rdx0)*(bc);
bun = vs.buns[vs.use_bun].bundle;
spt = short_buf;
i = width;
if (inky->dither)
	{
	x = x0;
	fac2 = (x+y)*63;
	fac3 = x*37;
	while (--i >= 0)
		{
		colo = p/q;
		if ((SHORT)((p)%(q)*256L/(q)) > 
			((((x)&(y))*83+fac2+fac3)&255))
			colo++;
		*spt++ = bun[colo];
		fac2 += 63;
		fac3 += 37;
		x++;
		p += bc;
		}
	}
else
	{
	while (--i >= 0)
		{
		*spt++ = bun[(p+q/2)/q];
		p += bc;
		}
	}
PUT_HSEG(vb.pencel,short_buf,x0,y,width);
}

Pixel rad_dot(const Ink *inky, const SHORT x, const SHORT y)
/* This one is so slow don't bother to have a hline routine... */
{
	if(vl.rgr == 0) /* can't be zero */
		vl.rgr = 1;
	return(dfrom_range(calc_distance(x,y,vl.rgc.x,vl.rgc.y)%vl.rgr,
			vl.rgr,x,y,inky->dither));
}

/*********** Start of soften oriented ink stuff **********************/

static int get_neighbors(Pixel *npix, SHORT x, SHORT y)
/* Gets immediate neighbors from undo returns number of neighbors */
{
Coor width, height;

	if(x == rdta.cr.x)
	{
		width = 2; /* remove left column of pixels leave x where it is */
	}
	else
	{
		if(x == rdta.cr.MaxX - 1) /* last one */
			width = 2;
		else
			width = 3;
		--x; /* 3 x 3 rectangle starts one less than x */
	}

	if(y == rdta.cr.y)
	{
		height = 2; /* remove top row of pixels leave x where it is */
	}
	else
	{
		if(y == rdta.cr.MaxY - 1) /* last one */
			height = 2;
		else
			height = 3;
		--y; /* rectangle starts one less than y */
	}
	
	pj_get_rectpix(undof,npix,x,y,width,height);
	return(width*height);
}
static void average_colors(Rgb3 *aver, Pixel *pix, int count, Cmap *cmap)
{
Pixel *maxpix;
Rgb3 *ctab;
int r,g,b;
Rgb3 *pcolor;

	maxpix = pix + count;
	ctab = cmap->ctab;
	r = g = b = 0;
	while(pix < maxpix)
	{
		pcolor = ctab + *pix++;
		r += pcolor->r;
		g += pcolor->g;
		b += pcolor->b;
	}
	aver->r = r / count;
	aver->g = g / count;
	aver->b = b / count;
}


Pixel soft_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Rgb3 rgb;
Pixel neighbors[9];

	average_colors(&rgb, neighbors, 
				get_neighbors(neighbors,x,y),
				vb.pencel->cmap);
	return(bclosest_col(&rgb,COLORS,inky->dither));
}
void soft_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
LONG rgbs[(SBSIZE+3)*3];
LONG *prgb, *rgbstart;
Pixel line[SBSIZE+2];
Pixel *pline, *maxpline;
Rgb3 rgb, *ctab, *color;
int div;
Coor gety;
Coor getx;
Coor getw;


	ctab = vb.pencel->cmap->ctab;
	getx = x0;
	getw = width;
	div = 0;

	if(x0 > 0)
	{
		rgbstart = &rgbs[0];
		--getx;
		++getw;
	}
	else
		rgbstart = &rgbs[3];

	if(getx + getw < vb.pencel->width - 1)
		++getw;

	pj_stuff_dwords(0, rgbs, (getw+1) * 3);
	maxpline = line + getw;

	gety = y - 2;
	if(y <= 0)
		++gety;

	/* add rgbs from lines to array (vertical) */

	while(gety <= y)
	{
		++gety;
		if(gety >= vb.pencel->height)
			break;

		pj__get_hseg(undof, line, getx, gety, getw);

		prgb = rgbstart;
		pline = line;

		while(pline < maxpline)
		{
			color = ctab + *pline++;
			*prgb++ += color->r;
			*prgb++ += color->g;
			*prgb++ += color->b;
		}
		div += 3;
	}

	if(x0 == getx)
	{
		rgbs[0] = (rgbs[3] + rgbs[6])>>1;
		rgbs[1] = (rgbs[4] + rgbs[7])>>1;
		rgbs[2] = (rgbs[5] + rgbs[8])>>1;
		--getw;
	}

	if(getw == width)
	{
		prgb = rgbs + (width*3);
		prgb[3] = (prgb[-3] + prgb[0])>>1;
		prgb[4] = (prgb[-2] + prgb[1])>>1;
		prgb[5] = (prgb[-1] + prgb[2])>>1;
	}

	pline = line;
	maxpline = line + width;
	prgb = rgbs;

	while(pline < maxpline)
	{
		rgb.r = (prgb[0] + prgb[3] + prgb[6])/div;
		rgb.g = (prgb[1] + prgb[4] + prgb[7])/div;
		rgb.b = (prgb[2] + prgb[5] + prgb[8])/div;
		*pline++ = bclosest_col(&rgb,COLORS,inky->dither);
		prgb += 3;
	}
	pj__put_hseg(vb.pencel, line, x0, y, width);
}


Pixel anti_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Pixel neighbors[9];
Rgb3 rgb;
int count;

	if((count = get_neighbors(neighbors,x,y)) == 9)
	{
		if( (neighbors[4] == neighbors[3] && neighbors[4] == neighbors[5])
			|| (neighbors[4] == neighbors[1] && neighbors[4] == neighbors[7])
			|| (neighbors[4] == neighbors[0] && neighbors[4] == neighbors[8])
			|| (neighbors[4] == neighbors[2] && neighbors[4] == neighbors[6]))
		{
			return(neighbors[4]);
		}
	}
	else
	{
		/* for now dots on edge of port won't get hit */
		return(pj_get_dot(undof,x,y));
	}

	average_colors(&rgb, neighbors, count, vb.pencel->cmap);
	return(bclosest_col(&rgb,COLORS,inky->dither));
}

/*********** Start of transparent oriented ink stuff **********************/

static Pixel tvdot(const Ink *inky, const SHORT x, const SHORT y, 
					Rcel *source)
{
Thash *t = inky->inkdata;
SHORT underc = GET_DOT(source,x,y);
Rgb3 rgb;

if (inky->dither)
	{
	true_blend(vb.pencel->cmap->ctab+underc, 
		&tcolor_src, inky->strength, &rgb);
	return(bclosest_col(&rgb,COLORS, TRUE));
	}
t += underc;
if (!t->valid)
	{
	true_blend(vb.pencel->cmap->ctab+underc, 
		&tcolor_src, inky->strength, &rgb);
	t->closest = closestc(&rgb,vb.pencel->cmap->ctab,COLORS);
	t->valid = TRUE;
	}
return(t->closest);
}

Pixel tsp_dot(const Ink *inky, const SHORT x, const SHORT y)
{
return(tvdot(inky,x,y,undof));
}


void tsp_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
/* This one has basically an in-line tclosest_color for speed. */
{
UBYTE sbuf[SBSIZE];
UBYTE dbuf[SBSIZE];
UBYTE *spt, *dpt;
SHORT i;
Thash *t;
Rgb3 rgb;
Rgb3 *ctab;

pj_get_hseg(undof, sbuf, x0, y, width);
spt = sbuf;
dpt = dbuf;
i = width;
ctab = vb.pencel->cmap->ctab;
if (inky->dither)
	{
	while (--i >= 0)
		{
		true_blend(ctab + *spt++, 
			&tcolor_src, inky->strength, &rgb);
		*dpt++ = bclosest_col(&rgb, COLORS,TRUE);
		}
	}
else
	{
	while (--i >= 0)
		{
		t = inky->inkdata;
		t += *spt;
		if (!t->valid)
			{
			true_blend(ctab + *spt, 
				&tcolor_src, inky->strength, &rgb);
			t->closest = closestc(&rgb,ctab,COLORS);
			t->valid = TRUE;
			}
		spt++;
		*dpt++ = t->closest;
		}
	}
PUT_HSEG(vb.pencel, dbuf, x0, y, width);
}

Pixel tlc_dot(const Ink *inky, const SHORT x, const SHORT y)
{
return(tvdot(inky,x,y,vb.pencel));
}

/***************** gel transparency thing ***************/
static Pixel tccolor(const SHORT dcolor,const SHORT tcolor,
			const SHORT tintper,Thash *t)
{
Rgb3 rgb;
Rgb3 *cm;

t += dcolor;
if (!t->valid)
	{
	cm = vb.pencel->cmap->ctab;
	true_blend(cm+dcolor, cm+tcolor, tintper, &rgb);
	t->closest = closestc(&rgb,cm,COLORS);
	t->valid = 1;
	}
return(t->closest);
}
static Pixel gel_dot(SHORT x, SHORT y, Pixel color, SHORT dither)
{
Rgb3 rgb;

if (ink_type == opq_INKID)
	{
	return(tccolor(pj_get_dot(vb.pencel,x,y), color,
						gel_factor, gel_thash));
	}
else
	{
	true_blend(vb.pencel->cmap->ctab+pj_get_dot(vb.pencel,x,y),
				vb.pencel->cmap->ctab+color, gel_factor, &rgb);
	return(bclosest_col(&rgb, COLORS, dither));
	}
}
/************** render dot ************************/

void render_dot(SHORT x, SHORT y, void *data)
{
Pixel color;
(void)data;

if (x < rdta.cr.x  || y < rdta.cr.y 
	 || x >= rdta.cr.MaxX || y >= rdta.cr.MaxY )
	return;

if (render_with_mask)
	{
	if (vs.use_mask)
		{
		if((mask_rast->bm.bp[0])[(y*mask_rast->bm.bpr)+(x>>3)]&(0x80>>(x&7)))
				return;
		}
	if (vs.make_mask)
		(mask_rast->bm.bp[0])[y*mask_rast->bm.bpr+(x>>3)] |= (0x80>>(x&7));
	}

ink_aid.ccolor = vs.ccolor; /* must set because of potential color cycling */
color = (*ink_dot)(vl.ink,x,y);
if(gel_factor)
	color = gel_dot(x,y,color,vl.ink->dither);
PUT_DOT(vb.pencel,color,x,y);
if (vs.zoom_open)
	upd_zoom_dot(color,x,y);
}

/****** start of render hline things ************/

void gink_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
/* generic ink hline NOTE this is Un-clipped */
{
Pixel buf[SBSIZE];
Pixel *pbuf, *maxbuf;

	pbuf = buf;
	maxbuf = buf+width;
	while (pbuf < maxbuf)
		*pbuf++ = inky->dot(inky,x0++,y);
	PUT_HSEG(vb.pencel,buf,x0-width,y,width);
}

static BYTE lsp_disabled = 0;
void disable_lsp_ink()
{
	++lsp_disabled;
}
void enable_lsp_ink()
{
	--lsp_disabled;
}

Errcode render_hline(register SHORT y,register SHORT x0,SHORT x1,Raster *r)
{
SHORT width, w1, w2;
SHORT x;
void (*ink_hline)(const Ink *inky, SHORT x0,const SHORT y,SHORT width);

	if((USHORT)y >= r->height) /* clip that baby first thing... */
		return(0);
	width = r->width;
	if (x0 >= width)
		return(0);
	if (x1 < 0)
		return(0);

	ink_aid.ccolor = vs.ccolor; /* must set because of 
								 * potential color cycling */

	/* set line spread end points before clipping for lsp ink */
	if(ink_type == lsp_INKID && lsp_disabled <= 0)	
	{
		rdta.rdx0 = x0;
		rdta.rdx1 = x1;
		rdta.rwidth = x1-x0+1;
	}
	if (x0 < 0)
		x0 = 0;
	if (x1 >= width)
		x1 = width - 1;
	if (!render_hlineable)
	{
		while(x0 <= x1)
			render_dot(x0++, y, NULL);
		return(0);
	}
	w1 = width = x1-x0+1;

	if (render_write_mask)
		set_bit_hline(mask_rast->bm.bp[0], mask_rast->bm.bpr, y, x0, x1);
	x = x0;

	ink_hline = vl.ink->hline;
	while (w1 > 0)  /* call ink functions to spit out SBSIZE at a time */
	{
		w2 = w1;
		if (w2 > SBSIZE)
			w2 = SBSIZE;
		(*ink_hline)(vl.ink,x,y,w2);
		x += w2;
		w1 -= w2;
	}
	if (y_needs_zoom(y))
		rect_zoom_it(x0,y,width,1);
	return(0);
}

Errcode poll_render_hline(SHORT y,SHORT x0,SHORT x1,Raster *r)
{
render_hline(y,x0,x1,r);
return(poll_abort());
}

