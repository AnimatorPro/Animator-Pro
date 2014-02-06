/* rastlib.h - This contains the function library structure associated with
 * a raster (struct rastlib).   For the most part struct rastlib contains
 * one member for each library function.  However blits and other functions
 * that involve TWO rasters are represented by an array of 4 functions.  
 * This is to handle blits to and from memory as well as within a raster
 * (or between two rasters of the same type.)
 */

#ifndef RASTLIB_H
#define RASTLIB_H

/* cput_dot() and cget_dot() are clipped routines, the only two clipped calls
 * in the raster library
 */

#ifndef RASTER_H
	#include "raster.h"
#endif /* RASTER_H */

#ifndef LibRast
/* Make it so Device drivers can define Raster to be one of their local
 * types. */
	#define LibRast Raster
#endif 


enum { 
/* Symbolic names for different types of blits (used to index into
 * the 4 dimensional array of blit, swap, etc. calls). */
	RL_TO_SAME = 0,
	RL_TO_BYTEMAP = 1,
	RL_FROM_BYTEMAP = 2,
	RL_TO_OTHER = 3,
};

enum {
	NUM_LIB_CALLS = 47,

	/* So don't have to recompile when add a new one. */
	MAX_LIB_CALLS = 100
};

/*** typedefs of all the elements of struct rastlib - because more than
 *** occassionally we'll have to cast a function pointer with slightly
 *** differently typed parameters - mixing up signed/unsigned or
 *** perhaps RamRasts and LibRasts... */
typedef Errcode (*rl_type_close_raster)(LibRast *r);
typedef void (*rl_type_cput_dot)(LibRast *r,Pixel color,Coor x,Coor y);
typedef void (*rl_type_put_dot)(LibRast *r,Pixel color,Coor x,Coor y);
typedef Pixel (*rl_type_cget_dot)(LibRast *r,Coor x,Coor y);
typedef Pixel (*rl_type_get_dot)(LibRast *r,Coor x,Coor y);
typedef void (*rl_type_put_hseg)(LibRast *r
,	void *pixbuf,Ucoor x,Ucoor y,Ucoor w);
typedef void (*rl_type_get_hseg)(LibRast *r
,	void *pixbuf,Ucoor x,Ucoor y,Ucoor w);
typedef void (*rl_type_put_vseg)(LibRast *r
,	void *pixbuf,Ucoor x,Ucoor y,Ucoor h);
typedef void (*rl_type_get_vseg)(LibRast *r
,	void *pixbuf,Ucoor x,Ucoor y,Ucoor h);
typedef void (*rl_type_put_rectpix)(LibRast *r
,	void *pixbuf,Coor x,Coor y,Ucoor w,Ucoor h);
typedef void (*rl_type_get_rectpix)(LibRast *r
,	void *pixbuf,Coor x,Coor y,Ucoor w,Ucoor h);
typedef void (*rl_type_set_hline)(LibRast *r
,	Pixel color,Coor x,Coor y,Ucoor w);
typedef void (*rl_type_set_vline)(LibRast *r
,	Pixel color,Coor x,Coor y,Ucoor h);
typedef void (*rl_type_set_rect)(LibRast *r
,	Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
typedef void (*rl_type_set_rast)(LibRast *r,Pixel color);
typedef void (*rl_type_xor_rect)(LibRast *r
,	Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
typedef void (*rl_type_mask1blit)(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my
,	LibRast *r, Coor rx, Coor ry, Ucoor width, Ucoor height
,	Pixel oncolor );
typedef void (*rl_type_mask2blit)(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my
,	LibRast *r, Coor rx, Coor ry, Ucoor width, Ucoor height
,	Pixel oncolor, Pixel offcolor );
typedef void (*rl_type_unbrun_rect)(LibRast *r,void *ucbuf, LONG pixsize
,	Coor x,Coor y,Ucoor width,Ucoor height);
typedef void (*rl_type_unlccomp_rect)(LibRast *r,void *ucbuf, LONG pixsize
,	Coor x,Coor y,Ucoor width,Ucoor height);
typedef void (*rl_type_unss2_rect)(LibRast *r,void *ucbuf, LONG pixsize
,	Coor x,Coor y,Ucoor width,Ucoor height);
typedef void (*rl_type_diag_to_ptable)(LibRast *sr
,	Pixel *ptable, Ucoor ptsize
,	Coor x0, Coor y0, Coor x1, Coor y1);
typedef void (*rl_type_blitrect)(LibRast *source, Coor src_x, Coor src_y
,	LibRast *dest, Coor dest_x, Coor dest_y
,	Coor width, Coor height);
typedef void (*rl_type_swaprect)(LibRast *ra, Coor ax, Coor ay
,	LibRast *rb, Coor bx, Coor by, Coor width, Coor height);
typedef void (*rl_type_tblitrect)(LibRast *s, Coor sx, Coor sy
,	LibRast *d, Coor dx, Coor dy, Coor width, Coor height
,	Pixel tcolor );
typedef void (*rl_type_xor_rast)(LibRast *s, LibRast *d);
typedef void (*rl_type_zoomblit)( LibRast *source, Coor src_x, Coor src_y
,	LibRast *dest, Coor dest_x, Coor dest_y
,	Ucoor width, Ucoor height, LONG zoom_x, LONG zoom_y );
typedef void (*rl_type_set_colors)(LibRast *r
,	LONG start, LONG count, void *cbuf);
typedef void (*rl_type_uncc64)(LibRast *r, void *cbuf);
typedef void (*rl_type_uncc256)(LibRast *r, void *cbuf);
typedef void (*rl_type_wait_vsync)(LibRast *r); 


typedef struct rastlib {
	rl_type_close_raster close_raster;
	rl_type_cput_dot cput_dot;
	rl_type_put_dot put_dot;
	rl_type_cget_dot cget_dot;
	rl_type_get_dot get_dot;
	rl_type_put_hseg put_hseg;
	rl_type_get_hseg get_hseg;
	rl_type_put_vseg put_vseg;
	rl_type_get_vseg get_vseg;
	rl_type_put_rectpix put_rectpix;
	rl_type_get_rectpix get_rectpix;
	rl_type_set_hline set_hline;
	rl_type_set_vline set_vline;
	rl_type_set_rect set_rect;
	rl_type_set_rast set_rast;
	rl_type_xor_rect xor_rect;
	rl_type_mask1blit mask1blit;
	rl_type_mask2blit mask2blit;
	rl_type_unbrun_rect unbrun_rect;
	rl_type_unlccomp_rect unlccomp_rect;
	rl_type_unss2_rect unss2_rect;
#ifdef PRIVATE_CODE
	rl_type_diag_to_ptable diag_to_ptable;
	VFUNC reserved[1];
#else
	VFUNC reserved[2];
#endif
	rl_type_blitrect blitrect[4];
	rl_type_swaprect swaprect[4];
	rl_type_tblitrect tblitrect[4];
	rl_type_xor_rast xor_rast[4];
	rl_type_zoomblit zoomblit[4];
	rl_type_set_colors set_colors;
	rl_type_uncc64 uncc64;
	rl_type_uncc256 uncc256;
	rl_type_wait_vsync wait_vsync;
#ifdef SLUFFED
	VFUNC libpad[MAX_LIB_CALLS - NUM_LIB_CALLS];
#endif /* SLUFFED */
} Rastlib;
STATIC_ASSERT(rastlib, sizeof(Rastlib) == NUM_LIB_CALLS * sizeof(VFUNC));

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

