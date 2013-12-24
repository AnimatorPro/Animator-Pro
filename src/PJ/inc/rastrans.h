#ifndef RASTRANS_H
#define RASTRANS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef VERTICES_H
	#include "vertices.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

typedef struct min_max {
	FRECT_FIELDS;
	SHORT ymin_ix;
	SHORT ymax_ix;
} Min_max;

typedef struct xformspec {
	Short_xy bpoly[4];  /* bounding poly of transformed cel */
	Min_max mmax;       /* current bpoly minmax */
	Min_max ommax;      /* previously rendered minmax */
} Xformspec;

void init_xformspec(Xformspec *xf);
void load_rect_minmax(Rectangle *rect,Xformspec *xf);
void load_poly_minmax(Xformspec *xf);
Boolean isin_bpoly(Xformspec *xf,Rcel *src_cel,SHORT x,SHORT y);

Errcode raster_transform(Rcel *src_cel,Rcel *dscreen,Xformspec *xf,
						 Errcode (*putline)(void *plinedat, Pixel *line,
						 					Coor x, Coor y, Ucoor width), 
						 void *plinedat,
						 Boolean erase_last,
#ifdef RASTRANS_C /*******************************************/
						 /* these are only needed if erase_last is true */

						 void (*undraw_line)(Coor x, Coor y, Ucoor width, 
											 void *edat),
						 void (*undraw_rect)(Coor x, Coor y,
						 					 Ucoor width, Ucoor height,
											 void *edat), 
						 void *edat);
#else /*******************************************/
						 ...);
#endif /* RASTRANS_C ******************************/

#endif /* RASTRANS_H */
