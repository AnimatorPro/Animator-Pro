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

struct rgb3;

void *pj_unbrun_scale_line(BYTE *src, Coor sw, BYTE *dst, SHORT *xtable);

extern void
pj_unbrun_scale_rect(RASType *dst, void *ucbuf,
		USHORT sw, USHORT sh, SHORT dx, SHORT dy, USHORT dw, USHORT dh);

void *pj_unbrun_skip_line(BYTE *src, Coor sw);

extern void *pj_brun_comp_line(BYTE *src, BYTE *cbuf, int count);

void *pj_brun_rect(RASType *r,void *cbuf,
				SHORT x,SHORT y,USHORT width,USHORT height);

extern void *
pj_fccomp(struct rgb3 *last_ctab, struct rgb3 *this_ctab,
		void *cbuf, unsigned int count);

void *pj_lccomp_rects(RASType *r1, void *cbuf, 
				   SHORT x1, SHORT y1, 
				   RASType *r2, 
				   SHORT x2, SHORT y2, USHORT width, USHORT height);

void *pj_ss2_rects(RASType *r1, void *cbuf, 
				SHORT x1, SHORT y1, 
				RASType *r2, 
				SHORT x2, SHORT y2, USHORT width, USHORT height);

#undef RASType 

#endif /* RASTCOMP_H */
