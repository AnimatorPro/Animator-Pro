
#ifndef RENDER_H
#define RENDER_H

#ifndef BHASH_H
	#include "bhash.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef POLY_H
	#include "poly.h"
#endif

typedef struct renderdata {
	Cliprect cr;			/* the current render form clip rectangle */

	/* color gradient rectangle data */

    SHORT rdx0,rdy0,rdx1,rdy1; /* gradient range min and max rect */
	SHORT rwidth,rheight;  /* gradient range width and height */
} Rendata;

extern Rendata rdta;

/* Hash for transparent color */
typedef struct thash {
	UBYTE valid, closest;
} Thash;

extern Rgb3 tcolor_src;

Errcode rblit_cel(register Rcel *c, Tcolxldat *txd);

Errcode render_blit(Rcel *src, SHORT sx,SHORT sy, Rcel *dest, 
					SHORT dx,SHORT dy,SHORT w,SHORT h, 
					Tcolxldat *txd, Cmap *scmap);

void render_mask_blit(UBYTE *mplane, SHORT mbpr,
					  SHORT mx, SHORT my,
					  void *drast, /* currently ignored uses vb.pencel */
					  SHORT rx, SHORT ry, USHORT width, USHORT height, ... );

Errcode render_transform(Rcel *cel, struct xformspec *xf
, struct tcolxldat *txd);

void render_dot(SHORT x,SHORT y);
Errcode transpblit(register Rcel *tcel,int clearcolor,int clear,int tinting);

Errcode render_hline(register SHORT y,register SHORT x0,SHORT x1,Raster *r);
Errcode poll_render_hline(SHORT y,SHORT x0,SHORT x1,Raster *r);

Errcode render_a_poly_or_spline(struct poly *wpoly, Boolean filled,
	Boolean closed, Boolean curved);
Errcode render_opoly(Poly *p, Boolean closed);

void render_mask_alpha_blit(UBYTE *alpha, int abpr, int x, int y, int w, int h
, Rcel *r, Pixel oncolor);

#endif /* RENDER_H */

