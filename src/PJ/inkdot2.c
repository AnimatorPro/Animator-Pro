#include "jimk.h"
#include "bhash.h"
#include "errcodes.h"
#include "inkdot.h"
#include "options.h"
#include "rastlib.h"
#include "render.h"
#include "util.h"

static Pixel rclosest_color(Thash *th, const Rgb3 *oldc,
							const SHORT underc, const SHORT dither)
{
	if (dither)
	{
		return(bclosest_col(oldc+underc,COLORS, dither));
	}
	th += underc;
	if (!th->valid)
	{
		th->closest = closestc(oldc+underc,vb.pencel->cmap->ctab,COLORS);
		th->valid = TRUE;
	}
	return(th->closest);
}
static void clip_xy(Short_xy *pp)
{
SHORT i;

	if ((i = pp->x) < 0)
		pp->x = 0;
	else if (i >= vb.pencel->width)
		pp->x = vb.pencel->width-1;

	if ((i = pp->y) < 0)
		pp->y = 0;
	else if (i >= vb.pencel->height)
		pp->y = vb.pencel->height-1;
}

/*********** Reveal alt ink stuff *************************/

Pixel rvl_dot(const Ink *inky, const SHORT x, const SHORT y)
{
if (vl.alt_cel)
	{
	return(rclosest_color(inky->inkdata, vl.alt_cel->cmap->ctab,
							pj_get_dot(vl.alt_cel,x,y), inky->dither));
	}
else
	return(vs.ccolor);
}

void rvl_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
UBYTE sbuf[SBSIZE];
UBYTE *spt;
int w;
Rgb3 *ctab;
Rgb3 *dctab;
Thash *th;
Thash *t;

if (vl.alt_cel)
	{
	ctab = vl.alt_cel->cmap->ctab;
	GET_HSEG(vl.alt_cel,sbuf,x0,y,width);
	spt = sbuf;
	w = width;
	if (inky->dither)
		{
		while (--w >= 0)
			{
			*spt = bclosest_col(ctab+*spt,COLORS,TRUE);
			spt++;
			}
		}
	else
		{
		dctab = vb.pencel->cmap->ctab;
		th = inky->inkdata;
		while (--w >= 0)
			{
			t = th+*spt;
			if (!t->valid)
				{
				t->closest =  closestc(ctab+*spt,dctab,COLORS);
				t->valid = TRUE;
				}
			*spt++ = t->closest;
			}
		}
	PUT_HSEG(vb.pencel,sbuf,x0,y,width);
	}
else
	opq_hline(inky,x0,y,width);
}


/************ xor ink stuff *************************/
Pixel xor_dot(const Ink *inky, const SHORT x, const SHORT y)
{
(void)inky;
return(pj_get_dot(undof,x,y)^vs.ccolor);
}

/************ jumble ink stuff **********************/


Pixel jmb_dot(const Ink *inky, const SHORT x, const SHORT y)
{
SHORT endc;
Short_xy nxy;

endc = inky->strength+1;
nxy.x = x + pj_random()%(endc) - (endc>>1);
nxy.y = y + pj_random()%(endc) - (endc>>1);
clip_xy(&nxy);
return(pj_get_dot(undof,nxy.x,nxy.y));
}

/************ add ink  stuff *************************/
Pixel add_dot(const Ink *inky, const SHORT x, const SHORT y)
{
(void)inky;
return((pj_get_dot(undof,x,y)+vs.ccolor)&(COLORS-1));
}

/************ glow ink stuff *************************/
Pixel glr_dot(const Ink *inky, const SHORT x, const SHORT y)
{
return(((UBYTE *)(inky->inkdata))[pj_get_dot(undof,x,y)]);
}

/*********** crystalize ink stuff ***************/
Pixel cry_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Pixel odot;
int extras;
(void)inky;

	if(x > 0)
	{
		odot = pj__get_dot(undof,x-1,y);
		extras = 0;
	}
	else
		extras = 1;

	if(x < vb.pencel->width - 1)
		odot += pj__get_dot(undof,x+1,y);
	else
		++extras;

	if(y > 0)
		odot += pj__get_dot(undof, x, y-1);
	else
		++extras;

	if(y < vb.pencel->height - 1)
		odot += pj__get_dot(undof, x, y+1);
	else
		++extras;

	if(extras)
		odot += (extras * pj__get_dot(undof,x,y));
	return(odot);
}
void cry_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
Pixel obuf[SBSIZE];
Pixel *out;
Pixel *maxout;
Pixel line[SBSIZE+2];
Pixel *pline;
(void)inky;

	pj__get_hseg(undof,&line[1],x0,y,width);
	line[0] = line[1];
	line[width+1] = line[width];
	if(x0 > 0)
		line[0] = pj__get_dot(undof,x0 - 1,y);
	if(x0 + width < vb.pencel->width)
		line[width+1] = pj__get_dot(undof,x0 + width,y);

	pline = line;
	out = obuf;
	maxout = obuf + width;

	while(out < maxout)
	{
		*out++ = pline[0] + pline[1] + pline[2];
		++pline;
	}

	pline = line+1;
	if(y > 0)
		pj__get_hseg(undof,pline,x0,y-1,width);

	out = obuf;
	while(out < maxout)
		*out++ += *pline++;

	pline = line;
	if(y < vb.pencel->height - 1)
		pj__get_hseg(undof,pline,x0,y+1,width);
	else
		pj__get_hseg(undof,pline,x0,y,width);

	out = obuf;
	while(out < maxout)
		*out++ += *pline++;
	pj__put_hseg(vb.pencel,obuf,x0,y,width);
}

/********** shatter ink stuff ********************/
Pixel shat_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Short_xy nxy;

if (y&1)
	nxy.x = x+inky->strength;
else
	nxy.x = x - inky->strength;
if (nxy.x < 0 || nxy.x >= vb.pencel->width)
	return(inky->aid->tcolor);
else
	return(pj_get_dot(undof,nxy.x,y));
}

/********** hollow ink stuff ***********************/
Pixel out_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Pixel endc;
(void)inky;

endc = pj_get_dot(undof,x,y);
if (endc == pj_get_dot(undof,x+1,y) 
	 && endc == pj_get_dot(undof,x-1,y) 
	 && endc == pj_get_dot(undof,x,y-1) 
	 && endc == pj_get_dot(undof,x,y+1) ) 
	return(vs.inks[0]);
else
	return(endc);
}


/*********** brighten ink stuff *********************/
static Pixel brighten_ccomp(int c, SHORT percent)
{
	c += ((c+4)*percent+75)/100;
	if (c > RGB_MAX-1)
		c = RGB_MAX-1;
	return(c);
}

Pixel bri_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Pixel endc;
Rgb3 rgb, *c;

endc = pj_get_dot(undof,x,y);
c = &(vb.pencel->cmap->ctab[endc]);
rgb.r = brighten_ccomp(c->r, inky->strength);
rgb.g = brighten_ccomp(c->g, inky->strength);
rgb.b = brighten_ccomp(c->b, inky->strength);
return(bclosest_col(&rgb,COLORS,inky->dither));
}

/************ desaturate ink stuff *********************/
static void desaturate(Rgb3 *rgb,Rgb3 *d, SHORT percent)
{
Rgb3 grey;

	grey.r = grey.g = grey.b = (rgb->r + rgb->g + rgb->b + 2)/3;
	true_blend(rgb, &grey, percent, d);
}

Pixel des_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Rgb3 rgb;

desaturate(vb.pencel->cmap->ctab + pj_get_dot(undof,x,y), 
		   &rgb, inky->strength);
return(bclosest_col(&rgb, COLORS,inky->dither));
}

/************* sweep ink stuff ************************/
Pixel swe_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Pixel color, endc;
(void)inky;

color = pj_get_dot(undof,x,y);
endc = pj_get_dot(undof,x,y-1);
if (endc != color)
	{
	if (endc == pj_get_dot(undof, x,y+1) 
				&& endc == pj_get_dot(undof,x+1,y) 
				&& endc == pj_get_dot(undof, x-1,y) )
		color = endc;
	}
return(color);
}

/************* Start of close-hole stuff ********************/

/* this table was made up by hand using eyeballs and intuition.  THere's
   really no algorithm behind it. */
static char chtab[32] = {
	0x0, 0x8, 0x0, 0xcc, 0x2e, 0x8, 0xea, 0xc8,
	0x7f, 0x8, 0x40, 0x0, 0x3a, 0x8, 0x0, 0x0,
	0x74, 0xfc, 0x40, 0xc8, 0x7f, 0x4, 0x46, 0x80,
	0x76, 0x0, 0x40, 0x0, 0x24, 0x0, 0x0, 0x0,
	};

Pixel clh_dot(const Ink *inky, const SHORT x,const SHORT y)
/* this could be a lot faster if it used get_rectpix() for the neighbors */
{
SHORT color;
SHORT nbd;
(void)inky;

color = pj_get_dot(undof,x,y);

/* if pixels already set leave it */
if (color == vs.ccolor)
	return(color);


/* get pixel neighborhood */
nbd = 0;
if ( pj_get_dot(undof,x-1,y-1) == vs.ccolor)
	nbd |= 1;
if ( pj_get_dot(undof,x,y-1) == vs.ccolor)
	nbd |= 2;
if ( pj_get_dot(undof,x+1,y-1) == vs.ccolor)
	nbd |= 4;
if ( pj_get_dot(undof,x-1,y) == vs.ccolor)
	nbd |= 8;
if ( pj_get_dot(undof,x+1,y) == vs.ccolor)
	nbd |= 16;
if ( pj_get_dot(undof,x-1,y+1) == vs.ccolor)
	nbd |= 32;
if ( pj_get_dot(undof,x,y+1) == vs.ccolor)
	nbd |= 64;
if ( pj_get_dot(undof,x+1,y+1) == vs.ccolor)
	nbd |= 128;
if ((0x80>>(nbd&7))&chtab[nbd>>3])
	color = vs.ccolor;
return(color);
}

/*************** emboss ink stuff *********************/
static int emb1c(const long s, const long d, const SHORT strength)
{
int r;

	r = d + ((d-s)*strength+50)/100;
	if (r < 0)
		r = 0;
	if (r > RGB_MAX-1)
		r = RGB_MAX-1;
	return(r);
}

Pixel emb_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Short_xy nxy;
Rgb3 rgb,*c,*c2;

nxy.x = x-1;
nxy.y = y-1;
clip_xy(&nxy);
c = vb.pencel->cmap->ctab;
c2 = c+pj_get_dot(undof,x,y);
c = c+pj_get_dot(undof,nxy.x,nxy.y);
rgb.r = emb1c(c->r, c2->r,inky->strength);
rgb.g = emb1c(c->g, c2->g,inky->strength);
rgb.b = emb1c(c->b, c2->b,inky->strength);
return(bclosest_col(&rgb, COLORS,inky->dither));
}

/************* pull/smear ink stuff ********************/
Pixel pull_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Short_xy nxy;
(void)inky;

nxy.x = x + (icb.lastsx-icb.sx);
nxy.y = y + (icb.lastsy-icb.sy);
clip_xy(&nxy);
return(pj_get_dot(vb.pencel,nxy.x,nxy.y));
}

Pixel smea_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Short_xy nxy;
(void)inky;

nxy.x = x + ((icb.lastsx-icb.sx)+1)/2;
nxy.y = y + ((icb.lastsy-icb.sy)+1)/2;
clip_xy(&nxy);
return(pj_get_dot(undof, nxy.x,nxy.y));
}



