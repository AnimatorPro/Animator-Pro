/* rastcall.c */

#include "rastcall.ih"

/* Function: pj_put_dot
 *
 *  Set dot to colour.
 */
void
pj_put_dot(Raster *r, Pixel col, Coor x, Coor y)
{
	CPUT_DOT(r, col, x, y);
}

void
pj__put_dot(Raster *r, Pixel col, Coor x, Coor y)
{
	PUT_DOT(r, col, x, y);
}

/* Function: pj_get_dot
 *
 *  Get colour of dot.
 */
Pixel
pj_get_dot(Raster *r, Coor x, Coor y)
{
	return(CGET_DOT(r, x, y));
}

Pixel
pj__get_dot(Raster *r, Coor x, Coor y)
{
	return(GET_DOT(r, x, y));
}

/*--------------------------------------------------------------*/

/* Function: pj_put_hseg
 *
 *  Move pixels from memory to a horizontal line of destination raster.
 *  This will clip and put pixels in from parts of buffer that overlap
 *  destination buffer.
 *
 *  See rcphseg.c.
 */
void
pj_put_hseg(Raster *r, void *pixbuf, Ucoor x, Ucoor y, Ucoor w);

void
pj__put_hseg(Raster *r, void *pixbuf, Ucoor x, Ucoor y, Ucoor w)
{
	PUT_HSEG(r, pixbuf, x, y, w);
}

/* Function: pj_get_hseg
 *
 *  Move pixels from a horizontal line of source raster to memory buffer.
 *  this will clip and put pixels in from parts of buffer that overlap
 *  destination buffer.
 *
 *  See rcghseg.c.
 */
void
pj_get_hseg(Raster *r, void *pixbuf, Ucoor x, Ucoor y, Ucoor w);

void
pj__get_hseg(Raster *r, void *pixbuf, Ucoor x, Ucoor y, Ucoor w)
{
	GET_HSEG(r, pixbuf, x, y, w);
}

/* Function: pj_put_vseg
 *
 *  Move pixels from memory to a vertical line of destination raster.
 *  (Clipped will put pixels in destination that overlap.)
 *
 *  See rcpvseg.c.
 */
void
pj_put_vseg(Raster *r, void *pixbuf, Ucoor x, Ucoor y, Ucoor h);

#ifdef SLUFFED
void
pj__put_vseg(Raster *r, void *pixbuf, Ucoor x, Ucoor y, Ucoor h)
{
	PUT_VSEG(r, pixbuf, x, y, h);
}
#endif /* SLUFFED */

/* Function: pj_get_vseg
 *
 *  Move pixels from a vertical line of source raster to memory buffer.
 *  (Clipped will only get pixels where buffer overlaps source and
 *  will leave other parts of buffer unaffected.)
 */
void
pj_get_vseg(Raster *r, void *pixbuf, Ucoor x, Ucoor y, Ucoor h);

#ifdef SLUFFED
void
pj__get_vseg(Raster *r, void *pixbuf, Ucoor x, Ucoor y, Ucoor h)
{
	GET_VSEG(r, pixbuf, x, y, h);
}
#endif /* SLUFFED */

/* Function: _pj_put_rectpix
 *
 *  Loads all pixels from a linear pixel buffer into a screen rectangle.
 *
 *  See rcprpix.c.
 */
void
_pj_put_rectpix(Raster *r, void *pixbuf, Coor x, Coor y, Ucoor w, Ucoor h);

/* Function: pj_get_rectpix
 *
 *  Loads all pixels from a screen rectangle into a linear buffer
 *  on a line by line basis.
 *  If the rectangle is clipped garbage will
 *  be in the clipped off section of the buffer.
 *
 *  See rcgrpix.c.
 */
void
pj_get_rectpix(Raster *r, void *pixbuf, Coor x, Coor y, Ucoor w, Ucoor h);

/* Function: pj_set_hline
 *
 *  Draw a horizontal line in a solid color.  This is clipped, so it's
 *  ok for the line to partially or entirely outside the raster.
 *
 *  r - what we draw on.
 *  col - colour of horizontal line.
 *  x - left end of line.
 *  y - vertical position of line.
 *  w - width of line in pixels.
 *
 *  See rcshline.c.
 */
void
pj_set_hline(Raster *r, Pixel col, Coor x, Coor y, Ucoor w);

#ifdef SLUFFED
void
pj__set_hline(Raster *r, Pixel col, Coor x, Coor y, Ucoor w)
{
	SET_HLINE(r, col, x, y, w);
}
#endif /* SLUFFED */

/* Function: pj_set_vline
 *
 *  Draw a solid vertical line.
 *  (Clipped.)
 *
 *  See rcsvline.c.
 */
void
pj_set_vline(Raster *r, Pixel col, Coor x, Coor y, Ucoor h);

#ifdef SLUFFED
void
pj__set_vline(Raster *r, Pixel col, Coor x, Coor y, Ucoor h)
{
	SET_VLINE(r, col, x, y, h);
}
#endif /* SLUFFED */

/* Function: pj_set_rect
 *
 *  Draw a solid colour rectangle.  Rectangle is clipped to fit raster.
 *
 *  r - what get's drawn on.
 *  col - colour to draw with.
 *  x - left edge of rectangle.
 *  y - top edge of rectangle.
 *  w - width.
 *  y - height.
 *
 *  See rcsetrec.c.
 */
void
pj_set_rect(Raster *r, Pixel col, Coor x, Coor y, Ucoor w, Ucoor h);

void
pj__set_rect(Raster *r, Pixel col, Coor x, Coor y, Ucoor w, Ucoor h)
{
	SET_RECT(r, col, x, y, w, h);
}

/* Function: pj_set_rast
 *
 *  Sets entire raster to a colour fast.
 */
void
pj_set_rast(Raster *r, Pixel col)
{
	(r->lib->set_rast)(r, col);
}

/* Function: pj_xor_rect
 *
 *  see rcxorect.c.
 */
void
pj_xor_rect(Raster *r, Pixel col, Coor x, Coor y, Ucoor w, Ucoor h);

#ifdef SLUFFED
void
pj__xor_rect(Raster *r, Pixel col, Coor x, Coor y, Ucoor w, Ucoor h)
{
	XOR_RECT(r, col, x, y, w, h);
}
#endif /* SLUFFED */

/* Function: pj_mask1blit
 *
 *  Sets rectangle of raster rectangle of mask.
 *  (mask on = oncol, off = noaction)
 *
 *  See rcmsk1bl.c.
 */
void
pj_mask1blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
		Raster *r, Coor rx, Coor ry, Ucoor w, Ucoor h,
		Pixel oncol);

#ifdef SLUFFED
void
pj__mask1blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
		Raster *r, Coor rx, Coor ry, Ucoor w, Ucoor h,
		Pixel oncol)
{
	MASK1BLIT(mbytes, mbpr, mx, my, r, rx, ry, w, h, oncol);
}
#endif /* SLUFFED */

/* Function: pj_mask2blit
 *
 *  Sets rectangle of raster rectangle of mask.
 *  (mask on = oncol, off = noaction)
 */
void
pj_mask2blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
		Raster *r, Coor rx, Coor ry, Ucoor w, Ucoor h,
		Pixel oncol, Pixel offcol);

#ifdef SLUFFED
void
pj__mask2blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
		Raster *r, Coor rx, Coor ry, Ucoor w, Ucoor h,
		Pixel oncol, Pixel offcol)
{
	MASK2BLIT(mbytes, mbpr, mx, my, r, rx, ry, w, h, oncol, offcol);
}
#endif /* SLUFFED */

/* Function: pj_unbrun_rect */
void
pj_unbrun_rect(Raster *r, void *ucbuf, LONG pixsize,
		Coor x, Coor y, Ucoor w, Ucoor h)
{
	(r->lib->unbrun_rect)(r, ucbuf, pixsize, x, y, w, h);
}

/* Function: pj_unlccomp_rect */
void
pj_unlccomp_rect(Raster *r, void *ucbuf, LONG pixsize,
		Coor x, Coor y, Ucoor w, Ucoor h)
{
	(r->lib->unlccomp_rect)(r, ucbuf, pixsize, x, y, w, h);
}

/* Function: pj_unss2_rect */
void
pj_unss2_rect(Raster *r, void *ucbuf, LONG pixsize,
		Coor x, Coor y, Ucoor w, Ucoor h)
{
	(r->lib->unss2_rect)(r, ucbuf, pixsize, x, y, w, h);
}

/* Function: pj_diag_to_ptable */
void
pj_diag_to_ptable(Raster *r, Pixel *ptable, Ucoor ptsize,
		Coor x0, Coor y0, Coor x1, Coor y1)
{
	(r->lib->diag_to_ptable)(r, ptable, ptsize, x0, y0, x1, y1);
}

/* Function: pj_set_colors */
void
pj_set_colors(Raster *r, LONG start, LONG count, UBYTE *table)
{
	SET_COLORS(r, start, count, table);
}

/* Function: pj_uncc64 */
void
pj_uncc64(Raster *r, void *cbuf)
{
	UNCC64(r, cbuf);
}

/* Function: pj_uncc256 */
void
pj_uncc256(Raster *r, void *cbuf)
{
	UNCC256(r, cbuf);
}

/* Function: pj_wait_rast_vsync */
void
pj_wait_rast_vsync(Raster *r)
{
	WAIT_VSYNC(r);
}
