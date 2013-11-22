
/* gfx.c - low level PC - VGA graphics routines.  Some fixed point
   arithmetic helpers too. */

#include "jimk.h"


#ifdef SLUFFED
poke_bg(p)
char *p;
{
jset_colors(0,1,p);
}

restore_bg()
{
jset_colors(0,1,sys_cmap);
}

flash_bg(p)
char *p;
{
poke_bg(p);
wait_a_jiffy(4);
restore_bg();
}


flash_green()
{
flash_bg(pure_green);
}


flash_red()
{
flash_bg(pure_red);
}

flash_blue()
{
flash_bg(pure_blue);
}
#endif SLUFFED

see_cmap()
{
wait_sync();
jset_colors(0, COLORS, render_form->cmap);
}


copy_form(s, d)
Vscreen *s, *d;
{
copy_words(s->p, d->p, 32000);
copy_cmap(s->cmap, d->cmap);
}

xor_form(s,d)
Vscreen *s, *d;
{
xor_group(s->p, d->p, 32000);
}

exchange_form(s, d)
Vscreen *s, *d;
{
exchange_words(s->p, d->p, 32000);
exchange_words(s->cmap, d->cmap, COLORS/2*3);
}

color_form(f,color)
Vscreen *f;
WORD color;
{
color += (color<<8);
stuff_words(color, f->p, f->bpr*f->h/sizeof(WORD));
}

zero_lots(pt, size)
register char *pt;
long size;
{
int lsize;

size >>=1;	/* convert to word count */
while (size > 0)
	{
	if (size > 32000)
		lsize = 32000;
	else
		lsize = size;
	stuff_words(0, pt, lsize);
	pt = norm_pointer(pt+lsize);
	pt = norm_pointer(pt+lsize);
	size -= lsize;
	}
}

clear_form(f)
Vscreen *f;
{
zero_lots(f->p, (long)f->bpr*f->h);
}

#ifdef SLUFFED
/* clear form and restore color map to initial */
cclear_form(f)
Vscreen *f;
{
clear_form(f);
copy_structure(init_cmap, f->cmap, COLORS*3);
}
#endif SLUFFED


draw_frame(color, x0, y0, x1, y1)
int color, x0, y0, x1, y1;
{
int w, h;

w = x1-x0;
h = y1-y0-2;
chli(vf.p, x0, y0, w, color);
chli(vf.p, x0, y1-1, w, color);
cvli(vf.p, x0, y0+1, h, color);
cvli(vf.p, x1-1, y0+1, h, color);
}

sscale_by(x,p,q)
int x,p,q;
{
long l;

l = x;
l *= p;
l /= q;
return(l);
}

uscale_by(x, p, q)
unsigned x, p, q;
{
long l;

l = x;
l *= p;
l /= q;
return(l);
}

/* round and scale */
rscale_by(x,p,q)
int x,p,q;
{
long l;
int sign;

sign = 1;
if (x < 0)
	sign = -sign;
if (p < 0)
	sign = -sign;
l = x;
l *= p;
if (sign > 0)
	l += (q/2);
else
	l -= (q/2);
l /= q;
return(l);
}

/* count number of non-zero elements in a byte array */
long
count_cused(c, i)
register PLANEPTR c;
register long i;
{
long acc;

acc = 0;
while (--i >= 0)
	{
	if (*c++)
		acc++;
	}
return(acc);
}

make_cused(p, c)
PLANEPTR p,c;
{
unsigned i;

zero_structure(c, COLORS);
i = 64000;
for (;;)
        {
        if (i == 0)
                break;
        c[*p++] |= 1;
        --i;
        }
}

exchange_bytes(a, b, count)
char *a, *b;
int count;
{
char swap;

while (--count >= 0)
	{
	swap = *a;
	*a++ = *b;
	*b++ = swap;
	}
}
