/*****************************************************************************
 * PJGFX.H - Prototypes for the graphics library functions.
 *
 *	Note:  The PJSTYPES.H or STDTYPES.H file must be included before this
 *		   this file.  Don't code an include for it below, or the glue code
 *		   will break!
 ****************************************************************************/

#ifndef PJGFX_H
#define PJGFX_H

/*----------------------------------------------------------------------------
 * simple clipped drawing functions (dots, lines, rectangles)...
 *--------------------------------------------------------------------------*/

void	pj_put_dot(FlicRaster *r, Pixel color, Coor x, Coor y);
Pixel	pj_get_dot(FlicRaster *r, Coor x, Coor y);
void	pj_put_hseg(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor w);
void	pj_get_hseg(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor w);
void	pj_put_vseg(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor h);
void	pj_get_vseg(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor h);
void	_pj_put_rectpix(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor w, Ucoor h);
void	pj_get_rectpix(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor w, Ucoor h);
void	pj_set_hline(FlicRaster *r, Pixel color, Coor x, Coor y, Ucoor w);
void	pj_set_vline(FlicRaster *r, Pixel color, Coor x, Coor y, Ucoor h);
void	pj_set_rast(FlicRaster *r, Pixel color);
void	pj_set_rect(FlicRaster *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);
void	pj_xor_rect(FlicRaster *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);

/*----------------------------------------------------------------------------
 * simple unclipped drawing functions...
 *--------------------------------------------------------------------------*/

void	pj__put_dot(FlicRaster *r, Pixel color, Coor x, Coor y);
Pixel	pj__get_dot(FlicRaster *r, Coor x, Coor y);
void	pj__put_hseg(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor w);
void	pj__get_hseg(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor w);
void	pj__put_vseg(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor h);
void	pj__get_vseg(FlicRaster *r, void *pixbuf, Coor x, Coor y, Ucoor h);
void	pj__set_hline(FlicRaster *r, Pixel color, Coor x, Coor y, Ucoor w);
void	pj__set_vline(FlicRaster *r, Pixel color, Coor x, Coor y, Ucoor h);
void	pj__set_rect(FlicRaster *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);
void	pj__xor_rect(FlicRaster *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);

/*----------------------------------------------------------------------------
 * masked and keyed blits (clipped & unclipped)...
 *--------------------------------------------------------------------------*/

void	pj_mask1blit(unsigned char *mbytes, Coor mbpr, Coor mx, Coor my,
					 FlicRaster *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
					 Pixel oncolor );

void	pj_mask2blit(unsigned char *mbytes, Coor mbpr, Coor mx, Coor my,
					 FlicRaster *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
					 Pixel oncolor, Pixel offcolor );

void	pj__mask1blit(unsigned char *mbytes, Coor mbpr, Coor mx, Coor my,
					  FlicRaster *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
					  Pixel oncolor );

void	pj__mask2blit(unsigned char *mbytes, Coor mbpr, Coor mx, Coor my,
					  FlicRaster *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
					  Pixel oncolor, Pixel offcolor );

/*----------------------------------------------------------------------------
 * decompressors...
 *--------------------------------------------------------------------------*/

void pj_unbrun_rect(FlicRaster *r, void *ucbuf, long pixsize,
					Coor x, Coor y, Ucoor width, Ucoor height);

void pj_unlccomp_rect(FlicRaster *r, void *ucbuf, long pixsize,
					  Coor x, Coor y, Ucoor width, Ucoor height);

void pj_unss2_rect(FlicRaster *r, void *ucbuf, long pixsize,
				   Coor x, Coor y, Ucoor width, Ucoor height);

/*----------------------------------------------------------------------------
 * intra-raster and inter-raster blits...
 *--------------------------------------------------------------------------*/

void pj_xor_rast(FlicRaster *s, FlicRaster *d);

void pj_blitrect(FlicRaster *source, Coor src_x, Coor src_y,
				 FlicRaster *dest, Coor dest_x, Coor dest_y,
				 Coor width, Coor height);

void pj_tblitrect(FlicRaster *s, Coor sx, Coor sy,
				  FlicRaster *d, Coor dx, Coor dy, Coor width, Coor height,
				  Pixel tcolor );

void pj_swaprect(FlicRaster *ra, Coor ax, Coor ay,
				 FlicRaster *rb, Coor bx, Coor by, Coor width, Coor height);

void pj_zoomblit(FlicRaster *source, Coor src_x, Coor src_y,
				 FlicRaster *dest, Coor dest_x, Coor dest_y,
				 Ucoor width, Ucoor height, long zoom_x, long zoom_y );

void pj__blitrect(FlicRaster *source, Coor src_x, Coor src_y,
				  FlicRaster *dest, Coor dest_x, Coor dest_y,
				  Coor width, Coor height);

void pj__tblitrect(FlicRaster *s, Coor sx, Coor sy,
				   FlicRaster *d, Coor dx, Coor dy, Coor width, Coor height,
				   Pixel tcolor );

void pj__swaprect(FlicRaster *ra, Coor ax, Coor ay,
				  FlicRaster *rb, Coor bx, Coor by, Coor width, Coor height);

/*----------------------------------------------------------------------------
 * hardware functions (these do nothing on a non-hardware raster)...
 *--------------------------------------------------------------------------*/

void pj_wait_rast_vsync(FlicRaster *r);

void pj_set_colors(FlicRaster *r, long start, long count, void *rgb_table);

void pj_uncc64(FlicRaster *r, void *cbuf);

void pj_uncc256(FlicRaster *r, void *cbuf);

/*----------------------------------------------------------------------------
 * these pragmas allow -3r clients to use our -3s style functions...
 *	 the FLICLIB3S alias is defined in PJSTYPES.H.
 *--------------------------------------------------------------------------*/

#ifdef __WATCOMC__
	#pragma aux (FLICLIB3S) pj_put_dot;
	#pragma aux (FLICLIB3S) pj_get_dot;
	#pragma aux (FLICLIB3S) pj_put_hseg;
	#pragma aux (FLICLIB3S) pj_get_hseg;
	#pragma aux (FLICLIB3S) pj_put_vseg;
	#pragma aux (FLICLIB3S) pj_get_vseg;
	#pragma aux (FLICLIB3S) _pj_put_rectpix;
	#pragma aux (FLICLIB3S) pj_get_rectpix;
	#pragma aux (FLICLIB3S) pj_set_hline;
	#pragma aux (FLICLIB3S) pj_set_vline;
	#pragma aux (FLICLIB3S) pj_set_rast;
	#pragma aux (FLICLIB3S) pj_set_rect;
	#pragma aux (FLICLIB3S) pj_xor_rect;
	#pragma aux (FLICLIB3S) pj__put_dot;
	#pragma aux (FLICLIB3S) pj__get_dot;
	#pragma aux (FLICLIB3S) pj__put_hseg;
	#pragma aux (FLICLIB3S) pj__get_hseg;
	#pragma aux (FLICLIB3S) pj__put_vseg;
	#pragma aux (FLICLIB3S) pj__get_vseg;
	#pragma aux (FLICLIB3S) pj__set_hline;
	#pragma aux (FLICLIB3S) pj__set_vline;
	#pragma aux (FLICLIB3S) pj__set_rect;
	#pragma aux (FLICLIB3S) pj__xor_rect;
	#pragma aux (FLICLIB3S) pj_mask1blit;
	#pragma aux (FLICLIB3S) pj_mask2blit;
	#pragma aux (FLICLIB3S) pj__mask1blit;
	#pragma aux (FLICLIB3S) pj__mask2blit;
	#pragma aux (FLICLIB3S) pj_unbrun_rect;
	#pragma aux (FLICLIB3S) pj_unlccomp_rect;
	#pragma aux (FLICLIB3S) pj_unss2_rect;
	#pragma aux (FLICLIB3S) pj_xor_rast;
	#pragma aux (FLICLIB3S) pj_blitrect;
	#pragma aux (FLICLIB3S) pj_tblitrect;
	#pragma aux (FLICLIB3S) pj_swaprect;
	#pragma aux (FLICLIB3S) pj_zoomblit;
	#pragma aux (FLICLIB3S) pj__blitrect;
	#pragma aux (FLICLIB3S) pj__tblitrect;
	#pragma aux (FLICLIB3S) pj__swaprect;
	#pragma aux (FLICLIB3S) pj_wait_rast_vsync;
	#pragma aux (FLICLIB3S) pj_set_colors;
	#pragma aux (FLICLIB3S) pj_uncc64;
	#pragma aux (FLICLIB3S) pj_uncc256;
#endif
#endif /* PJGFX_H */
