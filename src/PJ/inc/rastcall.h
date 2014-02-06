#ifndef RASTCALL_H
#define RASTCALL_H

#ifndef RASTER_H
	#include "raster.h"
#endif

#ifdef PRIVATE_CODE

/* functions used by raster library code */

extern void pj_set_grc_calls(void *lib);
extern void *pj_get_grc_lib(void);

/* some functions that do nothing for bits of library not implemented */
extern void pj_vdo_nutin(void);
extern SHORT pj_sdo_nutin(void);
extern Pixel pj_rcdo_nutin(void);
extern Errcode errdo_nutin(void); /* returns Err_unimpl */

/* clipping subroutines ***/

Coor pj_lclipdim(Coor *val,Coor length, Coor maxval);
Coor pj_lclipdim2(Coor *sval,Coor *dval,Coor length, Coor maxval);
Ucoor pj_lclip2rects(Coor *aval, Coor *bval, Ucoor length, Ucoor amax, Ucoor bmax);

#define SBUF_SIZE 1024	/* size of allowable stack buffer used in
						 * raster calls */

/* PRIVATE_CODE */ #endif

#ifdef RASTCALL_INTERNALS
	#define RASType Raster
#else
/* because Raster types have same first fields but different typedefs */
#define RASType void
#endif


/* builds a "clipbox" given a root raster this will point all clipped routines
 * to the new boundarys. The new boundarys will be themselves clipped to
 * the root, returns FALSE if the new boundarys do not intersect with the
 * root */

Boolean pj_clipbox_make(Clipbox *cb, RASType *r,
					  Coor x,Coor y,Coor width,Coor height);

/* direct calls to build bytemap rasters */

/* opens bytemap raster use pj_close_raster() to close */
Errcode pj_open_bytemap(Rasthdr *spec,Bytemap *bm);

/* allocates and opens bytemap raster use free_raster() to free */
Errcode pj_alloc_bytemap(Rasthdr *spec,Bytemap **pbm);

#ifndef REXLIB_CODE /**** host side only ****/

Errcode pj_build_bytemap(Rasthdr *spec,Raster *r, UBYTE *pixels);

/* calls to build bitmap rasters */

Errcode pj_open_bitmap(Rasthdr *spec,Bitmap *bm);
Errcode pj_alloc_bitmap(Rasthdr *spec,Bitmap **pbm);

Errcode pj_build_bitmap(Rasthdr *spec, Raster *r, UBYTE *pixels);

/* This call will load a raster to represent vanilla 320 X 200 mcga display */

Errcode pj_open_mcga_raster(Raster *r);

/* a little item the NULL raster just dumps output and gets zeros for input */

Errcode pj_open_nullrast(RASType *r);

/****** custom raster building *******/

void pj_grc_load_fullcalls(struct rastlib *lib);
void pj_grc_load_dcompcalls(struct rastlib *lib);
void pj_grc_load_compcalls(struct rastlib *lib);

/* temporary raster buffer allocator */

Errcode pj_get_rectbuf(Coor bytes_per_row, long *pheight, UBYTE **lbuf);

/* REXLIB_CODE */ #endif

/* temporary raster allocator:
 * (will get height available even if smaller than requested) */

Errcode pj_open_temprast(Bytemap *rr,SHORT width,SHORT height,
					  SHORT pdepth);


/* single raster library calls */

void pj_rast_free(RASType *r);
void pj_close_raster(RASType *r);

void pj_put_dot(RASType *r,Pixel color,Coor x,Coor y);
void pj__put_dot(RASType *r,Pixel color,Coor x,Coor y);
Pixel pj_get_dot(RASType *r,Coor x,Coor y);
Pixel pj__get_dot(RASType *r,Coor x,Coor y);

extern void pj_put_hseg (RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor w);
extern void pj__put_hseg(RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor w);
extern void pj_get_hseg (RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor w);
extern void pj__get_hseg(RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor w);
extern void pj_put_vseg (RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor h);
extern void pj__put_vseg(RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor h);
extern void pj_get_vseg (RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor h);
extern void pj__get_vseg(RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor h);

extern void
_pj_put_rectpix(RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor w, Ucoor h);

extern void
pj_get_rectpix(RASType *r, Pixel *pixbuf, Coor x, Coor y, Ucoor w, Ucoor h);

void pj__set_hline(RASType *r,Pixel color,Coor x,Coor y,Ucoor w);
void pj_set_hline(RASType *r,Pixel color,Coor x,Coor y,Ucoor w);
void pj__set_vline(RASType *r,Pixel color,Coor x,Coor y,Ucoor h);
void pj_set_vline(RASType *r,Pixel color,Coor x,Coor y,Ucoor h);
void pj_set_rast(RASType *r,Pixel color);
void pj_clear_rast(RASType *r);
void pj__set_rect(RASType *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
void pj_set_rect(RASType *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
void pj__xor_rect(RASType *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);
void pj_xor_rect(RASType *r,Pixel color,Coor x,Coor y,Ucoor w,Ucoor h);

void pj__mask1blit(UBYTE *bitplane, Coor mbpr, Coor mx, Coor my,
			   RASType *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
			   Pixel oncolor);

void pj_mask1blit(UBYTE *bitplane, Coor mbpr, Coor mx, Coor my,
			   RASType *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
			   Pixel oncolor);

void pj__mask2blit(UBYTE *bitplane, Coor mbpr, Coor mx, Coor my,
			   RASType *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
			   Pixel oncolor, Pixel offcolor);

void pj_mask2blit(UBYTE *bitplane, Coor mbpr, Coor mx, Coor my,
			   RASType *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
			   Pixel oncolor, Pixel offcolor);

/* decompressors */

void pj_unbrun_rect(RASType *r,void *ucbuf, LONG pixsize,
				 Coor x,Coor y,Ucoor width,Ucoor height);

void pj_unlccomp_rect(RASType *r,void *ucbuf, LONG pixsize,
				 Coor x,Coor y,Ucoor width,Ucoor height);

void pj_unss2_rect(RASType *r,void *ucbuf, LONG pixsize,
				   Coor x,Coor y,Ucoor width,Ucoor height);

/* binary calls */

void pj_xor_rast(RASType *s, RASType *d);

void pj__blitrect(RASType *source, Coor src_x, Coor src_y,
			 RASType *dest, Coor dest_x, Coor dest_y,Coor width, Coor height);

void pj_blitrect(RASType *source, Coor src_x, Coor src_y,
			 RASType *dest, Coor dest_x, Coor dest_y,Coor width, Coor height);

void pj__tblitrect(RASType *s, Coor sx, Coor sy,
		  RASType *d, Coor dx, Coor dy, Coor width, Coor height,
		  Pixel tcolor);

void pj_tblitrect(RASType *s, Coor sx, Coor sy,
		  RASType *d, Coor dx, Coor dy, Coor width, Coor height,
		  Pixel tcolor);


void pj__swaprect(RASType *ra, Coor ax, Coor ay,
			  RASType *rb, Coor bx, Coor by, Coor width, Coor height);

void pj_swaprect(RASType *ra, Coor ax, Coor ay,
			  RASType *rb, Coor bx, Coor by, Coor width, Coor height);


void pj_zoomblit( RASType *source, Coor src_x, Coor src_y,
			   RASType *dest, Coor dest_x, Coor dest_y,
			   Ucoor width, Ucoor height, LONG zoom_x, LONG zoom_y);

/* these calls do nothing unless the raster is representing a video display */

void pj_wait_rast_vsync(RASType *r); /* waits until bottom of frame
								   * or vertical blanking start */

void pj_set_colors(RASType *r, LONG start, LONG count, UBYTE *rgb_table);

void pj_uncc64(RASType *r, void *cbuf);

void pj_uncc256(RASType *r, void *cbuf);

#ifndef REXLIB_CODE /**** host side only ****/
#ifdef PRIVATE_CODE

void pj_diag_to_ptable(RASType *sr, Pixel *ptable, Ucoor ptsize,
					Coor x0, Coor y0, Coor x1, Coor y1);

/* PRIVATE_CODE */ #endif


/* REXLIB_CODE */ #endif


#undef RASType

#endif /* RASTCALL_H */
