#ifndef GFX_H
#define GFX_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef CMAP_H
	#include "cmap.h"
#endif

#ifndef RASTCALL_H
	#include "rastcall.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif



void pj_cmap_load(void *rast, Cmap *cmap);
void pj_cmap_copy(Cmap *s,Cmap *d);

/* Set a horizontal line on a bitplane.  No clipping, and x1 better
 * be less than or equal to x2! */
void set_bit_hline(unsigned char *buf, 
	unsigned int bpr, unsigned int y, unsigned int x1, unsigned int x2);
extern unsigned char bit_masks[8];

#endif /* GFX_H */
