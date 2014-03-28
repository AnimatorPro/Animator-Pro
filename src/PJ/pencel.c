#define PENCEL_C

/* pencel.c - Routines to maintain a Rcel type structure. of the current 
 * vb.pencel size and shape a pencel is not garanteed to be a Ramrast */

#include "jimk.h"
#include "errcodes.h"
#include "pentools.h"

Errcode alloc_pencel(Rcel **pcel)

/* allocates a pencel (Rcel) the size of the vb.pencel */
{
	return(valloc_ramcel(pcel,vb.pencel->width,vb.pencel->height));
}
void swap_pencels(Rcel *s, Rcel *d)
/* only works if rcels are of same dimensions and specs */
{
	pj_swaprect(s,0,0,d,0,0,d->width,d->height);
	swap_cmaps(s->cmap, d->cmap);
}
Rcel *clone_pencel(Rcel *s)
/* get copy of an rcel in memory */
{
Rcel *d;

	if((alloc_pencel(&d)) >= Success)
		pj_rcel_copy(s, d);
	return(d);
}

