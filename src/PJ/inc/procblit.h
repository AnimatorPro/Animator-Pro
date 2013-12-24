#ifndef PROCBLIT_H
#define PROCBLIT_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif

#ifdef PROCBLIT_INTERNALS
	#define RASType Raster
	#define OPTdata	void *
#else
	#define RASType void
	#define OPTdata ...
#endif /* PROCBLIT_INTERNALS */

/**************** process blit functions ***************/

typedef struct tcolxldat {  /* structure of data for procline functions used
						     * in cel moving etc */
	Pixel tcolor; /* this field MUST be the first one! */
	UBYTE fill[4 - sizeof(Pixel)];
	Pixel *xlat;
} Tcolxldat;

/****** line processing functions and those that use them ****/

typedef void (*Procline)(Pixel *s,Pixel *d,Coor w,void *data);

void ubli_line(Pixel *source_buf, Pixel *dest_buf, 
					   Coor width, const Tcolxldat *tcx);
void pj_tbli_line(Pixel *source_buf, Pixel *dest_buf, 
					   Coor width, const Tcolxldat *tcx);

void ubli_xlatline(Pixel *source_buf, Pixel *dest_buf, 
					   Coor width, const Tcolxldat *tcx);
void tbli_xlatline(Pixel *source_buf, Pixel *dest_buf, 
					   Coor width, const Tcolxldat *tcx);

Errcode procblit(RASType *src, Coor src_x, Coor src_y,
			RASType *dest, Coor dest_x, Coor dest_y, Ucoor width, Ucoor height,
		 	Procline pline, OPTdata );

Errcode abprocblit(RASType *src, Coor src_x, Coor src_y,
			RASType *dest, Coor dest_x, Coor dest_y, Ucoor width, Ucoor height,
		 	RASType *alt, Coor alt_x, Coor alt_y, Procline pline, OPTdata );

Errcode xlatblit(RASType *src, Coor src_x, Coor src_y, RASType *dest,
		  		Coor dest_x, Coor dest_y, Ucoor width, Ucoor height,
				Pixel *ttable );

#undef OPTdata
#undef RASType 

#endif /* PROCBLIT_H */
