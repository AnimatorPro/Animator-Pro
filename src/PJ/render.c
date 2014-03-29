
/* render.c - Routines to draw on screen, possibly behind menus, in
   current ink.  Most of them eventually funnel through render_dot, but
   the blits for speed and other reasons are implemented separately. */

#include "jimk.h"
#include "bhash.h"
#include "brush.h"
#include "errcodes.h"
#include "gfx.h"
#include "inkaid.h"
#include "inkdot.h"
#include "inks.h"
#include "mask.h"
#include "poly.h"
#include "rastcall.h"
#include "rastrans.h"
#include "render.h"
#include "zoom.h"

extern char under_flag;

Rendata rdta;

static Errcode
render_blit(Rcel *src, SHORT sx, SHORT sy, Rcel *dest, SHORT dx, SHORT dy,
		SHORT w, SHORT h, Tcolxldat *txd, Cmap *scmap);

static Errcode
tblit(Rcel *src, SHORT sx, SHORT sy,
		Rcel *dest, SHORT dx, SHORT dy, SHORT w, SHORT h,
		Pixel tcolor, int clear, int tinting, SHORT dither,
		Cmap *scmap, Pixel *xlat);

static Errcode r_box(Raster *r, SHORT x, SHORT y, SHORT xx, SHORT yy);

void set_render_clip(Rectangle *rect)

/* sets render clip to box specified in rectangle x,y,width,height */
{
	if(rect == NULL)
	{
		rdta.cr.x = rdta.cr.y = 0;
		rdta.cr.MaxX = vb.pencel->width;
		rdta.cr.MaxY = vb.pencel->height;
		return;
	}

	/* load and clip the new clip to the render form */

	rdta.cr.x = rect->x;
	if (rdta.cr.x < 0)
		rdta.cr.x = 0;
	if(rdta.cr.x > vb.pencel->width)
		rdta.cr.x = vb.pencel->width;

	rdta.cr.y = rect->y;
	if(rdta.cr.y < 0)
		rdta.cr.y = 0;
	if(rdta.cr.y > vb.pencel->height)
		rdta.cr.y = vb.pencel->height;

	rdta.cr.MaxX = rect->x + rect->width;
	if (rdta.cr.MaxX > vb.pencel->width)
		rdta.cr.MaxX = vb.pencel->width;
	rdta.cr.MaxY = rect->y + rect->height;
	if (rdta.cr.MaxY > vb.pencel->height)
		rdta.cr.MaxY = vb.pencel->height;
	return;
}
/*************************************************/
/* functions for setting the gradient boundaries */

void set_gradrect(Rectangle *rect)
{
	rdta.rdx0 = rect->x;
	rdta.rdy0 = rect->y;
	rdta.rdx1 = rect->x+rect->width-1;
	rdta.rdy1 = rect->y+rect->height-1;
	rdta.rwidth = rect->width;
	rdta.rheight = rect->height;
}

void set_twin_gradrect(void)
{
	set_gradrect(&vs.twin);
}

void set_xy_gradrect(SHORT x0,SHORT y0,SHORT x1,SHORT y1)
{
	rdta.rdx0 = x0;
	rdta.rdy0 = y0;
	rdta.rdx1 = x1;
	rdta.rdy1 = y1;
	rdta.rwidth = x1-x0+1;
	rdta.rheight = y1-y0+1;
}
static void set_centrad_gradrect(Raster *r, SHORT cenx,SHORT ceny,SHORT rad)
{
int xrad = rad;
int yrad = rad*r->aspect_dy/r->aspect_dx;

	rdta.rdx0 = cenx-xrad;
	rdta.rdy0 = ceny-yrad;
	rdta.rdx1 = cenx+xrad;
	rdta.rdy1 = ceny+yrad;
	rdta.rwidth = 2*xrad+1;
	rdta.rheight = 2*yrad+1;
}

void set_full_gradrect(void)
{
Rectangle r;

	r.x = r.y = 0;
	r.width = vb.pencel->width;
	r.height = vb.pencel->height;
	set_gradrect(&r);
}

/* end gradient rectangle setting functions */


static Errcode inkblit( Rcel *src,
			    	 SHORT sx,SHORT sy, /* beware, ignores sx, and sy */
					 Rcel *dst,
					 SHORT dx,SHORT dy,
					 SHORT w,SHORT h,
					 Pixel tcolor, Pixel *xlat ) 
 /* it does not look at cel->cmap and can take a raster input */
{
PLANEPTR s;
register SHORT sbpr;
PLANEPTR sbyte;
SHORT srcx, srcy;
Errcode err = Success;
int occolor, xend, yend;
int x,y;
(void)sx;
(void)sy;
(void)dst;

	if(src->type == RT_BYTEMAP)
	{
		sbpr = ((Bytemap *)src)->bm.bpr;
		s = ((Bytemap *)src)->bm.bp[0];
	}
	else
	{
		sbpr = 0;
		srcy = -1; /* note ++srcy in outer for loop */
	}

	occolor = vs.ccolor;
	xend = dx+w;
	yend = dy+h;

	for (y=dy; y<yend; y++)
	{
		if(sbpr)
		{
			sbyte = s;
			s += sbpr;
		}
		else
		{
			srcx = 0;
			++srcy;
		}

		if(under_flag)
		{
			for (x=dx; x<xend; x++)
			{
				if(pj_get_dot(undof,x,y) == tcolor)
				{
					if(sbpr)
						vs.ccolor = *sbyte++;
					else
					{
						vs.ccolor = pj_get_dot(src,srcx,srcy);
						++srcx;
					}
					if(xlat)
						vs.ccolor = xlat[vs.ccolor];
					ink_aid.ccolor = vs.ccolor;
					if ( !vs.zero_clear || vs.ccolor != tcolor)
						render_dot(x, y, NULL);
				}
				else
				{
					++sbyte;     /* the ++ is less than logic */
					++srcx;
				}
			}
		}
		else
		{
			for (x=dx; x<xend; ++x)
			{
				if(sbpr)
					vs.ccolor = *sbyte++;
				else
				{
					vs.ccolor = pj_get_dot(src,srcx,srcy);
					++srcx;
				}
				if(xlat)
					vs.ccolor = xlat[vs.ccolor];
				ink_aid.ccolor = vs.ccolor;
				if ( !vs.zero_clear || vs.ccolor != tcolor)
					render_dot(x, y, NULL);
			}
		}
	if ((err = poll_abort()) < Success)
		break;
	}
	ink_aid.ccolor = vs.ccolor = occolor;
return(err);
}
static Errcode tblit( Rcel *src, 
				   SHORT sx,SHORT sy, /* ignored! always assumed 0,0 */
				   Rcel *dest,  /* warning! ignored! always vb.pencel */
				   SHORT dx,SHORT dy,
				   SHORT w,SHORT h,
				   Pixel tcolor,
				   int clear, int tinting, SHORT dither,
				   Cmap *scmap, Pixel *xlat )

/* this applies a transparent (glass) source onto the destination */
{
SHORT sbpr; /* bytes per row and flag if source is a bytemap array */
PLANEPTR s;
UBYTE *spt;
UBYTE make_mask, use_mask;
SHORT x, y, scol, srow;
Rgb3 rgb;
UBYTE c, uc;
Errcode err = Success;
Rgb3 *dctab = dest->cmap->ctab, *sctab = scmap->ctab;

	sx = sy = 0; /* for now always from 0,0 */
	dest = vb.pencel; /* always the render form, so be it */

	/* note: it is assumed that the mask size
	 * is the same as the destination size */

	use_mask = (vs.use_mask && mask_rast);
	make_mask = (vs.make_mask && mask_rast);

	if(src->type == RT_BYTEMAP)
	{
		s = ((Bytemap *)src)->bm.bp[0];
		sbpr = ((Bytemap *)src)->bm.bpr;
	}
	else
		sbpr = 0;

	for (srow=sy, y=dy; srow < h; ++srow, ++y)
	{
		if(sbpr)
		{
			spt = s;
			s += sbpr;
		}

		if (y < 0 || y >= dest->height)
			continue;

		for (scol=sx; scol<w; ++scol)
		{
			x = scol + dx;

			if(sbpr)
				c = *spt++;
			else
				c = pj_get_dot(src,scol,srow);

			if (x < 0 || x >= dest->width)
				continue;

			if(use_mask && pj__get_dot(mask_rast,x,y))
				continue;

			if(xlat)
				c = xlat[c];
			uc = pj_get_dot(undof,x,y);
			if (under_flag)
			{
				if (uc != tcolor)
					continue;
			}
			else if (clear && c == tcolor)
				continue;

			if(make_mask)
				pj_put_dot(mask_rast,1,x,y);

	        true_blend(dctab+uc, sctab+c, tinting, &rgb);

			pj_put_dot(dest, bclosest_col(&rgb,COLORS,dither), x,y);
		}
	if ((err = poll_abort()) < Success)
		break;
	}
return(err);
}
Errcode transpblit(Rcel *tcel, int clearcolor, int clear, int tinting)
{
Errcode err;

	if((err = make_bhash()) < Success)
		return(err);
	err = tblit(tcel,0,0,vb.pencel,
		  tcel->x,tcel->y,tcel->width, tcel->height,
		  clearcolor,clear,tinting,vl.ink->dither,tcel->cmap, NULL);
	free_bhash();
	return(err);
}

static Boolean is_transp_ink(void)
{
return(vl.ink->ot.id == tlc_INKID || vl.ink->ot.id == tsp_INKID);
}

static int make_brender_cashes(void)
{
if (is_transp_ink())
	return(make_bhash());
else
	return(make_render_cashes());
}

static void free_brender_cashes(void)
{
if (is_transp_ink())
	free_bhash();
else
	free_render_cashes();
}
Errcode rblit_cel(Rcel *c, Tcolxldat *txd)
{
Errcode err;

	if((err = make_brender_cashes()) >= 0)
	{
		set_gradrect((Rectangle *)&(c->RECTSTART));
		err = render_blit(c,0,0,vb.pencel,
					c->x,c->y,c->width,c->height,
				    txd, c->cmap);
		free_brender_cashes();
	}
	return(err);
}

static Errcode
render_blit(Rcel *src, SHORT sx, SHORT sy, Rcel *dest, SHORT dx, SHORT dy,
		SHORT w, SHORT h, Tcolxldat *txd, Cmap *scmap)
/* this render blits an rcel, It is one of the "Celblit" function family 
 * it does not look at cel->cmap and can take a raster input */
{
Errcode err;
Celblit cblit;
Pixel tcolor = txd->tcolor;
Pixel *xlat;

	start_abort_atom();

	if(vs.render_one_color)
		xlat = txd->xlat;
	else
		xlat = NULL;

	switch (vl.ink->ot.id) 
	{
		case opq_INKID:	/* opaque */
			if (vs.make_mask || vs.use_mask)
			{
				err = inkblit(src,sx,sy,dest,dx,dy,w,h,tcolor,txd->xlat);
				break;
			}
			else
			{
				cblit = get_celblit(txd->xlat != NULL);
				(*cblit)(src,sx,sy,dest,dx,dy,w,h,txd);
				rect_zoom_it(dx,dy,w,h);
				err = Success;
				break;
			}
		case tlc_INKID:	/* translucent */
		case tsp_INKID:	/* transparent */
			if(xlat)
				scmap = dest->cmap;
			err = tblit( src,sx,sy,
				   dest,dx,dy,w,h,tcolor,
				   vs.zero_clear,
				   vl.ink->strength,
				   vl.ink->dither,
				   scmap, xlat);
			rect_zoom_it(dx,dy,w,h);
			break;
		default:
			err = inkblit(src,sx,sy,dest,dx,dy,w,h,tcolor,xlat);
			break;
	}
	return(errend_abort_atom(err));
}

/* Function: render_mask_blit
 *
 *  Does the render dot equivalent of pj_mask1blit() args must be same order.
 *
 *  drast - currently ignored uses vb.pencel.
 */
void
render_mask_blit(UBYTE *mplane, SHORT mbpr, SHORT mx, SHORT my,
		void *drast, SHORT rx, SHORT ry, USHORT width, USHORT height)
{
UBYTE mbit_mx;
UBYTE *mbyte;
UBYTE mbits;
UBYTE mbit;
int x, y, MaxX, MaxY;
(void)drast;

	mplane += (my * mbpr) + (mx >> 3);
	mbit_mx = 0x80 >> (mx & 0x07);
	MaxX = rx + width;
	MaxY = ry + height;

	for(y = ry;y < MaxY;++y)
	{
		mbyte = mplane;
		mplane += mbpr;
		mbits = *mbyte++;
		mbit = mbit_mx;

		for(x = rx;x < MaxX;++x)
		{
			if(mbits & mbit)
				render_dot(x, y, NULL);

			if((mbit >>= 1) == 0)
			{
				mbits = *mbyte++;
				mbit = 0x80;
			}
		}
	}
}

static void alpha_blend(Rgb3 *source1, Rgb3 *source2, Rgb3 *dest, int alpha)
/*****************************************************************************
 * Blend source1 and source2 into dest using alpha.
 ****************************************************************************/
{
int valpha = 255-alpha;		/* Inverse alpha */

dest->r = (source1->r * valpha + source2->r * alpha + 127)/255;
dest->g = (source1->g * valpha + source2->g * alpha + 127)/255;
dest->b = (source1->b * valpha + source2->b * alpha + 127)/255;
}

void render_mask_alpha_blit(UBYTE *alpha, int abpr, int dx, int dy, 
int width, int height, Rcel *r, Pixel oncolor)
/*****************************************************************************
 * Render color transparently through alpha channel data onto a raster...
 ****************************************************************************/
{
int x = dx, y = dy;
int w = width, h = height;
int 	curx;
int 	endx;
UBYTE	curalpha;
UBYTE	*palpha;
Rgb3 dest;
Cmap *cmap = r->cmap;
Rgb3 *ctab = cmap->ctab;
Rgb3 *s1, *s2;
(void)oncolor;

endx = x + w;

while (h--) 
	{
	palpha	= alpha;
	for (curx = x; curx < endx; ++curx) 
		{
		curalpha = *palpha++;
		if (curalpha) 
			{
			if (curalpha == 255) 
				{
				render_dot(curx, y, NULL);
				} 
			else 
				{
				/* Sample raster before and after render_dot.  Put back a
				 * mixture of two values. */
				s1 = &ctab[pj_get_dot(r, curx, y)];
				render_dot(curx, y, NULL);
				s2 = &ctab[pj_get_dot(r, curx, y)];
				alpha_blend(s1, s2, &dest, curalpha);
				pj_put_dot(r, closestc(&dest, ctab, cmap->num_colors)
				, curx, y);
				}
			}
		}
	alpha += abpr;
	++y;
	}
rect_zoom_it(dx,dy,width,height);
}



Errcode render_disk(Raster *r, SHORT cenx,SHORT ceny,SHORT diam)
{
Errcode err;

	if((err = make_render_cashes()) < Success)
		return(err);
	set_centrad_gradrect(r,cenx,ceny,(diam+1)>>1);
	start_abort_atom();
	err = doval(cenx,ceny,diam,r->aspect_dx, r->aspect_dy,
			    NULL, NULL, poll_render_hline, r, TRUE);
	free_render_cashes();
	return(errend_abort_atom(err));
}


#ifdef SLUFFED
void render_rectframe(Raster *r, register Rectangle *rect)

/* renders quadrilateral frame for a rectangle */
{
SHORT x1,y1;

	set_full_gradrect();
	if (make_render_cashes() < 0)
		return;

	x1 = rect->x+rect->width-1;
	y1 = rect->y+rect->height-1;
	render_line(rect->x, rect->y, x1, rect->y);
	render_line(x1, rect->y, x1, y1);
	render_line(x1, y1, rect->x, y1);
	render_line(rect->x, y1, rect->x, rect->y);
	free_render_cashes();
}
#endif /* SLUFFED */

Errcode render_box(SHORT x,SHORT y,SHORT xx,SHORT yy)
{
Errcode err;

	if (make_render_cashes() < 0)
		return(Err_no_memory);
	set_xy_gradrect(x,y,xx,yy);
	err = r_box((Raster *)vb.pencel,x,y,xx,yy);
	free_render_cashes();
	return(err);
}
#ifdef SLUFFED
int render_rect(Rectangle *rect)
{
	return(render_box(rect->x,rect->y,
		   rect->x+rect->width-1,rect->y+rect->height-1));
}
#endif /* SLUFFED */
Errcode render_beveled_box(Rectangle *r, int bevel, Boolean filled)
{
Poly poly;
LLpoint pts[8];
int x1,y1;
int i;

	x1 = r->x+r->width-1;
	y1 = r->y+r->height-1;
	if (bevel > r->width/2)
		bevel = r->width/2;
	if (bevel > r->height/2)
		bevel = r->height/2;
	for (i=0; i<7; i++)
		{
		pts[i].next = &pts[i+1];
		pts[i].z = 0;
		}
	pts[7].next = &pts[0];
	pts[7].z = 0;
	pts[0].x = r->x+bevel;
	pts[0].y = r->y;
	pts[1].x = x1-bevel;
	pts[1].y = r->y;
	pts[2].x = x1;
	pts[2].y = r->y+bevel;
	pts[3].x = x1;
	pts[3].y = y1-bevel;
	pts[4].x = x1-bevel;
	pts[4].y = y1;
	pts[5].x = r->x+bevel;
	pts[5].y = y1;
	pts[6].x = r->x;
	pts[6].y = y1-bevel;
	pts[7].x = r->x; 
	pts[7].y = r->y+bevel;
	poly.pt_count = 8;
	poly.clipped_list = pts;
	poly.polymagic = POLYMAGIC;
	render_poly(&poly, filled, TRUE);

	return Success;
}

static Errcode r_box(Raster *r, SHORT x, SHORT y, SHORT xx, SHORT yy)
{
int i;
int swap;
Errcode err;

	if (x > xx)
	{
		swap = x;
		x = xx;
		xx = swap;
	}
	if (y > yy)
	{
		swap = y;
		y = yy;
		yy = swap;
	}
	start_abort_atom();
	for (i=y;i<=yy;i++)
	{
		if ((err = poll_render_hline(i, x, xx,r)) < Success)
			break;
	}
	err = errend_abort_atom(err);
return(err);
}


void render_outline(Short_xy *pt,int count)
{
Short_xy *last;

	last = pt+count-1;
	while (--count >= 0)
	{
		render_line(last->x, last->y, pt->x, pt->y); 
		last = pt;
		pt++;
	}
}

void render_brush(SHORT x, SHORT y)

/* takes a brush "mask" and does a render dot for each non 0 dot in brush */
{
Pixel *plane;
register Pixel *dot;
int bpr;
SHORT MaxX, MaxY, xstart;
Rbrush *rb = vl.brush;

	if(!vs.use_brush)
	{
		render_dot(x, y, NULL);
		return;
	}

	if(vl.ink->ot.id == opq_INKID && !(vs.use_mask || vs.make_mask))
	{
		blit_brush(rb,vb.pencel,x,y);
		rect_zoom_it(x - rb->cent.x, y - rb->cent.y,
					 rb->width, rb->height );
		return;
	}

	disable_lsp_ink();
	plane = rb->rast->bm.bp[0];
	bpr = rb->rast->bm.bpr;

	y -= rb->cent.y;
	xstart = x - rb->cent.x;  
	MaxY = y + rb->height;
	MaxX = xstart + rb->width;

	for(;y < MaxY;++y)
	{
		dot = plane;
		plane += bpr;
		for(x = xstart;x < MaxX;++x) 
		{
			if(*dot++)
				render_dot(x, y, NULL);
		}
	}
	enable_lsp_ink();
}

static void render_brush_with_data(SHORT x, SHORT y, void *data)
{
	(void)data;
	render_brush(x, y);
}

static void
render_line_with_data(SHORT x1, SHORT y1, SHORT x2, SHORT y2, void *data)
{
	(void)data;
	render_line(x1, y1, x2, y2);
}

Errcode render_opoly(Poly *p, Boolean closed)
{
LLpoint *this;

	this = p->clipped_list;
	render_brush(this->x,this->y);	/* round off the 1st end */
	hollow_polygon(p, render_line_with_data, NULL, closed);
	return(Success);
}

Errcode render_circle(Raster *r, SHORT cenx,SHORT ceny,SHORT diam)
{
Errcode err;
SHORT gradrad;

	gradrad = (diam+1)>>1;
	if(vs.use_brush)
		gradrad += get_brush_size();
	set_centrad_gradrect(r,cenx,ceny,gradrad);
	if((err = make_render_cashes()) < Success)
		return(err);
	doval(cenx,ceny,diam, r->aspect_dx, r->aspect_dy,
			vs.use_brush ? render_brush_with_data : render_dot, vb.pencel,
			NULL, NULL, FALSE);
	free_render_cashes();
	return(Success);
}


Errcode render_separate(PLANEPTR ctable,int ccount,Rectangle *rect)
{
Errcode err;
register PLANEPTR maptable = NULL;
SHORT x, y, maxx, maxy;
SHORT xstart;
SHORT in;
UBYTE *pixbuf = NULL;
UBYTE *pixpt;

	if((err = make_render_cashes()) < 0)
		return(err);
	if ((maptable = pj_malloc(COLORS)) == NULL)
		{
		err = Err_no_memory;
		goto ERROR;
		}
	if ((pixbuf = pj_malloc(rect->width)) == NULL)
		{
		err = Err_no_memory;
		goto ERROR;
		}
	zero_structure(maptable, COLORS);
	while (--ccount >= 0)
		maptable[*ctable++] = 1;
	set_gradrect(rect);

	maxx = rect->x + rect->width;
	maxy = rect->y + rect->height;

	start_abort_atom();
	for (y=rect->y; y<maxy; ++y)
	{
		pj__get_hseg(vb.pencel,pixbuf,rect->x,y,rect->width);
		pixpt = pixbuf;
		in = FALSE;
		for (x=rect->x; x<maxx; ++x)
		{
			if (in)
			{
				if (!maptable[*pixpt++])
				{
					if((err = render_hline(y, xstart, x-1, 
										   (Raster *)vb.pencel)) < Success)
					{
						goto end_abort_error;
					}
					in = FALSE;
				}
			}
			else
			{
				if (maptable[*pixpt++])
				{
					xstart = x;
					in = TRUE;
				}
			}
		}

		if (in)
		{
			if((err = render_hline(y, xstart, x-1, 
								   (Raster *)vb.pencel)) < Success)
			{
				goto end_abort_error;
			}
		}

		if ((err = poll_abort()) < Success)
			goto end_abort_error;
	}
end_abort_error:
	err = errend_abort_atom(err);
ERROR:
	free_render_cashes();
	pj_gentle_free(maptable);
	pj_gentle_free(pixbuf);
	return(err);
}

typedef struct rldat {
	Cmap *scmap;
	Tcolxldat *txd;
} Rldat;

static Errcode rblit_line(void *rldat, Pixel *line, Coor x, Coor y, Ucoor width)
{
Rldat *rld = rldat;
Raster linerast;
Rasthdr spec;

	spec.width = width;
	spec.height = 1;
	spec.aspect_dx = spec.aspect_dy = 1;
	spec.pdepth = 8;

	pj_build_bytemap(&spec,&linerast,(UBYTE *)line);
	return(render_blit((Rcel *)&linerast,0,0,vb.pencel,x,y,width,1,
		    			rld->txd,rld->scmap));
}

Errcode render_transform(Rcel *cel, Xformspec *xf, Tcolxldat *txd)
{
Errcode err;
Rldat rld;

	rld.txd = txd;
	rld.scmap = cel->cmap;

	if((err = make_brender_cashes()) < 0)
		return(err);
	start_abort_atom();
	set_xy_gradrect(xf->mmax.x,xf->mmax.y,xf->mmax.MaxX,xf->mmax.MaxY);
	err = raster_transform(cel,vb.pencel,xf,rblit_line,&rld,FALSE);
	free_brender_cashes();
	return(errend_abort_atom(err));
}
