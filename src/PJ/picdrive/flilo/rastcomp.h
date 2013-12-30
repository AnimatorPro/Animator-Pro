#ifndef RASTCOMP_H
#define RASTCOMP_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RASTCALL_H
	#include "rastcall.h"
#endif

/* raster compressors */

#define EMPTY_DCOMP 8  /* sizeof of a FLI_SKIP chunk with no change */

#ifdef RASTCOMP_INTERNALS 
	#define RASType Raster
	#define MAX_RUN 127
	#define INERTIA 4
#else
	#define RASType void /* because many types have same first fields */
#endif

void *flow_brun_rect(RASType *r,void *cbuf,
				SHORT x,SHORT y,USHORT width,USHORT height);

void *pj_lccomp_rects(RASType *r1, void *cbuf, 
				   SHORT x1, SHORT y1, 
				   RASType *r2, 
				   SHORT x2, SHORT y2, USHORT width, USHORT height);

#undef RASType 

#endif /* RASTCOMP_H */
