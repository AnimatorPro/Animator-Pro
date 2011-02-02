#include "torture.h"

#define RPIX_TIME_ITCOUNT  10
#define SRECT_TIME_ITCOUNT 15
#define SRAST_TIME_ITCOUNT 30
#define XRECT_TIME_ITCOUNT 15

static void make_buffer(Pixel *pixbuf, long length, Pixel color)
{
	while(length--)
		*pixbuf++ = color;
}

static Boolean verify_buffer(Pixel *pixbuf, int width, int height, Pixel color)
{
	int x;
	int y;

	for (y = 0; y < height; ++y)
		for (x = 0; x < width; ++x, ++pixbuf)
			if (*pixbuf != color)
				{
				tcb.error_x = x;
				tcb.error_y = y;
				tcb.error_expected = color;
				tcb.error_found = *pixbuf;
				return FALSE;
				}
	return TRUE;
}

void test_rectpix(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Rastlib 	*rlib = r->lib;
Raster		*bmr = &tcb.bytemap_raster;
PLANEPTR	pixbuf = bmr->hw.bm.bp[0];
Coor		x;
Coor		y;
Coor		w;
Coor		h;
Ucoor		width;
Ucoor		height;
Pixel		color;
short		do_verify;
short		iteration_count;
short		iteration_limit;
Rpixfunc_t	put_rectpix;
Rpixfunc_t	get_rectpix;


	width = r->width;
	height = r->height;
	do_verify = !tcb.timing_only_run;

	/*-----------------------------------------------------------------------
	 * put_rectpix testing...
	 *	 if the driver didn't provide a routine, and we're not testing via
	 *	 generics, skip the testing.
	 *---------------------------------------------------------------------*/

	if (is_generic(rlib,put_rectpix) && !tcb.test_via_generics)
		{
		log_bypass("put_rectpix() and get_rectpix()");
		goto RPIX_DONE;
		}

	put_rectpix = rlib->put_rectpix;
	get_rectpix = rlib->get_rectpix;

	/*-----------------------------------------------------------------------
	 * test put_rectpix...
	 *---------------------------------------------------------------------*/

	clear_screen();

	iteration_limit = (tcb.timing_only_run) ? RPIX_TIME_ITCOUNT : 1;

	log_start("Testing put_rectpix()...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		color = 1 + iteration_count;
		make_buffer(pixbuf, width*height, color++);
		time_it(
			put_rectpix(r, pixbuf, 0, 0, width, height);
			);
		for (y = 0, h=height/4; y+h <= (height/2); y += h)
			for (x=width/2, w=width/4; x+w <= width; x+=w)
				{
				make_buffer(pixbuf, w*h, color++);
				time_it(
					put_rectpix(r, pixbuf, x, y, w, h);
					);
				}
		for (y = height/2, h=height/8; y+h <= height; y+=h)
			for (x = 0, w=width/8; x+w <= width/2; x+=w)
				{
				make_buffer(pixbuf, w*h, color++);
				time_it(
					put_rectpix(r, pixbuf, x, y, w, h);
					);
				}
		for (y = height/2, h=height/16; y+h <= height; y+=h)
			for (x = width/2, w=width/16; x+w <= width; x+=w)
				{
				make_buffer(pixbuf, w*h, color++);
				time_it(
					put_rectpix(r, pixbuf, x, y, w, h);
					);
				}
		}

	log_end("...testing of put_rectpix() completed.\n\n");

	/*-----------------------------------------------------------------------
	 * test get_rectpix...
	 *	note that unlike most other timing tests, in this section we
	 *	actually call the verify routine even if we are only getting
	 *	timings, we just don't crap out on a verify error.  this is because
	 *	the drawing process above calls make_buffer() for every rectangle
	 *	drawn, so to even out the times, we need to run verify_buffer() on
	 *	every rectangle read.
	 * note: the above timing issue has basically been fixed, by wrapping
	 *		 time_it() calls around the get_rectpix() calls.
	 *---------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RPIX_TIME_ITCOUNT : 1;

	log_start("Testing get_rectpix()...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		color = 1 + iteration_count;
		x=0;
		y=0;
		w=width;
		h=height;
		if (do_verify)	/* if we're verifying, we can't check parts of the  */
			{			/* big rectangle that were overwritten by the small */
			w /=2;		/* rectangles. if we're timing, we have to obtain   */
			h /=2;		/* & psuedo-verify the full screen or else we end up*/
			}			/* making get_rectpix look faster than put_rectpix. */
		memset(pixbuf, 0, w*h);
		time_it(
			get_rectpix(r, pixbuf, x, y, w, h);
			);
		if (!verify_buffer(pixbuf, w, h, color++))
			if (do_verify)
				goto GET_RPIX_ERROR;
		for (y = 0, h=height/4; y+h <= (height/2); y += h)
			for (x=width/2, w=width/4; x+w <= width; x+=w)
				{
				memset(pixbuf, 0, w*h);
				time_it(
					get_rectpix(r, pixbuf, x, y, w, h);
					);
				if (!verify_buffer(pixbuf, w, h, color++))
					if (do_verify)
						goto GET_RPIX_ERROR;
				}
		for (y = height/2, h=height/8; y+h <= height; y+=h)
			for (x = 0, w=width/8; x+w <= width/2; x+=w)
				{
				memset(pixbuf, 0, w*h);
				time_it(
					get_rectpix(r, pixbuf, x, y, w, h);
					);
				if (!verify_buffer(pixbuf, w, h, color++))
					if (do_verify)
						goto GET_RPIX_ERROR;
				}
		for (y = height/2, h=height/16; y+h <= height; y+=h)
			for (x = width/2, w=width/16; x+w <= width; x+=w)
				{
				memset(pixbuf, 0, w*h);
				time_it(
					get_rectpix(r, pixbuf, x, y, w, h);
					);
				if (!verify_buffer(pixbuf, w, h, color++))
					if (do_verify)
						goto GET_RPIX_ERROR;
				}
		}

	log_end("...testing of get_rectpix() completed.\n\n");

	single_step();
	goto RPIX_DONE;

GET_RPIX_ERROR:

	log_verror(x+tcb.error_x, y+tcb.error_y, tcb.error_found, tcb.error_expected);
	single_step();

RPIX_DONE:

	return;
}

void test_set_rect(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Rastlib 	 *rlib = r->lib;
Raster		 *bmr = &tcb.bytemap_raster;
PLANEPTR	 pixbuf = bmr->hw.bm.bp[0];
Coor		 x;
Coor		 y;
Coor		 w;
Coor		 h;
Ucoor		 width;
Ucoor		 height;
Pixel		 color;
short		 do_verify;
short		 iteration_count;
short		 iteration_limit;
Sxrectfunc_t set_rect;
Rpixfunc_t	 get_rectpix;

	width = r->width;
	height = r->height;
	do_verify = !tcb.timing_only_run;

	/*-----------------------------------------------------------------------
	 * set_rect testing...
	 *	 if the driver didn't provide a routine, and we're not testing via
	 *	 generics, skip the testing.
	 *---------------------------------------------------------------------*/

	if (is_generic(rlib,set_rect) && !tcb.test_via_generics)
		{
		log_bypass("set_rect()");
		goto SRECT_DONE;
		}
	set_rect = rlib->set_rect;
	get_rectpix = rlib->get_rectpix;

	/*-----------------------------------------------------------------------
	 * test set_rect...
	 *---------------------------------------------------------------------*/

	clear_screen();

	iteration_limit = (tcb.timing_only_run) ? SRECT_TIME_ITCOUNT : 1;

	log_start("Testing set_rect()...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		color = 1 + iteration_count;
		set_rect(r, color++, 0, 0, width, height);
		for (y = 0, h=height/4; y+h <= (height/2); y += h)
			for (x=width/2, w=width/4; x+w <= width; x+=w)
				{
				set_rect(r, color++, x, y, w, h);
				}
		for (y = height/2, h=height/8; y+h <= height; y+=h)
			for (x = 0, w=width/8; x+w <= width/2; x+=w)
				{
				set_rect(r, color++, x, y, w, h);
				}
		for (y = height/2, h=height/16; y+h <= height; y+=h)
			for (x = width/2, w=width/16; x+w <= width; x+=w)
				{
				set_rect(r, color++, x, y, w, h);
				}
		}

	log_end("...testing of set_rect() completed.\n\n");

	/*-----------------------------------------------------------------------
	 * verify results of set_rect
	 *---------------------------------------------------------------------*/

	if (!do_verify)
		goto SRECT_DONE;

	log_start("Verifying results of set_rect() by reading with get_rectpix()...\n");

	color = 1;
	x=0;
	y=0;
	w=width/2;
	h=height/2;
	memset(pixbuf, 0, w*h);
	time_it(
		get_rectpix(r, pixbuf, x, y, w, h);
		);
	if (!verify_buffer(pixbuf, w, h, color++))
		goto GET_SRECT_ERROR;
	for (y = 0, h=height/4; y+h <= (height/2); y += h)
		for (x=width/2, w=width/4; x+w <= width; x+=w)
			{
			memset(pixbuf, 0, w*h);
			time_it(
				get_rectpix(r, pixbuf, x, y, w, h);
				);
			if (!verify_buffer(pixbuf, w, h, color++))
				goto GET_SRECT_ERROR;
			}
	for (y = height/2, h=height/8; y+h <= height; y+=h)
		for (x = 0, w=width/8; x+w <= width/2; x+=w)
			{
			memset(pixbuf, 0, w*h);
			time_it(
				get_rectpix(r, pixbuf, x, y, w, h);
				);
			if (!verify_buffer(pixbuf, w, h, color++))
				goto GET_SRECT_ERROR;
			}
	for (y = height/2, h=height/16; y+h <= height; y+=h)
		for (x = width/2, w=width/16; x+w <= width; x+=w)
			{
			memset(pixbuf, 0, w*h);
			time_it(
				get_rectpix(r, pixbuf, x, y, w, h);
				);
			if (!verify_buffer(pixbuf, w, h, color++))
				goto GET_SRECT_ERROR;
			}

	log_progress("...verification completed.\n\n");

	single_step();
	goto SRECT_DONE;

GET_SRECT_ERROR:

	log_verror(x+tcb.error_x, y+tcb.error_y, tcb.error_found, tcb.error_expected);
	single_step();

SRECT_DONE:

	return;
}

void test_set_rast(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Rastlib 	 *rlib = r->lib;
Raster		 *bmr = &tcb.bytemap_raster;
PLANEPTR	 pixbuf = bmr->hw.bm.bp[0];
Ucoor		 width;
Ucoor		 height;
Pixel		 color;
short		 do_verify;
short		 iteration_count;
short		 iteration_limit;
Srastfunc_t  set_rast;
Rpixfunc_t	 get_rectpix;

	width = r->width;
	height = r->height;
	do_verify = !tcb.timing_only_run;

	/*-----------------------------------------------------------------------
	 * set_rast testing...
	 *	 if the driver didn't provide a routine, and we're not testing via
	 *	 generics, skip the testing.
	 *---------------------------------------------------------------------*/

	if (is_generic(rlib,set_rast) && !tcb.test_via_generics)
		{
		log_bypass("set_rast()");
		goto SRAST_DONE;
		}

	set_rast = rlib->set_rast;
	get_rectpix = rlib->get_rectpix;

	/*-----------------------------------------------------------------------
	 * test set_rast...
	 *---------------------------------------------------------------------*/

	clear_screen();

	iteration_limit = (tcb.timing_only_run) ? SRAST_TIME_ITCOUNT : 1;

	log_start("Testing set_rast()...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		color = 1 + iteration_count;
		set_rast(r, color);
		}

	log_end("...testing of set_rast() completed.\n\n");

	/*-----------------------------------------------------------------------
	 * verify results of set_rast
	 *---------------------------------------------------------------------*/

	if (!do_verify)
		goto SRAST_DONE;

	memset(pixbuf, 0, width*height);

	log_start("Verifying results of set_rast() by reading with get_rectpix()...\n");

	color = 1;
	get_rectpix(r, pixbuf, 0, 0, width, height);
	if (!verify_buffer(pixbuf, width, height, color))
		{
		log_verror(tcb.error_x, tcb.error_y, tcb.error_found, tcb.error_expected);
		single_step();
		goto SRAST_DONE;
		}
	log_progress("...verification completed.\n\n");

	single_step();

SRAST_DONE:

	return;
}

void test_xor_rect(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Rastlib 	 *rlib = r->lib;
Raster		 *bmr = &tcb.bytemap_raster;
PLANEPTR	 pixbuf = bmr->hw.bm.bp[0];
Coor		 x;
Coor		 y;
Coor		 w;
Coor		 h;
Ucoor		 width;
Ucoor		 height;
Pixel		 color;
short		 do_verify;
short		 iteration_count;
short		 iteration_limit;
Sxrectfunc_t xor_rect;
Rpixfunc_t	 get_rectpix;

	width = r->width;
	height = r->height;
	do_verify = !tcb.timing_only_run;

	/*-----------------------------------------------------------------------
	 * xor_rect testing...
	 *	 if the driver didn't provide a routine, and we're not testing via
	 *	 generics, skip the testing.
	 *---------------------------------------------------------------------*/

	if (is_generic(rlib,xor_rect) && !tcb.test_via_generics)
		{
		log_bypass("xor_rect()");
		goto XRECT_DONE;
		}
	xor_rect = rlib->xor_rect;
	get_rectpix = rlib->get_rectpix;

	/*-----------------------------------------------------------------------
	 * test xor_rect...
	 *---------------------------------------------------------------------*/

	clear_screen();

	iteration_limit = (tcb.timing_only_run) ? XRECT_TIME_ITCOUNT : 1;

	log_start("Testing xor_rect()...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		color = 1 + iteration_count;
		xor_rect(r, color++, 0, 0, width, height);
		for (y = 0, h=height/4; y+h <= (height/2); y += h)
			for (x=width/2, w=width/4; x+w <= width; x+=w)
				{
				xor_rect(r, color++, x, y, w, h);
				}
		for (y = height/2, h=height/8; y+h <= height; y+=h)
			for (x = 0, w=width/8; x+w <= width/2; x+=w)
				{
				xor_rect(r, color++, x, y, w, h);
				}
		for (y = height/2, h=height/16; y+h <= height; y+=h)
			for (x = width/2, w=width/16; x+w <= width; x+=w)
				{
				xor_rect(r, color++, x, y, w, h);
				}
		}

	log_end("...testing of xor_rect() completed.\n\n");

	/*-----------------------------------------------------------------------
	 * verify results of xor_rect
	 *---------------------------------------------------------------------*/

	if (!do_verify)
		goto XRECT_DONE;

	if (!single_step())
		return;

	log_start("Verifying results of xor_rect() by reading with get_rectpix()...\n");

	color = 1;
	x=0;
	y=0;
	w=width/2;
	h=height/2;
	memset(pixbuf, 0, w*h);
	get_rectpix(r, pixbuf, x, y, w, h);
	if (!verify_buffer(pixbuf, w, h, color++))
		goto GET_XRECT_ERROR;
	for (y = 0, h=height/4; y+h <= (height/2); y += h)
		for (x=width/2, w=width/4; x+w <= width; x+=w)
			{
			memset(pixbuf, 0, w*h);
			get_rectpix(r, pixbuf, x, y, w, h);
			if (!verify_buffer(pixbuf, w, h, 1^color++))
				goto GET_XRECT_ERROR;
			}
	for (y = height/2, h=height/8; y+h <= height; y+=h)
		for (x = 0, w=width/8; x+w <= width/2; x+=w)
			{
			memset(pixbuf, 0, w*h);
			get_rectpix(r, pixbuf, x, y, w, h);
			if (!verify_buffer(pixbuf, w, h, 1^color++))
				goto GET_XRECT_ERROR;
			}
	for (y = height/2, h=height/16; y+h <= height; y+=h)
		for (x = width/2, w=width/16; x+w <= width; x+=w)
			{
			memset(pixbuf, 0, w*h);
			get_rectpix(r, pixbuf, x, y, w, h);
			if (!verify_buffer(pixbuf, w, h, 1^color++))
				goto GET_XRECT_ERROR;
			}

	log_progress("...verification completed.\n\n");

	single_step();
	goto XRECT_DONE;

GET_XRECT_ERROR:

	log_verror(x+tcb.error_x, y+tcb.error_y, tcb.error_found, tcb.error_expected);
	single_step();

XRECT_DONE:

	return;
}
