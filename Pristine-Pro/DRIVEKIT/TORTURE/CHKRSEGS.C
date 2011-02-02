#include "torture.h"

#define SHRTSEG_WIDTH  37

#define  HFSEG_TIME_ITCOUNT 600
#define  HSSEG_TIME_ITCOUNT 2000

#define  VFSEG_TIME_ITCOUNT 300
#define  VSSEG_TIME_ITCOUNT 600

void test_segs(Raster *r)
/*****************************************************************************
 * test put_hseg, get_hseg, put_vseg, get_vseg.
 ****************************************************************************/
{
short		iteration_count;
short		iteration_limit;
short		do_verify;
Raster		*bmr	= &tcb.bytemap_raster;
Rastlib 	*rlib	= r->lib;
PLANEPTR	pixbuf;
Coor		x;
Coor		y;
Coor		seg1;
Coor		seg2;
Coor		seg3;
Coor		step;
Ucoor		width;
Ucoor		height;
Segfunc_t	put_hseg;
Segfunc_t	get_hseg;
Segfunc_t	put_vseg;
Segfunc_t	get_vseg;

	width  = r->width - 1;
	height = r->height - 1;
	do_verify = !tcb.timing_only_run;

	/*-----------------------------------------------------------------------
	 * hseg testing...
	 *	 if the driver didn't provide a routine, and we're not testing via
	 *	 generics, skip the hseg testing.
	 *---------------------------------------------------------------------*/

	if (is_generic(rlib,put_hseg) && !tcb.test_via_generics)
		{
		log_bypass("put_hseg() and get_hseg()");
		goto HSEG_DONE;
		}

	put_hseg = rlib->put_hseg;
	get_hseg = rlib->get_hseg;

	/*-----------------------------------------------------------------------
	 * test put_hseg using short segments...
	 *---------------------------------------------------------------------*/

	clear_screen();

	step = 1 + height / 8;
	seg1 = 0;
	seg2 = (width / 2) - (SHRTSEG_WIDTH / 2);
	seg3 = width - SHRTSEG_WIDTH;

	pixbuf = bmr->hw.bm.bp[0];

	iteration_limit = (tcb.timing_only_run) ? HSSEG_TIME_ITCOUNT : 1;

	log_start("Testing put_hseg() using short line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		make_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1);
		time_start();
		for (y = 0; y < height; y += step)
			{
			put_hseg(r, pixbuf, seg1, y, SHRTSEG_WIDTH);
			put_hseg(r, pixbuf, seg2, y, SHRTSEG_WIDTH);
			put_hseg(r, pixbuf, seg3, y, SHRTSEG_WIDTH);
			}

		y = height;
		put_hseg(r, pixbuf, seg1, y, SHRTSEG_WIDTH);
		put_hseg(r, pixbuf, seg2, y, SHRTSEG_WIDTH);
		put_hseg(r, pixbuf, seg3, y, SHRTSEG_WIDTH);
		time_end();
		}
	log_end("...put_hseg() short segment testing completed.\n\n");

	/*-----------------------------------------------------------------------
	 * test get_hseg using short segments...
	 *---------------------------------------------------------------------*/

	log_start("Testing get_hseg() using short line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		time_start();
		for (y = 0; y < height; y += step)
			{
			memset(pixbuf, 0, SHRTSEG_WIDTH);
			memset(pixbuf, 0, SHRTSEG_WIDTH);
			get_hseg(r, pixbuf, seg1, y, SHRTSEG_WIDTH);
			if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
				{
				x = seg1;
				goto HSEG_ERROR;
				}
			memset(pixbuf, 0, SHRTSEG_WIDTH);
			get_hseg(r, pixbuf, seg2, y, SHRTSEG_WIDTH);
			if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
				{
				x = seg2;
				goto HSEG_ERROR;
				}
			memset(pixbuf, 0, SHRTSEG_WIDTH);
			get_hseg(r, pixbuf, seg3, y, SHRTSEG_WIDTH);
			if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
				{
				x = seg3;
				goto HSEG_ERROR;
				}
			}

		 y = height;
		 memset(pixbuf, 0, SHRTSEG_WIDTH);
		 get_hseg(r, pixbuf, seg1, y, SHRTSEG_WIDTH);
		 if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
			 {
			 x = seg1;
			 goto HSEG_ERROR;
			 }
		 memset(pixbuf, 0, SHRTSEG_WIDTH);
		 get_hseg(r, pixbuf, seg2, y, SHRTSEG_WIDTH);
		 if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
			 {
			 x = seg2;
			 goto HSEG_ERROR;
			 }
		 memset(pixbuf, 0, SHRTSEG_WIDTH);
		 get_hseg(r, pixbuf, seg3, y, SHRTSEG_WIDTH);
		 if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
			 {
			 x = seg3;
			 goto HSEG_ERROR;
			 }
		 time_end();
		}

	log_end("...get_hseg() short segment testing completed.\n\n");

	if (!single_step())
		return;

	/*-----------------------------------------------------------------------
	 * test put_hseg using full-line segments...
	 *---------------------------------------------------------------------*/

	pixbuf = bmr->hw.bm.bp[0];
	x = 0;
	iteration_limit = (tcb.timing_only_run) ? HFSEG_TIME_ITCOUNT : 1;

	log_start("Testing put_hseg() using full-line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		make_ripple(bmr, width, 1, 0, 0, iteration_count, 1);
		time_start();
		for (y = 0; y < height; y += step)
			{
			put_hseg(r, pixbuf, x, y, width);
			}
		put_hseg(r, pixbuf, x, height, width);
		time_end();
		}

	log_end("...put_hseg() full-line segment testing completed.\n\n");

	/*-----------------------------------------------------------------------
	 * test get_hseg using full-line segments...
	 *---------------------------------------------------------------------*/

	log_start("Testing get_hseg() using full-line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		for (y = 0; y < height; y += step)
			{
			memset(pixbuf, 0, width);
			time_it(
				get_hseg(r, pixbuf, x, y, width);
				);
			if (do_verify && !verify_ripple(bmr, width, 1, 0, 0, iteration_count, 1))
				goto HSEG_ERROR;
			}
		memset(pixbuf, 0, width);
		memset(pixbuf, 0, width);
		time_it(
			get_hseg(r, pixbuf, x, height, width);
			);
		if (do_verify && !verify_ripple(bmr, width, 1, 0, 0, iteration_count, 1))
			goto HSEG_ERROR;
		}

	log_end("...get_hseg() full-line segment testing completed.\n\n");

	single_step();

	goto HSEG_DONE;

HSEG_ERROR:

	log_verror(x+tcb.error_x, y+tcb.error_y, tcb.error_found, tcb.error_expected);
	single_step();
	return;

HSEG_DONE:

	/*-----------------------------------------------------------------------
	 * vseg testing...
	 *	 if the driver didn't provide a routine, and we're not testing via
	 *	 generics, skip the vseg testing.
	 *---------------------------------------------------------------------*/

	if (is_generic(rlib,put_vseg) && !tcb.test_via_generics)
		{
		log_bypass("put_vseg() and get_vseg()");
		goto VSEG_DONE;
		}

	put_vseg = rlib->put_vseg;
	get_vseg = rlib->get_vseg;

	clear_screen();

	/*-----------------------------------------------------------------------
	 * test put_vseg using short segments...
	 *---------------------------------------------------------------------*/

	step = 1 + width / 8;
	seg1 = 0;
	seg2 = (height / 2) - (SHRTSEG_WIDTH / 2);
	seg3 = height - SHRTSEG_WIDTH;
	iteration_limit = (tcb.timing_only_run) ? VSSEG_TIME_ITCOUNT : 1;

	pixbuf = bmr->hw.bm.bp[0];

	log_start("Testing put_vseg() using short line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		make_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1);
		time_start();
		for (x = 0; x < width; x += step)
			{
			put_vseg(r, pixbuf, x, seg1, SHRTSEG_WIDTH);
			put_vseg(r, pixbuf, x, seg2, SHRTSEG_WIDTH);
			put_vseg(r, pixbuf, x, seg3, SHRTSEG_WIDTH);
			}

		x = width;
		put_vseg(r, pixbuf, x, seg1, SHRTSEG_WIDTH);
		put_vseg(r, pixbuf, x, seg2, SHRTSEG_WIDTH);
		put_vseg(r, pixbuf, x, seg3, SHRTSEG_WIDTH);
		time_end();
		}

	log_end("...put_vseg() short segment testing completed.\n\n");

	/*-----------------------------------------------------------------------
	 * test get_vseg using short segments...
	 *---------------------------------------------------------------------*/

	log_start("Testing get_vseg() using short line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		time_start();
		for (x = 0; x < width; x += step)
			{
			memset(pixbuf, 0, SHRTSEG_WIDTH);
			get_vseg(r, pixbuf, x, seg1, SHRTSEG_WIDTH);
			if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
				{
				y = seg1;
				goto VSEG_ERROR;
				}
			memset(pixbuf, 0, SHRTSEG_WIDTH);
			get_vseg(r, pixbuf, x, seg2, SHRTSEG_WIDTH);
			if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
				{
				y = seg2;
				goto VSEG_ERROR;
				}
			memset(pixbuf, 0, SHRTSEG_WIDTH);
			get_vseg(r, pixbuf, x, seg3, SHRTSEG_WIDTH);
			if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
				{
				y = seg3;
				goto VSEG_ERROR;
				}
			}

		x = width;
		memset(pixbuf, 0, SHRTSEG_WIDTH);
		get_vseg(r, pixbuf, x, seg1, SHRTSEG_WIDTH);
		if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
			{
			y = seg1;
			goto VSEG_ERROR;
			}
		memset(pixbuf, 0, SHRTSEG_WIDTH);
		get_vseg(r, pixbuf, x, seg2, SHRTSEG_WIDTH);
		if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
			{
			y = seg2;
			goto VSEG_ERROR;
			}
		memset(pixbuf, 0, SHRTSEG_WIDTH);
		get_vseg(r, pixbuf, x, seg3, SHRTSEG_WIDTH);
		if (do_verify && !verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, iteration_count, 1))
			{
			y = seg3;
			goto VSEG_ERROR;
			}
		time_end();
		}

	log_end("...get_vseg() short segment testing completed.\n\n");

	if (!single_step())
		return;

	/*-----------------------------------------------------------------------
	 * test put_vseg using full-line segments...
	 *---------------------------------------------------------------------*/

	pixbuf = bmr->hw.bm.bp[0];
	y = 0;
	iteration_limit = (tcb.timing_only_run) ? VFSEG_TIME_ITCOUNT : 1;

	log_start("Testing put_vseg() using full-line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		make_ripple(bmr, height, 1, 0, 0, iteration_count, 1);
		time_start();
		for (x = 0; x < width; x += step)
			{
			put_vseg(r, pixbuf, x, y, height);
			}
		x = width;
		put_vseg(r, pixbuf, x, y, height);
		time_end();
		}

	log_end("...put_vseg() full-line segment testing completed.\n\n");

	/*-----------------------------------------------------------------------
	 * test get_vseg using full-line segments...
	 *---------------------------------------------------------------------*/

	log_start("Testing get_vseg() using full-line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		make_ripple(bmr, height, 1, 0, 0, iteration_count, 1);
		for (x = 0; x < width; x += step)
			{
			memset(pixbuf, 0, height);
			time_it(
				get_vseg(r, pixbuf, x, y, height);
				);
			if (do_verify && !verify_ripple(bmr, height, 1, 0, 0, iteration_count, 1))
				goto VSEG_ERROR;
			}

		x = width;
		memset(pixbuf, 0, height);
		time_it(
			get_vseg(r, pixbuf, x, y, height);
			);
		if (do_verify && !verify_ripple(bmr, height, 1, 0, 0, iteration_count, 1))
			goto VSEG_ERROR;
		}

	log_end("...get_vseg() full-line segment testing completed.\n\n");

	single_step();

	goto VSEG_DONE;

VSEG_ERROR:

	log_verror(x, y+tcb.error_x, tcb.error_found, tcb.error_expected);
	single_step();
	return;


VSEG_DONE:

	return;
}
