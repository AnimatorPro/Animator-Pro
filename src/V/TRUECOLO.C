
/* truecolo.c - stuff to help me pretend that my pixels are RGB values
   and not an index into the color map.  */

#include "jimk.h"
#include "inks.h"

extern WORD quant_lx, quant_ly;

static int got_hash;

struct thash
	{
	UBYTE valid, closest;
	};
struct bhash
	{
	UBYTE valid, r, g, b;
	int closest;
	};
static struct thash *thash;
static struct bhash *bhash;
static UBYTE tcolor[3];


#define BSIZ (1024*4*sizeof(struct bhash) )

make_bhash()
{
flush_dither_err();
if ((bhash = begmemc(BSIZ)) != NULL)
	{
	return(1);
	}
else
	return(0);
}

is_bhash()
{
return(bhash!=NULL);
}


static
free_thash()
{
gentle_freemem(thash);
thash = NULL;
}


static
make_thash()
{
unsigned size;

if (vs.dither)
	return(make_bhash());
size = COLORS*sizeof(*thash);
if ((thash = begmemc(size)) != NULL)
	{
	return(1);
	}
else
	return(0);
}

cycle_ccolor()
{
vs.cdraw_ix++;
if (vs.cdraw_ix >= cluster_count())
	vs.cdraw_ix = 0;
vs.ccolor = vs.buns[vs.use_bun].bundle[vs.cdraw_ix];
if (vs.draw_mode == I_GLAZE || vs.draw_mode == I_GLASS)
	{
	if (got_hash)
		{
		free_render_cashes();
		make_render_cashes();
		}
	}
}

make_render_cashes()
{

got_hash = 1;
switch (vs.draw_mode)
	{
	case 4:	/* transparent */
	case 6:
		copy_bytes(vs.ccolor*3+render_form->cmap, tcolor, 3);
		return(make_thash());
	case 5:	/* defocus */
	case 15: /* antialias */
	case 17: /* brighten */
	case 18: /* desaturate */
	case 22: /* emboss */
		return(make_bhash());
	case 7:	/* alt screen source */
	case 12: /* cel pattern */
		return(make_thash());
	case 21: /* darken */
		tcolor[0] = tcolor[1] = tcolor[2] = 0;
		return(make_thash());
	case 1:	/* vspread */
	case 2:	/* h spread */
	case 3:	/* line spread */
	case 25:	/* rad spread */
		vs.randseed = 0;
	case 11: /* glow */
		return(make_glow());
	}
return(1);
}

UBYTE *glow_lookup;

static
make_glow()
{
int i;
UBYTE *bun, c, c1;

if ((glow_lookup = begmem(COLORS))== NULL)
	return(0);
for (i=0;i<COLORS;i++)
	glow_lookup[i]=i;
i = vs.buns[vs.use_bun].bun_count;
bun = vs.buns[vs.use_bun].bundle;
c1 = *bun;
while (--i > 0)
	{
	c = *bun++;
	glow_lookup[c] = *bun;
	}
glow_lookup[*bun] = c1;
return(1);
}

free_bhash()
{
gentle_freemem(bhash);
bhash = NULL;
}


free_render_cashes()
{
free_thash();
free_bhash();
gentle_freemem(glow_lookup);
glow_lookup = NULL;
got_hash = 0;
}


#ifdef CCODE	
/* replaced by colorave() in colorave.asm */
color_ave(x,y,rgb)
int x, y;
UBYTE *rgb;
{
int i,j, tmp, cx, cy;
UBYTE d[9*3];

/* unpack the neighborhood into rbg values */
tmp = 0;
for (j=y-1; j<=y+1; j++)
	{
	if (j < 0)
		cy = 0;
	else if (j > vf.h-1)
		cy = vf.h-1;
	else
		cy = j;
	for (i=x-1; i<=x+1; i++)
		{
		copy_bytes(render_form->cmap+getd(blurp,i,cy)*3, d+tmp, 3);
		tmp += 3;
		}
	}
*rgb++ = (d[0] + 2*d[3] + d[6] + 2*d[9] + 4*d[12] + 2*d[15]
	+ d[18] + 2*d[21] + d[24])>>4;
*rgb++ = (d[1] + 2*d[4] + d[7] + 2*d[10] + 4*d[13] + 2*d[16]
	+ d[19] + 2*d[22] + d[25])>>4;
*rgb++ = (d[2] + 2*d[5] + d[8] + 2*d[11] + 4*d[14] + 2*d[17]
	+ d[20] + 2*d[23] + d[26])>>4;
}
#endif /* CCODE */

color_dif(rgb,c)
BYTE *rgb, *c;
{
register int dc, dif;

dc = rgb[0]-c[0];
dif = dc*dc;
dc = rgb[1]-c[1];
dif += dc*dc;
dc = rgb[2]-c[2];
dif += dc*dc;
return(dif);
}



#ifdef CCODE
c_closestc(rgb,c)
BYTE *rgb;
BYTE *c;
{
long closest_dif, dif;
WORD dc;
register int i;
int closest;

closest_dif = 62000L;	/* arbitrary huge number */
for (i=0; i<COLORS; i++)
	{
	dc = rgb[0]-c[0];
	dif = dc*dc;
	dc = rgb[1]-c[1];
	dif += dc*dc;
	dc = rgb[2]-c[2];
	dif += dc*dc;
	if (dif < closest_dif)
		{
		closest_dif = dif;
		closest = i;
		}
	c += 3;
	}
return(closest);
}
#endif /* CCODE */

static int rerr,gerr,berr;

static 
flush_dither_err()
{
rerr = gerr = berr = 0;
}

/* bclosest_col - find closest color in color map to a true color value,
   using a cashe.  Has some problems with sign extension on Microsoft
   C.  */
bclosest_col(rgb,count)
register UBYTE *rgb;
int count;
{
register struct bhash *h;
int i;
int r,g,b;
UBYTE drgb[3];

if (vs.dither)
	{
	register int temp;

	temp = rgb[0] + rerr;
	if (temp < 0)
		temp = 0;
	if (temp > 63)
		temp = 63;
	drgb[0] = r = temp;
	temp = rgb[1] + gerr;
	if (temp < 0)
		temp = 0;
	if (temp > 63)
		temp = 63;
	drgb[1] = g = temp;
	temp = rgb[2] + berr;
	if (temp < 0)
		temp = 0;
	if (temp > 63)
		temp = 63;
	drgb[2] = b = temp;
	rgb = drgb;
	}

/* first look for a hash hit */
i = ((((rgb[0]&0xf)<<8) + ((rgb[1]&0xf)<<4) + ((rgb[2]&0xf))));
h = bhash+i;
if (h->valid && h->r == rgb[0] && h->g == rgb[1] && h->b == rgb[2] )
	{
	goto GOTIT;
	}
h->closest = closestc(rgb,render_form->cmap,count);
h->r = rgb[0];
h->g = rgb[1];
h->b = rgb[2];
h->valid = 1;
GOTIT:
if (vs.dither)
	{
	rgb = render_form->cmap + 3*h->closest;
	rerr = 3*(r - rgb[0])/4;
	gerr = 3*(g - rgb[1])/4;
	berr = 3*(b - rgb[2])/4;
	}
return(h->closest);
}

desaturate(rgb, d)
UBYTE *rgb, *d;
{
UBYTE grey[3];

grey[0] = grey[1] = grey[2] = (rgb[0] + rgb[1] + rgb[2] + 2)/3;
true_blend(rgb, grey, vs.tint_percent, d);
}

brighten_ccomp(c)
WORD c;
{
c += ((c+4)*vs.tint_percent+75)/100;
if (c > 63)
	c = 63;
return(c);
}

rclosest_color(oldc, underc)
UBYTE *oldc;
WORD underc;
{
register struct thash *t;

if (vs.dither)
	{
	return(bclosest_col(oldc+3*underc,COLORS));
	}
t = thash+underc;
if (!t->valid)
	{
	t->closest = closestc(oldc+3*underc,render_form->cmap,COLORS);
	t->valid = 1;
	}
return(t->closest);
}

tclosest_color(underc)
WORD underc;
{
register struct thash *t;
UBYTE rgb[3];

if (vs.dither)
	{
	true_blend(render_form->cmap+3*underc, 
		tcolor, vs.tint_percent, rgb);
	return(bclosest_col(rgb,COLORS));
	}
t = thash+underc;
if (!t->valid)
	{
	true_blend(render_form->cmap+3*underc, 
		tcolor, vs.tint_percent, rgb);
	t->closest = closestc(rgb,render_form->cmap,COLORS);
	t->valid = 1;
	}
return(t->closest);
}

tccolor(dcolor, tcolor, tintper, t)
WORD dcolor;
WORD tcolor;
WORD tintper;
register struct thash *t;
{
t += dcolor;
if (!t->valid)
	{
	UBYTE rgb[3];
	PLANEPTR cm;

	cm = render_form->cmap;
	true_blend(cm+3*dcolor, cm+3*tcolor, tintper, rgb);
	t->closest = closestc(rgb,cm,COLORS);
	t->valid = 1;
	}
return(t->closest);
}

true_blend(c1, c2, percent, d)
UBYTE *c1,*c2, *d;
UBYTE percent;
{
UBYTE vpercent;

vpercent = 100-percent;
*d++ = (*c1++ * vpercent + *c2++ * percent + 50)/100;
*d++ = (*c1++ * vpercent + *c2++ * percent + 50)/100;
*d++ = (*c1++ * vpercent + *c2++ * percent + 50)/100;
}

true_blends(c1,c2,p,q,d,count)
UBYTE *c1,*c2,*d;
int p,q;
int count;
{
long vp;
long lp,lq2;
long lq;

lq = q;
lp = p;
vp = lq - lp;
lq2 = lq/2;

while (--count >= 0)
	{
	*d++ = (*c1++ * vp + *c2++ * lp + lq2)/lq;
	*d++ = (*c1++ * vp + *c2++ * lp + lq2)/lq;
	*d++ = (*c1++ * vp + *c2++ * lp + lq2)/lq;
	}
}


true_fades(c1,rgb,p,q,d,count)
UBYTE *c1,*rgb,*d;
int p,q;
int count;
{
long vp;
long lp,lq2;
long lq;

lq = q;
lp = p;
vp = lq - lp;
lq2 = lq/2;
while (--count >= 0)
	{
	*d++ = (*c1++ * vp + rgb[0] * lp + lq2)/lq;
	*d++ = (*c1++ * vp + rgb[1] * lp + lq2)/lq;
	*d++ = (*c1++ * vp + rgb[2] * lp + lq2)/lq;
	}
}


nz_fitting_cmap(scm, dcm, ctable)
PLANEPTR scm, dcm, ctable;
{
int oz;

oz = vs.zero_clear;
vs.zero_clear = 0;
fitting_cmap(scm, dcm, ctable);
vs.zero_clear = oz;
}

fitting_cmap(scm, dcm, ctable)
PLANEPTR scm,dcm,ctable;
{
int i, clearc;
PLANEPTR zpt;
UBYTE save_rgb[3];
UBYTE c;

zpt = dcm+3*vs.inks[0];
copy_bytes(zpt, save_rgb, 3);
if (vs.zero_clear)
	{
	clearc = vs.inks[0];
	}
else
	clearc = -1;
for (i=0; i<COLORS; i++)
	{
	/* clear color always maps to itself! */
	if (i == clearc)
		*ctable++ = i;
	else
		{
		c = closestc(scm, dcm, COLORS);
		/* if clear color is closest force it to something else */
		if (c == clearc)
			{
			zpt[0] ^= 0x20;
			zpt[1] ^= 0x20;
			zpt[2] ^= 0x20;
			c = closestc(scm, dcm, COLORS);
			copy_bytes(save_rgb, zpt, 3);
			}
		*ctable++ = c;
		}
	scm+=3;
	}
}

#ifdef LATER
fitting_cmap(scm, dcm, ctable)
PLANEPTR scm,dcm,ctable;
{
int i;

i = COLORS;
/* zero always maps to zero! */
*ctable++ = 0;
scm += 3;
dcm += 3;
while (--i > 0)
	{
	*ctable++ = closestc(scm, dcm, COLORS-1)+1;
	scm+=3;
	}
}
#endif LATER


static
shuffle_cmap(s1,s2,d)
PLANEPTR s1,s2,d;
{
int i;
int dsize;
PLANEPTR p;

p = d;
dsize = 0;
i = COLORS;
while (--i >= 0)
	{
	if (!in_cmap(s1, d, dsize) )
		{
		copy_bytes(s1, p, 3);
		p += 3;
		dsize++;
		}
	s1 += 3;
	}
i = COLORS;
while (--i >= 0)
	{
	if (!in_cmap(s2, d, dsize) )
		{
		copy_bytes(s2, p, 3);
		p += 3;
		dsize++;
		}
	s2 += 3;
	}
return(dsize);
}

compromise_cmap(s1,s2,d)
PLANEPTR s1,s2,d;
{
int i;
PLANEPTR ccmap;
long ccount;

if ((ccmap = begmem(2*3*COLORS)) == NULL)
	return(0);
copy_cmap(s1,ccmap);	/* just in case... */
ccount = shuffle_cmap(s1,s2,ccmap);
pack_cmap(ccmap,ccount,d,COLORS);
freemem(ccmap);
}

