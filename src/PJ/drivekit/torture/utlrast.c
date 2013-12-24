#include "torture.h"

/*----------------------------------------------------------------------------
 * the following routine is inline'd under Watcom C, it's like a fast
 * memcmp() that knows about Watcom's habit of always keeping the same
 * value in DS and ES.	(pity that watcom's own lib code doesn't know that).
 *--------------------------------------------------------------------------*/

Boolean fast_compare(void *s, void *d, int count);
#pragma aux fast_compare =										  \
	0x8B 0xCA		  /*	 mov	   ecx,edx		   */		  \
	0x80 0xE2 0x03	  /*	 and	   dl,3 		   */		  \
	0xC1 0xE9 0x02	  /*	 shr	   ecx,2		   */		  \
	0x33 0xC0		  /*	 xor	   eax,eax		   */		  \
	0xF3 0xA7		  /*	 repe cmpsd 			   */		  \
	0x75 0x07		  /*	 jne short done 		   */		  \
	0x8A 0xCA		  /*	 mov	   cl,dl		   */		  \
	0xF3 0xA6		  /*	 repe cmpsb 			   */		  \
	0x0F 0x94 0xC0	  /*	 sete	   al			   */		  \
					  /* done:						   */		  \
	parm [esi] [edi] [edx]										  \
	modify exact [esi edi eax ecx edx];

void init_raster(Raster *r)
/*****************************************************************************
 * init a raster structure before asking the driver to fill it in.
 ****************************************************************************/
{
	memset(r, 0, sizeof(*r));
}

void init_bytemap_raster(Raster *r, Vdevice *vd, Vmode_info *vm, PLANEPTR pptr)
/*****************************************************************************
 * fill in the fields of a bytemap raster, based on the device it's used with.
 ****************************************************************************/
{

	r->type 			= RT_BYTEMAP;
	r->pdepth			= vm->bits;
	r->width			= vm->width.actual;
	r->height			= vm->height.actual;
	r->hw.bm.num_planes = 1;
	r->hw.bm.bpr		= vm->width.actual;
	r->hw.bm.psize		= vm->width.actual * vm->height.actual;
	r->hw.bm.bp[0]		= pptr;
	pj_open_bytemap((Rasthdr *)r,(Bytemap *)r);

}

void make_ripple(Raster *r,
				 Ucoor width, Ucoor height,
				 Coor xstart, Coor ystart,
				 Pixel color_start,
				 short color_incr)
/*****************************************************************************
 * make a ripple pattern in a bytemap raster.
 ****************************************************************************/
{
PLANEPTR	ptr;
Pixel		color;
Coor		x;
Coor		y;


	color = color_start;
	ptr   = r->hw.bm.bp[0];

	for (y = ystart; y < height; y++)
		{
		ptr = xstart + r->hw.bm.bp[0] + (y * r->hw.bm.bpr);
		for (x = xstart; x < width; x++)
			{
			*ptr++ = color;
			color += color_incr;
			}
		}
}

Boolean verify_ripple(Raster *r,
					  Ucoor width, Ucoor height,
					  Coor xstart, Coor ystart,
					  Pixel color_start,
					  short color_incr)
/*****************************************************************************
 * verify the ripple pattern in a bytemap raster.
 ****************************************************************************/
{
PLANEPTR	ptr;
Pixel		color;
Coor		x;
Coor		y;


	color = color_start;
	ptr   = r->hw.bm.bp[0];

	for (y = ystart; y < height; y++)
		{
		ptr = xstart + r->hw.bm.bp[0] + (y * r->hw.bm.bpr);
		for (x = xstart; x < width; x++)
			{
			if (color != *ptr)
				{
				tcb.error_x = x;
				tcb.error_y = y;
				tcb.error_expected = color;
				tcb.error_found = *ptr;
				return FALSE;
				}
			++ptr;
			color += color_incr;
			}
		}
	return TRUE;
}

void draw_blitpattern(Raster *r, short fgcolor, short bgcolor)
/*****************************************************************************
 * draw a little pattern on the raster for testing the blit/tblit functions.
 ****************************************************************************/
{
	pj_set_rect(r, bgcolor, XPAT, YPAT, WPAT, HPAT); /* clean out pattern area */

	fgcolor += 2;
	pj_set_hline(r, fgcolor,  XPAT,   YPAT, 	59);  /* draw border around 	*/
	pj_set_hline(r, fgcolor,  XPAT,   YPAT+39, 59);  /* pattern area			*/
	pj_set_vline(r, fgcolor,  XPAT,   YPAT, 	39);
	pj_set_vline(r, fgcolor,  XPAT+59,YPAT,    40);

	pj_set_rect(r, ++fgcolor, XPAT+6,  YPAT+4,	19, 9); /* make the pattern,   */
	pj_set_rect(r, ++fgcolor, XPAT+36, YPAT+4,	19, 9); /* five little boxes   */
	pj_set_rect(r, ++fgcolor, XPAT+6,  YPAT+28, 19, 9); /* inside the area box */
	pj_set_rect(r, ++fgcolor, XPAT+36, YPAT+28, 19, 9);
	pj_set_rect(r, ++fgcolor, XPAT+24, YPAT+16, 13, 9);

	pj_put_dot(r, ++fgcolor, XPAT+30, YPAT+8);			/* scatter some little	*/
	pj_put_dot(r, ++fgcolor, XPAT+30, YPAT+32); 	/* dots inside the		*/
													/* pattern, to help see */
	pj_put_dot(r, ++fgcolor, XPAT+15, YPAT+20); 	/* that zoom blits are	*/
	pj_put_dot(r, ++fgcolor, XPAT+45, YPAT+20); 	/* working properly.	*/

	pj_put_dot(r, ++fgcolor, XPAT+30, YPAT+20);
}

Boolean verify_raster(Raster *sr, Raster *vr, Boolean do_logging)
/*****************************************************************************
 * verify that contents of raster sr exactly match bytemap raster vr.
 *	(sr and vr have to have identical width/height!)
 ****************************************************************************/
{
	int 	errors;
	int 	x;
	int 	y;
	int 	width = sr->width;
	int 	height = sr->height;
	Pixel	*pvrastpix = vr->hw.bm.bp[0];
	Pixel	segbuf[2048];

	/*------------------------------------------------------------------------
	 * if there is a playback verify buffer allocated, that means our caller
	 * is the playback testing.  in this case, we know we don't need logging,
	 * and we suck in the whole hardware raster then do a single fast compare
	 * of the buffer with the bytemap raster.  if they're equal, we return
	 * TRUE right away.  if unequal, we fall into our line-at-a-time routine
	 * which will report the x/y locations of the errors.  if the playback
	 * routines can't get enough memory for the playback buffer, the slower
	 * loop will be used, so that either way the comparison gets done.
	 *----------------------------------------------------------------------*/

	if (tcb.playback_verify_buffer != NULL) {
		pj_get_rectpix(sr, tcb.playback_verify_buffer, 0, 0, width, height);
		if (fast_compare(tcb.playback_verify_buffer, pvrastpix, width*height))
			return TRUE;
	}

	/*------------------------------------------------------------------------
	 * if logging was requested, do it.
	 *----------------------------------------------------------------------*/

	if (do_logging) {
		char	*rastname;
		if (sr->type == RT_BYTEMAP)
			rastname = "bytemap";
		else if (sr == &tcb.display_raster)
			rastname = "primary display";
		else
			rastname = "secondary hardware";

		log_progress("   Verifying contents of %s raster...\n", rastname);
	}

	/*------------------------------------------------------------------------
	 * if we got a non-equal comparison in the monolithic fast_compare(),
	 * or if this isn't a playback test comparison, do a line-at-a-time
	 * comparison, and report the x/y locations of differences.
	 *----------------------------------------------------------------------*/

	errors = 0;

	for (y = 0; y < height; ++y, pvrastpix += width) {
		pj_get_hseg(sr, segbuf, 0, y, width);
		if (!fast_compare(segbuf, pvrastpix, width)) {
			for (x = 0; x < width; ++x) {
				if (pvrastpix[x] != segbuf[x]) {
					log_verror(x, y, segbuf[x], pvrastpix[x]);
					if (++errors >= 10)
						goto ERROR_EXIT;
				}
			}
		}
	}

	if (do_logging)
		if (errors == 0)
			log_progress("   ...verification complete.\n");

ERROR_EXIT:

	return (errors == 0);

}
