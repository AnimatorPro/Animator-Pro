
/* screen.c - Routines to maintain a Screen type structure.  Assures
   the pixel data begins on an even paragraph.  */

#include "jimk.h"

free_screen(s)
Vscreen *s;
{
if (s==NULL)
	return;
freemem(s);
}

Vscreen *
alloc_big_screen(w,h)
int w,h;
{
long size;
Vscreen *v;
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
v = (Vscreen *)mem;
mem += sizeof(Vscreen);
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

Vscreen *
alloc_screen()
{
Vscreen *v;

return(alloc_big_screen(XMAX,YMAX) );
}


Vscreen *
clone_screen(s)
Vscreen *s;
{
Vscreen *d;

if ((d = alloc_screen()) != NULL)
	copy_form(s, d);
return(d);
}

/* transform a screen structure into a cel.  Not really much to it. */
screen_to_cel(s, c)
Vscreen *s;
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


