
#include "jimk.h"
#include "shrinkin.str"

extern int gif_line;
extern Video_form *gif_form;

shrink_screen(s, d)
Video_form *s, *d;
{
int w,h;
UBYTE *spt, *dpt;
register UBYTE *ss, *dd;

clear_form(d);
spt = s->p;
dpt = d->p;
w = d->w;
while (--w >= 0)
	{
	stretch_ave_line(spt++, dpt++, s->h, d->h, s->bpr, d->bpr, d->cmap,0);
	}
}


expand_cmap(cmap, colors)
UBYTE *cmap;
int colors;
{
int i,j,k,divi,div2;
UBYTE *c1, *c2, *out;

if (colors > 16)
	return;
out = cmap+colors*3;
if (colors == 2)
	divi = 128;
else
	divi = COLORS/((colors * (colors-1))/2);
div2 = divi/2;
for (i=0; i<colors; i++)
	{
	c1 = cmap+i*3;
	for (j=i+1; j<colors; j++)
		{
		c2 = cmap+j*3;
		for (k=1; k<divi; k++)
			{
			*out++ = (c2[0]*k + c1[0]*(divi-k) + div2)/divi;
			*out++ = (c2[1]*k + c1[1]*(divi-k) + div2)/divi;
			*out++ = (c2[2]*k + c1[2]*(divi-k) + div2)/divi;
			if (out > cmap+3*COLORS)
				{
				goto ENOUGH;
				}
			}
		}
	}
ENOUGH:
return;
}


stretch_ave_line(s, d, sw, dw, snext, dnext, cmap,show)
UBYTE *s, *d, *cmap;
WORD sw,dw;
int snext, dnext;
int show;
{
int w;
int i;
int scalefac;
int r,g,b;
unsigned char *cm;
unsigned char rgb[3];

if (show && (gif_line&15)==0)
	{
	char buf[40];
	sprintf(buf, shrinkin_100 /* "line %d of %d" */, gif_line+1, gif_form->h);
	stext(buf, 0, 0, swhite, sblack);
	}


if (sw <= dw)
	{
	d = long_to_pt(pt_to_long(d) + (long)(((int)dw - sw)/2)*dnext);
	while (--sw >= 0)
		{
		*d = *s;
		d = norm_pointer(d+dnext);
		s = norm_pointer(s+snext);
		}
	}
else
	{
	scalefac = (sw+dw-1)/dw;
	w = sw/scalefac;
	while (--w >= 0)
		{
		r = g = b = 0;
		i = scalefac;
		while (--i >= 0)
			{
			cm = cmap + 3 * s[0];
			r += *cm++;
			g += *cm++;
			b += *cm++;
			s = norm_pointer(s + snext);
			}
		rgb[0] = (r+scalefac/2)/scalefac;
		rgb[1] = (g+scalefac/2)/scalefac;
		rgb[2] = (b+scalefac/2)/scalefac;
		d[0] = bclosest_col(rgb, COLORS);
		d = norm_pointer(d+dnext);
		}
	}
}
