
/* screen.c - stuff to manage a screen data structure.  A screen is
   a byte-a-pixel image buffer plus a color map.  The image buffer
   starts on a paragraph (16 byte) boundary - something some of the
   low level assembler drawing primitives count on. */

#include "jimk.h"

free_screen(s)
Video_form *s;
{
if (s==NULL)
	return;
freemem(s);
}

Video_form *
alloc_big_screen(w,h)
int w,h;
{
long size;
Video_form *v;
char *mem;

size = (long)w * (long)h;
size += 3*COLORS;
size += sizeof(*v);
size += 16;	/* lose to force allignment */
size += 256;	/* make screen same size as compression buffer... */
if ((mem = laskmem(size)) == NULL)
	{
	return(NULL);
	}
/* insure screen starts on paragraph */
v = (Video_form *)mem;
mem += sizeof(Video_form);
v->cmap = (PLANEPTR)mem;
mem += 3*COLORS;
v->p = (PLANEPTR)mem;
v->x = v->y = 0;
v->bpr = v->w = w;
v->h = h;
v->ix = 0;
v->p = norm_pointer(v->p);
v->p = make_ptr(0, ptr_seg(v->p)+1);
return(v);
}

Video_form *
alloc_screen()
{
Video_form *v;

return(alloc_big_screen(XMAX,YMAX) );
}


#ifdef SLUFFED
Video_form *
clone_screen(s)
Video_form *s;
{
Video_form *d;

if ((d = alloc_screen()) != NULL)
	copy_form(s, d);
return(d);
}
#endif  /* SLUFFED */

#ifdef SLUFFED
screen_to_cel(s, c)
Video_form *s;
Vcel *c;
{
c->w = s->w;
c->h = s->h;
c->x = c->y = 0;
c->bpr = s->bpr;
c->p = s->p;
c->cmap = s->cmap;
c->ix = 0;
}
#endif  /* SLUFFED */


