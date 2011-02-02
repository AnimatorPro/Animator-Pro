/* gfx.c - low level PC - VGA graphics routines.  Some fixed point
   arithmetic helpers too. */

/* Some VGA specific 320x200x256 graphics routines. */

#include "jimk.h"

/* a table for RIF and ANIM decompression */
unsigned WORD ytable[YMAX];

/* Poke the background color with an rgb value.  Handy for debugging
   sometimes. */
poke_bg(p)
char *p;
{
jset_colors(0,1,p);
}

/* Set the background color back to what it should be. */
restore_bg()
{
jset_colors(0,1,sys_cmap);
}

/* Wait for Vsync and feed hardware the current color palette */
see_cmap()
{
wait_sync();
jset_colors(0, COLORS, vf.cmap);
}

/* Flash the background color briefly. */
flash_bg(p)
char *p;
{
poke_bg(p);
wait_a_jiffy(4);
restore_bg();
}


#ifdef SLUFFED
flash_green()
{
flash_bg(pure_green);
}
#endif /*  SLUFFED  */


flash_red()
{
flash_bg(pure_red);
}


#ifdef SLUFFED
flash_blue()
{
flash_bg(pure_blue);
}
#endif /* SLUFFED */


copy_form(s, d)
Video_form *s, *d;
{
copy_words(s->p, d->p, 32000);
copy_cmap(s->cmap, d->cmap);
}

#ifdef SLUFFED
exchange_form(s, d)
Video_form *s, *d;
{
exchange_words(s->p, d->p, 32000);
exchange_words(s->cmap, d->cmap, COLORS/2*3);
}
#endif /* SLUFFED */

#ifdef SLUFFED
/* Set a screen sized form to a solid color */
color_form(f,color)
Video_form *f;
WORD color;
{
color += (color<<8);		/* Do it 2 bytes at a time */
stuff_words(color, f->p, f->bpr*f->h/sizeof(WORD));
}
#endif /* SLUFFED */

zero_bytes(pt, count)
register char *pt;
int count;
{
while (--count >= 0)
	*pt++ = 0;
}

copy_lots(s, d, count)
register long *s, *d;
register long count;
{
count /= sizeof(long);
while (--count >= 0)
	*d++ = *s++;
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
Video_form *f;
{
zero_lots(f->p, (long)f->bpr*f->h);
}


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

#ifdef SLUFFED
/* Little routine I should put into assembler someday.  Does x*p/q on
   some ints without blowing up if x*p is > 64K provided q will bring it
   back into range. Sscale_by if for 'signed scale by' */
sscale_by(x,p,q)
int x,p,q;
{
long l;

l = x;
l *= p;
l /= q;
return(l);
}
#endif /* SLUFFED */

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

itmult(trig, x)
WORD trig,x;
{
long result;

result = trig;
result *= x;
return(result/(1<<14));
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


