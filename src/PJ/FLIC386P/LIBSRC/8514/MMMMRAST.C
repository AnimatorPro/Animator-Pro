/*****************************************************************************
 * MMMMRAST.C - The raster library interface for the 8514 driver.
 *
 *
 *	 NOTE:	The following symbols in the driver are unreferenced.  These are
 *			functions that have been coded but never tested.  Don't add any
 *			of these functions to the raster library unless you're prepared
 *			to do extensive testing of the functions!
 *
 *			   pj_8514_blitrect 			 (mmmmblin.asm)
 *			   pj_8514_d_hline				 (mmmmdhli.asm)
 *			   pj_8514_get_dot				 (mmmmdot.asm)
 *			   pj_8514_put_dot				 (mmmmdot.asm)
 *			   _pj_8514_mask2blit			 (mmmmmsk2.asm)
 *			   pj_8514_set_rect 			 (mmmmrect.asm)
 *
 ****************************************************************************/

#include "a8514.h"

/* --------------- raster level functions -------------------------------*/

extern void pj_8514_wait_vsync(void);
extern void _pj_8514_put_dot(Rast8514 *v, UBYTE color, long x, long y);
extern Pixel _pj_8514_get_dot(Rast8514 *v, long x, long y);
extern void pj_8514_set_colors(Rast8514 *v,
	long start_ix, long count, UBYTE *ctable);
extern	void _pj_8514_get_hseg(Rast8514 *r,void *pixbuf,
	Ucoor x,Ucoor y,Ucoor width);
extern void _pj_8514_put_hseg(Rast8514 *r,void *pixbuf,
	Ucoor x,Ucoor y,Ucoor width);
void _pj_8514_d_hline(Rast8514 *bm, Pixel color, Coor minx,
	Coor y, Ucoor width);
void _mmmm_d_vline(Rast8514 *bm, Pixel color, Coor x,
	Coor miny, Ucoor height);
void _pj_8514_set_rect(Rast8514 *bm,  Pixel color,
			 Coor x, Coor y, Ucoor width, Ucoor height);
void mmmm_xor_rect(Rast8514 *bm,  Pixel color,
			 Coor x, Coor y, Ucoor width, Ucoor height);
void mmmm_set_rast(Rast8514 *v, Pixel color);
void mmmm_uncc(Rast8514 *v, void *csource);
void pj_8514_f_blitrect(LibRast *source, Coor sx, Coor sy,
			 Rast8514 *dest, Coor dx, Coor dy,
			 Coor width, Coor height);
void pj_8514_t_blitrect(LibRast *source, Coor sx, Coor sy,
			 Rast8514 *dest, Coor dx, Coor dy,
			 Coor width, Coor height);
void pj_8514_blitrect(Rast8514 *source, Coor sx, Coor sy,
			 Rast8514 *dest, Coor dx, Coor dy,
			 Coor width, Coor height);
void _pj_8514_mask1blit(UBYTE *mbytes, Coor mbpr, Coor sx, Coor sy,
					 Rast8514 *v, Coor dx, Coor dy,
					 Ucoor width, Ucoor height,
					 Pixel oncolor);


static void mmmm_unss2_rect(Rast8514 *v, void *ucbuf, LONG pixsize,
	Coor xorg, Coor yorg, Ucoor width, Ucoor height)
/* Uncompress data into a rectangular area inside raster using
   word-run-length/delta compression scheme used in Autodesk Animator 386
   for most frames except the first. */
/* (Unclipped.) */
{
pj_8514_unss2(xorg+v->hw.am.xcard, yorg+v->hw.am.ycard, ucbuf, width);
}


static void cel_wait_vsync(Rast8514 *r)
{
	if(r->hw.am.flags & IS_DISPLAYED)
		pj_8514_wait_vsync();
}

extern print_8514(Rast8514 *r);
extern Errcode pj_8514_close_rast(Rast8514 *rast);

void *pj_8514_get_rlib(void)
{
static Rastlib mmmm_raster_library;
static got_lib = 0;

	if (!got_lib)
	{
		mmmm_raster_library.close_raster = pj_8514_close_rast;
		mmmm_raster_library.put_dot = _pj_8514_put_dot;
		mmmm_raster_library.get_dot = _pj_8514_get_dot;
		mmmm_raster_library.set_colors = pj_8514_set_colors;
		mmmm_raster_library.set_hline = _pj_8514_d_hline;
		mmmm_raster_library.get_hseg = _pj_8514_get_hseg;
		mmmm_raster_library.put_hseg = _pj_8514_put_hseg;
		mmmm_raster_library.mask1blit = _pj_8514_mask1blit;
		mmmm_raster_library.set_rect = _pj_8514_set_rect;

		mmmm_raster_library.unss2_rect = mmmm_unss2_rect;

		mmmm_raster_library.blitrect[RL_TO_BYTEMAP] = pj_8514_t_blitrect;
		mmmm_raster_library.blitrect[RL_FROM_BYTEMAP] = pj_8514_f_blitrect;
		mmmm_raster_library.wait_vsync = cel_wait_vsync;
		got_lib = 1;
	}
	return(&mmmm_raster_library);
}


