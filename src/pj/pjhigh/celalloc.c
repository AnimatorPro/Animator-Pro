#include "pjbasics.h"

Errcode valloc_bytemap(Raster **pr, SHORT w, SHORT h)
{
Rasthdr spec;

	copy_rasthdr(vb.cel_a,&spec);
	spec.width = w;
	spec.height = h;
	return(pj_alloc_bytemap(&spec,(Bytemap **)pr));
}

Errcode valloc_ramcel(Rcel **pcel,SHORT w,USHORT h)
/* allocate an Rcel matching specs of current screen of a specifyed size 
 * will only get one from RAM */
{
Rasthdr rastspec;

	copy_rasthdr(vb.cel_a,&rastspec);
	rastspec.width = w;
	rastspec.height = h;
	return(pj_rcel_bytemap_alloc(&rastspec,pcel,vb.cel_a->cmap->num_colors));
}
static void _load_clone(Rcel *s,Rcel *d)
{
	d->x = s->x;
	d->y = s->y;
	d->aspect_dx = s->aspect_dx;
	d->aspect_dy = s->aspect_dy;
	pj_cmap_copy(s->cmap, d->cmap);
	pj_blitrect(s,0,0,d,0,0,d->width,d->height);
}
Rcel *clone_rcel(Rcel *s)
/* Make an identical ram instance of an rcel. including the contents */
{
Rcel *d;

	if((pj_rcel_bytemap_alloc((Rasthdr *)s,&d,s->cmap->num_colors)) >= Success)
		_load_clone(s,d);
	return(d);
}
Rcel *clone_any_rcel(Rcel *s)
/* Make an identical card or ram instance of an rcel. 
 * including the contents */
{
Rcel *d;

	if((alloc_vd_rcel(vb.vd, (Rasthdr *)s, &d,
					  s->cmap->num_colors, FALSE)) < Success)
	{
		if((pj_rcel_bytemap_alloc((Rasthdr *)s,&d,s->cmap->num_colors))<Success)
			return(NULL);
	}
	_load_clone(s,d);
	return(d);
}
Errcode valloc_anycel(Rcel **pcel,SHORT w,USHORT h)

/* allocate an Rcel matching specs of current screen of a specifyed size 
 * from any source available (should be starting with fastest) */
{
Errcode err;
Rcel *cel;
Rasthdr spec;

	spec = *(Rasthdr *)vb.cel_a;
	spec.width = w;
	spec.height = h;

	if((err = alloc_vd_rcel(vb.vd, &spec, pcel,
				    vb.cel_a->cmap->num_colors,FALSE)) < Success)
	{
		if((err = valloc_ramcel(pcel,w,h)) < 0)
			return(err);
	}
	cel = *pcel;
	cel->aspect_dx = vb.cel_a->aspect_dx;
	cel->aspect_dy = vb.cel_a->aspect_dy;
	return(0);
}


