#ifndef RCEL_H
#define RCEL_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RECTANG_H
	#include "rectang.h"
#endif

#ifndef RASTCALL_H
	#include "rastcall.h"
#endif

#ifndef CMAP_H
	#include "cmap.h"
#endif

/* a minimal drawing cel designed to be compatible with a window and 
 * raster and window screen but smaller
 * and less complicated for use as a memory rendering area and image 
 * processing buffer.  It is a raster with a few extra fields added 
 * that are common to a window and window screen.
 * It would be a lot easier to maintain things if windows and cels
 * and screens used pointers to rasters but in the interest of speed
 * the raster and it's library are full fields. and common fields 
 * are maintained in common positions.  In this way all the items
 * can be used in with the raster library protocall and be processed
 * with the same rendering routines */

#define CEL_FIELDS \
 	Cmap *cmap

typedef struct rcel {
	RASTHDR_FIELDS;  /* the raster for raster library */
	Rastbody hw;
	CEL_FIELDS;
} Rcel;

/* items found in gfxlib.lib on rex side */

void pj_rcel_close(Rcel *rc);
void pj_rcel_free(Rcel *c);

Errcode pj_rcel_bytemap_open(Rasthdr *spec,Rcel *cel,LONG num_colors);
Errcode pj_rcel_bytemap_alloc(Rasthdr *spec,Rcel **pcel,LONG num_colors);


/* do not use close_rcel() or free_rcel() on results of make_virtual_rcel() 
 * no cleanup necessary */

Boolean pj_rcel_make_virtual(Rcel *rc, Rcel *root, Rectangle *toclip);





#endif /* RCEL_H */
