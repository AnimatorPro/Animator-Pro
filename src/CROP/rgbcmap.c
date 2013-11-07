/* rgbcmap.c - Stuff to compute things in rgb space (ie without a color
   map.)  Used much by scaling routines.  Also stuff to quickly find
   the closest color in color map to an arbitrary rgb triple.  This
   file was mostly hacked out of the truecolo.c file in Vpaint. */

#include "jimk.h"
#include "crop.h"
#include "rgbcmap.str"

extern UBYTE bitmasks[];

extern struct bfile rgb_files[3];
extern char *rgb_names[3]; 
int dither;

long
count_hist_18(h)
UBYTE *h;
{
unsigned i;
long acc;
UBYTE b;
UBYTE mask;
int j;

acc = 0;
for (i=0; i<(unsigned)32*(unsigned)1024; i++)
	{
	if ((b = *h++) != 0)
		{
		mask = 0x80;
		j = 8;
		while (--j >= 0)
			{
			if (b&mask)
				acc++;
			mask >>= 1;
			}
		}
	}
return(acc);
}

hist_to_cmap(h, cm)
UBYTE *h;
UBYTE *cm;
{
int r,g,b;
UBYTE c;
UBYTE mask;
int j;

for (r=0; r<64; r++)
	for (g=0; g<64; g++)
		for (b=0; b<64; )
			{
			if ((c = *h++) != 0)
				{
				j = 8;
				mask = 0x80;
				while (--j >= 0)
					{
					if (c&mask)
						{
						cm[0] = r;
						cm[1] = g;
						cm[2] = b;
						cm = norm_pointer(cm+3);
						}
					b++;
					mask >>= 1;
					}
				}
			else
				b += 8;
			}
}

UBYTE *
get_hist()
{
long histosize;
UBYTE *histogram;

histosize = (1L<<15);	/* one bit for each of 256K colors */
if ((histogram = laskmem(histosize)) != NULL)	
	zero_lots(histogram, histosize);
return(histogram);
}

pack_from_hist(dcmap, histogram)
UBYTE *dcmap;
UBYTE *histogram;
{
long cused_count;
UBYTE *rgb;
char buf[60];
extern UBYTE init_cmap[];

cused_count = count_hist_18(histogram);
sprintf(buf, rgbcmap_100 /* "%ld colors in picture" */, cused_count);
stext(buf, 0, 40, sblack, swhite);
if ((rgb = lbegmem(cused_count*3)) != NULL)
	{
	hist_to_cmap(histogram, rgb);
	pack_cmap(rgb,cused_count,dcmap,COLORS-5);	/* leave space for menu colors*/
	copy_bytes(init_cmap+(COLORS-5)*3, dcmap+(COLORS-5)*3, 5*3);
	freemem(rgb);
	}
}

find_rgb_cmap(w, h, dcmap)
int w, h;
UBYTE *dcmap;
{
int ok = 0;
int i, j;
UBYTE *histogram;
long cbit;
unsigned cbyte;
UBYTE *linebufs, *lbs[3];
UBYTE *r,*g,*b;
char buf[60];

if ((linebufs = begmem(w*3)) != NULL)
	{
	/* set up a line buffer to make this a trifle faster */
	for (i=0; i<3; i++)
		lbs[i] = linebufs + i*w;
	if ((histogram = get_hist()) != NULL)
		{
		if (open_rgb_files())
			{
			stext(rgbcmap_101 /* "Making color histogram" */, 
				0, 20, sblack, swhite);
			for (i=0; i<h; i++)
				{
				if (i%10 == 0)
					{
					sprintf(buf, rgbcmap_104 /* "%3d of %d" */, i, h);
					stext(buf, 0, 30, sblack, swhite);
					}
				for (j=0; j<3; j++)
					{
					if (bread(rgb_files+j,lbs[j],w) != w)
						{
						truncated(rgb_names[j]);
						goto BADLINE;
						}
					}
				r = lbs[0];
				g = lbs[1];
				b = lbs[2];
				for (j=0; j<w; j++)
					{
#ifdef SOMETIMES
					cbit = ((long)(*r++&0xfc)<<(12-2))
						 + ((*g++&0xfc)<<(6-2))
						 + ((*b++&0xfc)>>2);
#endif  /* SOMETIMES */
					cbit = ((long)(*r++)<<(12))
						 + ((*g++)<<(6))
						 + ((*b++));
					cbyte = (cbit>>3);
					histogram[cbyte] |= bitmasks[(unsigned)cbit&7];
					}
				}
			if (pack_from_hist(dcmap, histogram))
				ok = 1;
			BADLINE:
			close_rgb_files();
			}
		freemem(histogram);
		}
	freemem(linebufs);
	}
return(ok);
}

cel_with_cmap(w,h)
int w,h;
{
UBYTE *p;
UBYTE *buf;
int i;
int ok = 0;
char err[40];

if ((pic_cel = alloc_cel(w,h,0,0)) != NULL)
	{
	if ((buf = begmem(w*3)) != NULL)
		{
		p = pic_cel->p;
		stext(rgbcmap_103 /* "Fitting to color map" */, 0, 60, sblack, swhite);
		for (i=0; i<h; i++)
			{
			if (i%10 == 0)
				{
				sprintf(err, rgbcmap_104 /* "%3d of %d" */, i, h);
				stext(err, 0, 70, sblack, swhite);
				}
			if (!fit_cel_line(p,w,buf,buf+w,buf+2*w))
				goto BADLOOP;
			p = norm_pointer(p + pic_cel->bpr);
			}
		ok = 1;
		BADLOOP:
		freemem(buf);
		}
	}
return(ok);
}

static
must_rline(buf,ix, w)
UBYTE *buf;
int ix;
int w;
{
if (bread(rgb_files+ix, buf, w) != w)
	{
	truncated(rgb_names[ix]);
	return(0);
	}
return(1);
}


fit_cel_line(d,w,b1,b2,b3)
UBYTE *d;
int w;
UBYTE *b1,*b2,*b3;
{
UBYTE rgb[3];

if (!must_rline(b1,0,w))
	return(0);
if (!must_rline(b2,1,w))
	return(0);
if (!must_rline(b3,2,w))
	return(0);
while (--w >= 0)
	{
	rgb[0] = *b1++;
	rgb[1] = *b2++;
	rgb[2] = *b3++;
	*d++ = bclosest_col(rgb,COLORS);
	}
return(1);
}

struct bhash
	{
	UBYTE valid, r, g, b;
	int closest;
	};
struct bhash *bhash;

#define BSIZ (1024*4*sizeof(struct bhash) )

make_bhash()
{
if ((bhash = begmem(BSIZ)) != NULL)
	{
	zero_structure(bhash, BSIZ);
	return(1);
	}
else
	return(0);
}

free_bhash()
{
gentle_freemem(bhash);
bhash = NULL;
}

#ifdef OLD
bclosest_col(rgb,count)
register UBYTE *rgb;
int count;
{
register struct bhash *h;
int i;
int r,g,b;
UBYTE drgb[3];


/* first look for a hash hit */
i = ((((rgb[0]&0xf)<<8) + ((rgb[1]&0xf)<<4) + ((rgb[2]&0xf))));
h = bhash+i;
if (h->valid && h->r == rgb[0] && h->g == rgb[1] && h->b == rgb[2] )
	{
	goto GOTIT;
	}
h->closest = closestc(rgb,vf.cmap,count);
h->r = rgb[0];
h->g = rgb[1];
h->b = rgb[2];
h->valid = 1;
GOTIT:
return(h->closest);
}
#endif  /* OLD */

int rerr,gerr,berr;

#ifdef SLUFFED
flush_dither_err()
{
rerr = gerr = berr = 0;
}
#endif  /* SLUFFED */

bclosest_col(rgb,count)
register UBYTE *rgb;
int count;
{
register struct bhash *h;
int i;
int r,g,b;
UBYTE drgb[3];

if (dither)
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
h->closest = closestc(rgb,sys_cmap,count);
h->r = rgb[0];
h->g = rgb[1];
h->b = rgb[2];
h->valid = 1;
GOTIT:
if (dither)
	{
	rgb = sys_cmap + 3*h->closest;
	rerr = 3*(r - rgb[0])/4;
	gerr = 3*(g - rgb[1])/4;
	berr = 3*(b - rgb[2])/4;
	}
return(h->closest);
}
