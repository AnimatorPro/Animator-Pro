#ifndef RASTLIB_H
#define RASTLIB_H

#ifndef RASTER_H
	#include "raster.h"
#endif /* RASTER_H */

#ifndef LibRast
/* Make it so Device drivers can define Raster to be one of their local
 * types. */
	#define LibRast Raster
#endif 


enum { 
	RL_TO_SAME = 0,
	RL_TO_BYTEMAP = 1,
	RL_FROM_BYTEMAP = 2,
	RL_TO_OTHER = 3,
};

#define NUM_LIB_CALLS		100		/* So don't have to recompile when add 
										a new one.  Should be 57 or so. */

/* put_dot() and get_dot() are clipped routines, the only two clipped calls
 * in the raster library
 *
 * the calls _put_dot() and _get_dot() are the unclipped unclipped dot calls 
 *
 */

typedef struct rastlib {
	Errcode (*close_raster)(LibRast *r);
	void (*cput_dot)(LibRast *r,Pixel color,Coor x,Coor y);
	void (*put_dot)(LibRast *r,Pixel color,Coor x,Coor y);
	Pixel (*cget_dot)(LibRast *r,Coor x,Coor y);
	Pixel (*get_dot)(LibRast *r,Coor x,Coor y);
	void (*put_hseg)(LibRast *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor w);
	void (*get_hseg)(LibRast *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor w);
	void (*put_vseg)(LibRast *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor h);
	void (*get_vseg)(LibRast *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor h);
	void (*put_rectpix)(LibRast *r,void *pixbuf,Coor x,Coor y,Ucoor w,Ucoor h);
	void (*get_rectpix)(LibRast *r,void *pixbuf,Coor x,Coor y,Ucoor w,Ucoor h);
	void (*set_hline)(LibRast *r,Pixel color,Coor x,Coor y,Ucoor w);
	void (*set_vline)(LibRast *r,Pixel color,Coor x,Coor y,Ucoor h);
	void (*set_rect)(LibRast *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
	void (*set_rast)(LibRast *r,Pixel color);
	void (*xor_rect)(LibRast *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
	void (*mask1blit)(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
				   LibRast *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
				   Pixel oncolor );
	void (*mask2blit)(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
				   LibRast *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
				   Pixel oncolor, Pixel offcolor );
	void (*unbrun_rect)(LibRast *r,void *ucbuf, LONG pixsize,
					 Coor x,Coor y,Ucoor width,Ucoor height);
	void (*unlccomp_rect)(LibRast *r,void *ucbuf, LONG pixsize,
					 Coor x,Coor y,Ucoor width,Ucoor height);
	void (*unss2_rect)(LibRast *r,void *ucbuf, LONG pixsize,
					   Coor x,Coor y,Ucoor width,Ucoor height);

	void *reserved[2];

	void (*blitrect[4])(LibRast *source, Coor src_x, Coor src_y,
				 LibRast *dest, Coor dest_x, Coor dest_y,
				 Coor width, Coor height);
	void (*swaprect[4])(LibRast *ra, Coor ax, Coor ay,
			  	  LibRast *rb, Coor bx, Coor by, Coor width, Coor height);
	void (*tblitrect[4])(LibRast *s, Coor sx, Coor sy,
			  LibRast *d, Coor dx, Coor dy, Coor width, Coor height,
			  Pixel tcolor );
	void (*xor_rast[4])(LibRast *s, LibRast *d);
	void (*zoomblit[4])( LibRast *source, Coor src_x, Coor src_y,
		           LibRast *dest, Coor dest_x, Coor dest_y,
		           Ucoor width, Ucoor height, LONG zoom_x, LONG zoom_y );
	void (*set_colors)(LibRast *r, LONG start, LONG count, void *cbuf);
	void (*uncc64)(LibRast *r, void *cbuf);
	void (*uncc256)(LibRast *r, void *cbuf);
	void (*wait_vsync)(LibRast *r); /* wait for top of vertical blank
	 								* (bottom of visible frame) if currently 
									* in vertical blank will wait until next
									* vertical blank start */

	char libpad[NUM_LIB_CALLS*sizeof(VFUNC)-188]; /* enough for 100 entries */
} Rastlib;

#ifdef PRIVATE_CODE

struct _rastlib_h_errcheck_ {
	char xx[sizeof(Rastlib) == NUM_LIB_CALLS*sizeof(VFUNC)];
};

void pj_grc_load_dcompcalls(Rastlib *lib);
void pj_grc_load_compcalls(Rastlib *lib);
void pj_grc_load_commcalls(Rastlib *lib);
void pj_grc_load_fullcalls(Rastlib *lib);

/* PRIVATE_CODE */ 
#endif

/* some macroized library calls */

#define CLOSE_RAST(r) (*((r)->lib->close_raster))((LibRast *)(r))
#define PUT_DOT(r,c,x,y) (*((r)->lib->put_dot))((LibRast *)(r),c,x,y)
#define CPUT_DOT(r,c,x,y) (*((r)->lib->cput_dot))((LibRast *)(r),c,x,y)
#define GET_DOT(r,x,y) (*((r)->lib->get_dot))((LibRast *)(r),x,y)
#define CGET_DOT(r,x,y) (*((r)->lib->cget_dot))((LibRast *)(r),x,y)
#define PUT_HSEG(r,pb,x,y,w) (*((r)->lib->put_hseg))((LibRast *)(r),pb,x,y,w)
#define GET_HSEG(r,pb,x,y,w) (*((r)->lib->get_hseg))((LibRast *)(r),pb,x,y,w)
#define PUT_VSEG(r,pb,x,y,h) (*((r)->lib->put_vseg))((LibRast *)(r),pb,x,y,h)
#define GET_VSEG(r,pb,x,y,h) (*((r)->lib->get_vseg))((LibRast *)(r),pb,x,y,h)
#define SET_HLINE(r,c,x,y,w) (*((r)->lib->set_hline))((LibRast *)(r),c,x,y,w)
#define SET_VLINE(r,c,x,y,h) (*((r)->lib->set_vline))((LibRast *)(r),c,x,y,h)
#define SET_RECT(r,c,x,y,w,h) (*((r)->lib->set_rect)) \
	((LibRast *)(r),c,x,y,w,h)
#define XOR_RECT(r,c,x,y,w,h) (*((r)->lib->xor_rect)) \
	((LibRast *)(r),c,x,y,w,h)
#define MASK1BLIT(mp,bpr,mx,my,r,rx,ry,w,h,oncol) \
	(*((r)->lib->mask1blit))(mp,bpr,mx,my,(LibRast *)(r),rx,ry,w,h,oncol)
#define MASK2BLIT(mp,bpr,mx,my,r,rx,ry,w,h,oncol,offcol) \
	(*((r)->lib->mask2blit))(mp,bpr,mx,my, \
	(LibRast *)(r),rx,ry,w,h,oncol,offcol)
#define SET_COLORS(r, start, count, cbuf) \
	(*((r)->lib->set_colors))((LibRast *)(r), start, count, cbuf)
#define UNCC64(r, cbuf) \
	(*((r)->lib->uncc64))((LibRast *)(r), cbuf)
#define UNCC256(r, cbuf) \
	(*((r)->lib->uncc256))((LibRast *)(r), cbuf)
#define WAIT_VSYNC(r) \
	(*((r)->lib->wait_vsync))((LibRast *)(r))

#endif /* RASTLIB_H */

