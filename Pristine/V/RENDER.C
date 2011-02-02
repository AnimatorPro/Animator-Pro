
/* render.c - Routines to draw on screen, possibly behind menus, in
   current ink.  Most of them eventually funnel through render_dot, but
   the blits for speed and other reasons are implemented separately. */

#include "jimk.h"
#include "poly.h"
#include "inks.h"

extern int copydot(),marqidot(),sdot(), xdot();
extern a1bdot(), rbdot(), rbbrush();
extern Vcel *cel;

extern PLANEPTR mask_plane;
extern char under_flag;


static WORD rdx0,rdy0,rdx1,rdy1,rwidth,rheight;
char *gel_thash;
WORD gel_factor;

render_grad_twin()
{
render_xywh(vs.twin_x,vs.twin_y,vs.twin_w,vs.twin_h);
}

render_xy(x0,y0,x1,y1)
WORD x0,y0,x1,y1;
{
rdx0 = x0;
rdy0 = y0;
rdx1 = x1;
rdy1 = y1;
rwidth = x1-x0+1;
rheight = y1-y0+1;
}

render_xywh(x,y,w,h)
WORD x,y,w,h;
{
rdx0 = x;
rdy0 = y;
rdx1 = x+w-1;
rdy1 = y+h-1;
rwidth = w;
rheight = h;
}

static
render_center_rad(cenx,ceny,rad)
WORD cenx, ceny, rad;
{
rdx0 = cenx-rad;
rdy0 = ceny-rad;
rdx1 = cenx+rad;
rdy1 = ceny+rad;
rwidth = rheight = 2*rad+1;
}

render_full_screen()
{
rdx0 = 0;
rdy0 = 0;
rdx1 = vf.w-1;
rdy1 = vf.h-1;
rwidth = vf.w;
rheight = vf.h;
}

#ifdef SLUFFED
bizarre_blit()
{
if (!make_render_cashes())
	return;
inkblit(cel->w, cel->h, 0, 0, cel->p, cel->bpr, cel->dx, cel->dy,
	NULL, BPR, vs.inks[0]);
free_render_cashes();
}
#endif SLUFFED

/* beware, ignores sx, sy and d */
static
inkblit(w,h,sx,sy,s, sbpr, dx, dy, d, dbpr, tcolor)
WORD w,h,sx,sy,sbpr,dx,dy,dbpr;
PLANEPTR s,d;
UBYTE tcolor;
{
int occolor, xend, yend, smod;
int x,y;

occolor = vs.ccolor;
xend = dx+w;
yend = dy+h;
smod = sbpr-w;
for (y=dy; y<yend; y++)
	{
	if (under_flag)
		{
		for (x=dx; x<xend; x++)
			{
			if (getd(uf.p,x,y) == tcolor)
				{
				vs.ccolor = *s;
				if ( !vs.zero_clear || vs.ccolor != tcolor)
					render_dot(x,y);
				}
			s++;
			}
		}
	else
		{
		for (x=dx; x<xend; x++)
			{
			vs.ccolor = *s;
			if ( !vs.zero_clear || vs.ccolor != tcolor)
				render_dot(x,y);
			s++;
			}
		}
	s += smod;
	}
vs.ccolor = occolor;
}


transpblit(tcel,clearcolor, clear, tinting)
register Vcel *tcel;
int clearcolor, clear, tinting;
{
if (!make_bhash())
	return;
tblit(tcel->w, tcel->h, 0, 0, tcel->p, tcel->bpr, tcel->x, tcel->y,
	render_form->p, BPR, clearcolor, clear, tinting, tcel->cmap);
free_bhash();
}


static
tblit(w,h,sx,sy,s, sbpr, dx, dy, d, dbpr, tcolor, clear, tinting, scmap)
WORD w,h,sx,sy,sbpr,dx,dy,dbpr;
PLANEPTR s,d;
UBYTE tcolor;
int clear, tinting;
PLANEPTR scmap;
{
int x, y, i, j, x0, y0;
unsigned char rgb[3];
UBYTE *spt, c, uc;
UBYTE *sline;

for (j=0; j<h; j++)
	{
	spt = s;
	s += sbpr;
	y = j + dy;
	if (y < 0 || y >= YMAX)
		continue;
	for (i=0; i<w; i++)
		{
		c = *spt++;
		x = i + dx;
		if (x < 0 || x >= XMAX)
			{
			continue;
			}
		if (vs.use_mask && mask_plane)
			{
			if ((mask_plane[y*Mask_line(XMAX)+(x>>3)] & (0x80>>(x&7))))
				{
				continue;
				}
			}
		uc = getd(uf.p,x,y);
		if (under_flag)
			{
			if (uc != tcolor)
				continue;
			}
		else if (clear && c == tcolor)
			continue;
		if (vs.make_mask && mask_plane)
			{
			mask_plane[y*Mask_line(XMAX)+(x>>3)] |= (0x80>>(x&7));
			}
		true_blend(render_form->cmap+3*getd(uf.p,x,y), 
			scmap+3*c, tinting, rgb);
		cdot(render_form->p,x,y,bclosest_col(rgb, COLORS));
		}
	}
}

make_brender_cashes()
{
if (vs.draw_mode == 4 || vs.draw_mode == 6)
	return(make_bhash());
else
	return(make_render_cashes());
}

free_brender_cashes()
{
free_render_cashes();
}

rblit_cel(c)
register Vcel *c;
{
if (make_brender_cashes())
	{
	render_xywh(c->x,c->y,c->w,c->h);
	render_blit(c->w, c->h, 0, 0, c->p, c->bpr,
		c->x, c->y, render_form->p, BPR, vs.inks[0], c->cmap);
	free_brender_cashes();
	return(1);
	}
return(0);
}

cfit_rblit_cel(c)
Vcel *c;
{
if (need_fit_cel(c))
	{
	if ((c = clone_cel(c)) == NULL)
		{
		outta_memory();
		return(0);
		}
	cfit_cel(c,render_form->cmap);
	rblit_cel(c);
	free_cel(c);
	}
else
	rblit_cel(c);
return(1);
}


#ifdef SLUFFED
render_cel()
{
rblit_cel(cel);
}
#endif SLUFFED

extern Vector cbvec;

render_blit(w,h,sx,sy,s, sbpr, dx, dy, d, dbpr, tcolor, scmap)
WORD w,h,sx,sy,sbpr,dx,dy,dbpr;
PLANEPTR s,d;
UBYTE tcolor;
PLANEPTR scmap;
{
Vector v;


switch (vs.draw_mode)
	{
	case 0:	/* opaque */
		if (vs.make_mask || vs.use_mask)
			inkblit(w,h,sx,sy,s, sbpr, dx, dy, d, dbpr, tcolor);
		else
			{
			if (under_flag)
				v = ublit8;
			else
				{
				set_zero_clear();
				v = cbvec;
				}
			(*v)(w,h,sx,sy,s, sbpr, dx, dy, d, dbpr, tcolor);
			}
		break;
	case 4:	/* translucent */
	case 6:	/* transparent */
		tblit(w,h,sx,sy,s, sbpr, dx, dy, d, dbpr, tcolor, vs.zero_clear,
			vs.tint_percent, scmap);
		break;
	default:
		inkblit(w,h,sx,sy,s, sbpr, dx, dy, d, dbpr, tcolor);
		break;
	}
}


static
dfrom_range(p,q,x,y)
unsigned p,q;
int x,y;
{
WORD color;
WORD start, samples;

if (q == 0)
	return(vs.crange.startc);
p *= vs.buns[vs.use_bun].bun_count-1;
if (vs.dither)
	{
	color = p/q;
	if (p%q*256L/q > (((x&y)*83+(x+y)*63+x*37)&255))	/* randomizer */
		color++;
	}
else
	color = (p+q/2)/q;
color = vs.buns[vs.use_bun].bundle[color];
return(color);
}

static
clip_xy(p)
int *p;
{
int i;

if ((i = *p) < 0)
	i = 0;
else if (i >= XMAX)
	i = XMAX-1;
*p++ = i;
if ((i = *p) < 0)
	i = 0;
else if (i >= YMAX)
	i = YMAX-1;
*p++ = i;
}


int render_xmin, render_ymin;
int render_xmax = XMAX, render_ymax = YMAX;
extern UBYTE *glow_lookup;

render_dot(x,y)
register WORD x,y;
{
extern WORD gel_factor;
extern char *gel_thash;
WORD color;
WORD nxy[2];
WORD endc;
UBYTE *c,*c2;
WORD i;
UBYTE rgb[3];
WORD temp;


if (y >= render_ymin & y < render_ymax && x >= render_xmin 
	&& x < render_xmax)
	{
	if (vs.use_mask && mask_plane)
		{
		if ((mask_plane[y*Mask_line(XMAX)+(x>>3)] & (0x80>>(x&7))))
			return;
		}
	if (vs.make_mask && mask_plane)
		{
		mask_plane[y*Mask_line(XMAX)+(x>>3)] |= (0x80>>(x&7));
		}
	switch (vs.draw_mode)
		{
		case 1:
			color = dfrom_range(y-rdy0,rheight,x,y);
			break;
		case 2:	/* horizontal spreads map to same thing... */
		case 3:
			color = dfrom_range(x-rdx0,rwidth,x,y);
			break;
		case 4:		/* translucent */
			color = tclosest_color(dwgetdot(x,y));
			break;
		case 5:		/* soften */
			colorave(x,y,rgb,uf.p,render_form->cmap);
			color = bclosest_col(rgb,COLORS);
			break;
		case 6:		/* transparent */
		case 21:	/* darken */
			color = tclosest_color(getd(uf.p,x,y));
			break;
		case 7:		/* reveal alt */
			if (alt_form)
				color = rclosest_color(alt_form->cmap, getd(alt_form->p,x,y));
			else
				color = vs.ccolor;
			break;
		case 8:	/* xor */
			color = getd(uf.p,x,y)^vs.ccolor;
			break;
		case 9: /* jumble */
			endc = vs.tint_percent+1;
			nxy[0] = x+random()%(endc) - (endc>>1);
			nxy[1] = y+random()%(endc) - (endc>>1);
			clip_xy(&nxy[0]);
			color = getd(uf.p,nxy[0],nxy[1]);
			break;
		case 10: /* add */
			color = (getd(uf.p,x,y)+vs.ccolor)&(COLORS-1);
			break;
		case 11:	/* glowrange */
			color = glow_lookup[getd(uf.p,x,y)];
			break;
		case 12:	/* Cel Tile */
			if (cel)
				{
				nxy[0] = x - cel->x;
				while (nxy[0] >= (int)cel->w)
					nxy[0] -= cel->w;
				while (nxy[0] < 0)
					nxy[0] += cel->w;
				nxy[1] = y - cel->y;
				while (nxy[1] >= (int)cel->h)
					nxy[1] -= cel->h;
				while (nxy[1] < 0)
					nxy[1] += cel->h;
				color = *(cel->p + cel->bpr * nxy[1] + nxy[0]);
				if (vs.zero_clear && color == vs.inks[0])
					return;
				color = rclosest_color(cel->cmap, color);
				}
			else
				color = vs.ccolor;
			break;
		case 13:	/* Crystalize */
			if (x<0 || x>=XMAX || y<0 || y >= YMAX)
				return;
			color = (getd(uf.p,x+1,y) + getd(uf.p,x-1,y) + getd(uf.p, x, y-1) +
				getd(uf.p, x, y+1))&(COLORS-1);
			break;
		case 14:	/* Shatter */
			if (y&1)
				{
				nxy[0] = x+vs.tint_percent;
				}
			else
				{
				nxy[0] = x - vs.tint_percent;
				}
			if (nxy[0] < 0 || nxy[0] >= XMAX)
				color = vs.inks[0];
			else
				color = getd(uf.p,nxy[0],y);
			break;
		case 15:	/* Antialias */
			if (x<0 || x>=XMAX || y<0 || y >= YMAX)
				return;
			endc = getd(uf.p,x,y);
			if ((endc == getd(uf.p,x+1,y) && endc == getd(uf.p,x-1,y)) ||
				(endc == getd(uf.p,x,y-1) && endc == getd(uf.p,x,y+1) ) ||
				(endc == getd(uf.p,x-1,y-1) && endc == getd(uf.p,x+1,y+1)) ||
				(endc == getd(uf.p,x-1,y+1) && endc == getd(uf.p,x+1,y-1)) )
				color = endc;
			else
				{
				colorave(x,y,rgb,uf.p,render_form->cmap);
				color = bclosest_col(rgb,COLORS);
				}
			break;
		case 16:	/* outlines */
			if (x<0 || x>=XMAX || y<0 || y >= YMAX)
				return;
			endc = getd(uf.p,x,y);
			if (endc == getd(uf.p,x+1,y) && endc == getd(uf.p,x-1,y) &&
				endc == getd(uf.p,x,y-1) && endc == getd(uf.p,x,y+1) ) 
				color = vs.inks[0];
			else
				color = endc;
			break;
		case 17:	/* brighten */
			endc = getd(uf.p,x,y);
			c = render_form->cmap + 3*endc;
			rgb[0] = brighten_ccomp(*c++);
			rgb[1] = brighten_ccomp(*c++);
			rgb[2] = brighten_ccomp(*c++);
			color = bclosest_col(rgb,COLORS);
			break;
		case 18:	/* desaturate */
			desaturate(render_form->cmap + 3*getd(uf.p,x,y), rgb);
			color = bclosest_col(rgb, COLORS);
			break;
		case 19:	/* sweep */
			if (x<0 || x>=XMAX || y<0 || y >= YMAX)
				return;
			color = getd(uf.p,x,y);
			endc = getd(uf.p,x,y-1);
			if (endc != color)
				{
				if (endc == getd(uf.p, x,y+1) && endc == getd(uf.p,x+1,y) &&
					endc == getd(uf.p, x-1,y) )
					color = endc;
				}
			break;
		case 20:	/* close holes */
			color = close_hole(x,y);
			break;
		case 22:	/* emboss */
			nxy[0] = x-1;
			nxy[1] = y-1;
			clip_xy(nxy);
			c = render_form->cmap+3*getd(uf.p,nxy[0],nxy[1]);
			c2 = render_form->cmap+3*getd(uf.p,x,y);
			rgb[0] = emb1c(*c++, *c2++);
			rgb[1] = emb1c(*c++, *c2++);
			rgb[2] = emb1c(*c++, *c2++);
			color = bclosest_col(rgb, COLORS);
			break;
		case 23:	/* pull */
			nxy[0] = x + (lastx-uzx);
			nxy[1] = y + (lasty-uzy);
			clip_xy(nxy);
			color = dwgetdot(nxy[0],nxy[1]);
			break;
		case 24:	/* smear */
			nxy[0] = x + ((lastx-uzx)+1)/2;
			nxy[1] = y + ((lasty-uzy)+1)/2;
			clip_xy(nxy);
			color = getd(uf.p, nxy[0],nxy[1]);
			break;
		case 25:	/* rad spread */
			temp = calc_distance(x,y,vs.rgx,vs.rgy)%vs.rgr;
			color = dfrom_range(temp,vs.rgr,x,y);
			break;
		default:
			color = vs.ccolor;
			break;
		}
	if (gel_factor)
		{
		if (vs.draw_mode == 0)
			{
			color = tccolor(dwgetdot(x,y), vs.ccolor,
				gel_factor, gel_thash);
			}
		else
			{
			true_blend(render_form->cmap+3*dwgetdot(x,y),
				render_form->cmap+3*color, gel_factor, rgb);
			color = bclosest_col(rgb, COLORS);
			}
		}
	if (vs.zoom_mode)
		upd_zoom_dot(x,y,color);
	dwlinestart[y][x] = color;
	}
}


static
emb1c(s,d)
int s, d;
{
int r;

r = d + ((d-s)*vs.tint_percent+50)/100;
if (r < 0)
	r = 0;
if (r > 63)
	r = 63;
return(r);
}


/* this table was made up by hand using eyeballs and intuition.  THere's
   really no algorithm behind it. */
static char chtab[32] = {
	0x0, 0x8, 0x0, 0xcc, 0x2e, 0x8, 0xea, 0xc8,
	0x7f, 0x8, 0x40, 0x0, 0x3a, 0x8, 0x0, 0x0,
	0x74, 0xfc, 0x40, 0xc8, 0x7f, 0x4, 0x46, 0x80,
	0x76, 0x0, 0x40, 0x0, 0x24, 0x0, 0x0, 0x0,
	};

static
close_hole(x,y)
int x,y;
{
WORD color;
WORD nbd;


color = getd(uf.p,x,y);

/* if pixels already set leave it */
if (color == vs.ccolor)
	return(color);


/* get pixel neighborhood */
nbd = 0;
if ( getd(uf.p,x-1,y-1) == vs.ccolor)
	nbd |= 1;
if ( getd(uf.p,x,y-1) == vs.ccolor)
	nbd |= 2;
if ( getd(uf.p,x+1,y-1) == vs.ccolor)
	nbd |= 4;
if ( getd(uf.p,x-1,y) == vs.ccolor)
	nbd |= 8;
if ( getd(uf.p,x+1,y) == vs.ccolor)
	nbd |= 16;
if ( getd(uf.p,x-1,y+1) == vs.ccolor)
	nbd |= 32;
if ( getd(uf.p,x,y+1) == vs.ccolor)
	nbd |= 64;
if ( getd(uf.p,x+1,y+1) == vs.ccolor)
	nbd |= 128;
if ((0x80>>(nbd&7))&chtab[nbd>>3])
	color = vs.ccolor;
return(color);
}



#ifdef SLUFFED
c_clip_blit(p)
register struct clipp {int w,h,sx,sy; UBYTE *spt; int sbpr; int dx,dy;} *p;
{
int dif;

if ((dif = render_xmin - p->dx) > 0)
	{
	p->dx += dif;
	p->sx += dif;
	p->w -= dif;
	}
if ((dif = render_ymin - p->dy) > 0)
	{
	p->dy += dif;
	p->sy += dif;
	p->h -= dif;
	}
if ((dif = p->dx + p->w - render_xmax) > 0)
	p->w -= dif;
if ((dif = p->dy + p->h - render_ymax) > 0)
	p->h -= dif;
return(p->w > 0 && p->h > 0);
}
#endif SLUFFED

render_bitmap_blit(w,h,sx,sy,spt,sbpr,dx,dy)
WORD w, h, sx, sy, sbpr,dx,dy;
PLANEPTR spt;
{
int i,j;
int xs,ys,xd,yd;

i = h;
ys = sy;
yd = dy;
while (--i >= 0)
	{
	j = w;
	xs = sx;
	xd = dx;
	while (--j >= 0)
		{
		if (spt[ys*sbpr + (xs>>3)] & bmasks[xs&7] )
			render_dot(xd,yd);
		xs++;
		xd++;
		}
	ys++;
	yd++;
	}
}

render_hline(y, x0, x1)
register WORD y,x0;
WORD x1;
{
WORD width;

/* clip that baby first thing... */
if (y < 0 || y >= YMAX)
	return;
if (x0 >= (int)XMAX)
	return;
if (x1 < 0)
	return;
if (x0 < 0)
	x0 = 0;
if (x1 >= XMAX)
	x1 = XMAX-1;

width = x1-x0+1;
if (vs.draw_mode == 3)
	{
	rdx0 = x0;
	rdx1 = x1;
	rwidth = width;
	}
while (--width >= 0)
	{
	render_dot(x0++,y);
	}
}


render_disk(cenx,ceny,rad)
WORD cenx, ceny, rad;
{
if (!make_render_cashes())
	return;
render_center_rad(cenx,ceny,rad);
ccircle(cenx,ceny,rad,NULL,render_hline,TRUE);
free_render_cashes();
}


render_frame(x0, y0, x1, y1)
int x0, y0, x1, y1;
{
render_full_screen();
if (!make_render_cashes())
	return;
render_line(x0, y0, x1, y0);
render_line(x1, y0, x1, y1);
render_line(x1, y1, x0, y1);
render_line(x0, y1, x0, y0);
free_render_cashes();
}

render_box(x,y,xx,yy)
WORD x, y, xx, yy;
{

if (!make_render_cashes())
	return(0);
render_xy(x,y,xx,yy);
r_box(x,y,xx,yy);
free_render_cashes();
return(1);
}

r_box(x,y,xx,yy)
WORD x, y, xx, yy;
{
int i;
int swap;

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
for (i=y;i<=yy;i++)
	{
	render_hline(i, x, xx);
	}
}


#ifdef SLUFFED
brush_blitblock(b)
register struct blitblock *b;
{
b->width = b->height = 16;
b->sx = b->sy = 0;
b->spt = (WORD *)dot_pens[vs.pen_width];
b->sbpr = 2;
b->dx = b->dy = -8;
b->dpt = (WORD *)render_form->p;
b->dbpr = BPR;
b->color = vs.ccolor;
}
#endif SLUFFED

render_brush(x,y)
WORD x, y;
{
if (vs.pen_width == 0)
	render_dot(x,y);
else
	{
	render_bitmap_blit(16,16,0,0,dot_pens[vs.pen_width],2,
		x-8, y-8);
	}
}

render_outline(pt, count)
Point *pt;
int count;
{
Point *last;

last = pt+count-1;
while (--count >= 0)
	{
	render_line(last->x, last->y, pt->x, pt->y); 
	last = pt;
	pt++;
	}
}


render_opoly(poly)
Poly *poly;
{
register struct llpoint *this, *next;
int i;

i = poly->pt_count;
if (!poly->closed)
	--i;
this = poly->clipped_list;
render_brush(this->x,this->y);	/* round off the 1st end */
while (--i >= 0)
	{
	next = this->next;
	render_line(this->x, this->y, next->x, next->y); 
	this = next;
	}
}

render_circle(cenx, ceny, rad)
WORD cenx, ceny, rad;
{
render_center_rad(cenx,ceny,rad+((vs.pen_width+1)>>1));
if (!make_render_cashes())
	return;
ccircle(cenx,ceny,rad,
	(vs.pen_width ? render_brush : render_dot), NULL, FALSE);
free_render_cashes();
}

render_separate(ctable, ccount,x0, y0, x1, y1)
PLANEPTR ctable;
int ccount;
int x0,y0,x1,y1;
{
register PLANEPTR maptable;
int i, j;
PLANEPTR p;

if (!make_render_cashes())
	return(0);
if ((maptable = begmemc(COLORS)) == NULL)
	{
	free_render_cashes();
	return(0);
	}
while (--ccount >= 0)
	maptable[*ctable++] = 1;
render_xy(x0,y0,x1,y1);
for (j=y0; j<=y1; j++)
	for (i=x0; i<=x1; i++)
		if (maptable[dwgetdot(i,j)])
			render_dot(i,j);
free_render_cashes();
freemem(maptable);
}

