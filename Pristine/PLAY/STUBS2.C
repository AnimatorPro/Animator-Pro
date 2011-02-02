#include "jimk.h"


struct bhash
	{
	UBYTE valid, r, g, b;
	int closest;
	};
struct thash *thash;
struct bhash *bhash;
UBYTE tcolor[3];


#define BSIZ (1024*4*sizeof(struct bhash) )


make_bhash()
{
flush_dither_err();
if ((bhash = begmem(BSIZ)) != NULL)
	{
	zero_structure(bhash, BSIZ);
	return(1);
	}
else
	return(0);
}


int rerr,gerr,berr;

flush_dither_err()
{
rerr = gerr = berr = 0;
}

free_bhash()
{
gentle_freemem(bhash);
bhash = NULL;
}


long 
pt_to_long(offset, seg)
unsigned offset, seg;
{
long result;

result = seg;
result <<= 4;
result += offset;
return(result);
}

void *
long_to_pt(l)
unsigned long l;
{
unsigned segment, offset;

offset = (l&15);
l >>= 4;
segment = l;
return(make_ptr(offset, segment));
}



bclosest_col(rgb,count)
register UBYTE *rgb;
int count;
{
register struct bhash *h;
int i;
int r,g,b;
UBYTE drgb[3];

if (0)   /*  if (vs.dither)  */
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
h->closest = closestc(rgb,vf.cmap,count);
h->r = rgb[0];
h->g = rgb[1];
h->b = rgb[2];
h->valid = 1;
GOTIT:

if (0)  /*  if (vs.dither) */
	{
	rgb = vf.cmap + 3*h->closest;
	rerr = 3*(r - rgb[0])/4;
	gerr = 3*(g - rgb[1])/4;
	berr = 3*(b - rgb[2])/4;
	}
return(h->closest);
}


/******** screen ******/
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
