#ifndef GFXLIB_H
#define GFXLIB_H


#ifndef REXLIB_H
	#include "rexlib.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#define RASType void

typedef struct gfxlib
	{
	Libhead hdr;

	/* raster library calls not accessed through direct jumps or part of
	 * other raster library calls */

	Errcode (*alloc_bytemap)(Rasthdr *spec,Bytemap **pbm);
	Errcode (*alloc_bitmap)(Rasthdr *spec,Bitmap **pbm);

	Errcode (*open_temprast)(Bytemap *r,SHORT width,SHORT height,
						     SHORT pdepth);

	Boolean (*pj_clipbox_make)(Clipbox *cb, RASType *r, 
					  	    Coor x,Coor y,Coor width,Coor height);

	void (*close_raster)(RASType *r);
	void (*pj_rast_free)(RASType *r);

	void (*put_hseg)(RASType *r, void *pixbuf, Coor x, Coor y, Ucoor w);
	void (*get_hseg)(RASType *r, void *pixbuf, Coor x, Coor y, Ucoor w);
	void (*put_vseg)(RASType *r, void *pixbuf, Coor x, Coor y, Ucoor h);
	void (*get_vseg)(RASType *r, void *pixbuf, Coor x, Coor y, Ucoor h);

	void (*put_rectpix)(RASType *r,void *pixbuf,Coor x,Coor y,Ucoor w,Ucoor h);
	void (*get_rectpix)(RASType *r,void *pixbuf,Coor x,Coor y,Ucoor w,Ucoor h);

	void (*set_hline)(RASType *r,Pixel color,Coor x,Coor y,Ucoor w);
	void (*set_vline)(RASType *r,Pixel color,Coor x,Coor y,Ucoor h);
	void (*pj_clear_rast)(RASType *r);
	void (*set_rect)(RASType *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
	void (*xor_rect)(RASType *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
	void (*mask1blit)(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
				   RASType *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
				   Pixel oncolor );
	void (*mask2blit)(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
				   RASType *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
				   Pixel oncolor, Pixel offcolor );
	/* binary calls */

	void (*xor_rast)(RASType *s, RASType *d);

	void (*_blitrect)(RASType *source, Coor src_x, Coor src_y,
				   RASType *dest, Coor dest_x, Coor dest_y,
				   Coor width, Coor height);

	void (*blitrect)(RASType *source, Coor src_x, Coor src_y,
				  RASType *dest, Coor dest_x, Coor dest_y,
				  Coor width, Coor height);

	void (*_tblitrect)(RASType *s, Coor sx, Coor sy,
				    RASType *d, Coor dx, Coor dy, Coor width, Coor height,
				    Pixel tcolor );

	void (*tblitrect)(RASType *s, Coor sx, Coor sy,
				   RASType *d, Coor dx, Coor dy, Coor width, Coor height,
				   Pixel tcolor );

	void (*_swaprect)(RASType *ra, Coor ax, Coor ay,
			  	   RASType *rb, Coor bx, Coor by, Coor width, Coor height);

	void (*swaprect)(RASType *ra, Coor ax, Coor ay,
			  	  RASType *rb, Coor bx, Coor by, Coor width, Coor height);


	void (*zoomblit)(RASType *source, Coor src_x, Coor src_y,
		           RASType *dest, Coor dest_x, Coor dest_y,
		           Ucoor width, Ucoor height, LONG zoom_x, LONG zoom_y );


/* UBYTE[3] color map routines */

	Errcode (*pj_cmap_alloc)(Cmap **pcm, LONG num_colors);
	void (*pj_cmap_free)(Cmap *cmap);
	void (*pj_cmap_load)(void *rast, Cmap *cmap);
	void (*pj_cmap_copy)(Cmap *s,Cmap *d);

/* rcel items for rasters with attached colormaps */

	Errcode (*pj_rcel_bytemap_open)(Rasthdr *spec,Rcel *cel,LONG num_colors);
	Errcode (*pj_rcel_bytemap_alloc)(Rasthdr *spec,Rcel **pcel,LONG num_colors);
	Boolean (*pj_rcel_make_virtual)(Rcel *rc, Rcel *root, Rectangle *toclip);

	void (*pj_rcel_close)(Rcel *rc);
	void (*pj_rcel_free)(Rcel *c);

} Gfxlib;

#undef RASType

#endif /* GFXLIB_H */
