#include "torture.h"

#define SHRTSEG_WIDTH 35

#define HSLINE_TIME_ITCOUNT 3000
#define HFLINE_TIME_ITCOUNT 1000

#define VSLINE_TIME_ITCOUNT 1000
#define VFLINE_TIME_ITCOUNT 400

void hline_clearscreen(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
Raster		*r = &tcb.display_raster;
Coor		y;
Ucoor		width;
Ucoor		height;
Linefunc_t	set_hline;

	width  = r->width;
	height = r->height;

	set_hline = r->lib->set_hline;

	for (y = 0; y < height; y++)
		set_hline(r, 0, 0, y, width);
}

void test_lines(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Raster		*bmr	= &tcb.bytemap_raster;
Rastlib 	*rlib	= r->lib;
PLANEPTR	pixbuf	= bmr->hw.bm.bp[0];
Coor		x;
Coor		y;
Coor		seg1;
Coor		seg2;
Coor		seg3;
Coor		step;
Ucoor		width;
Ucoor		height;
Pixel		color;
short		verify;
short		iteration_count;
short		iteration_limit;
Segfunc_t	get_hseg;
Segfunc_t	get_vseg;
Linefunc_t	set_hline;
Linefunc_t	set_vline;

	width  = r->width - 1;
	height = r->height - 1;
	verify = !tcb.timing_only_run;

	/*-----------------------------------------------------------------------
	 * hline testing...
	 *	 if the driver didn't provide a routine, and we're not testing via
	 *	 generics, skip the hline testing.
	 *---------------------------------------------------------------------*/

	if (is_generic(rlib,set_hline) && !tcb.test_via_generics)
		{
		log_bypass("set_hline()");
		goto HLINE_DONE;
		}

	set_hline = rlib->set_hline;
	get_hseg  = rlib->get_hseg;

	/*-----------------------------------------------------------------------
	 * test set_hline using short segments...
	 *---------------------------------------------------------------------*/

	clear_screen();

	step = 1 + height / 8;
	seg1 = 0;
	seg2 = (width / 2) - (SHRTSEG_WIDTH / 2);
	seg3 = width - SHRTSEG_WIDTH;

	iteration_limit = (tcb.timing_only_run) ? HSLINE_TIME_ITCOUNT : 1;

	log_start("Testing set_hline() using short line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		color = 1 + iteration_count;
		for (y = 0; y < height; y += step)
			{
			set_hline(r, color, seg1, y, SHRTSEG_WIDTH);
			set_hline(r, color, seg2, y, SHRTSEG_WIDTH);
			set_hline(r, color, seg3, y, SHRTSEG_WIDTH);
			}
		y = height;
		set_hline(r, color, seg1, y, SHRTSEG_WIDTH);
		set_hline(r, color, seg2, y, SHRTSEG_WIDTH);
		set_hline(r, color, seg3, y, SHRTSEG_WIDTH);
		}

	log_end("...set_hline() short segment testing completed.\n\n");

	/*-----------------------------------------------------------------------
	 * verify results of short segment lines...
	 *---------------------------------------------------------------------*/

	if (!verify)
		goto HLINE_BYPASS_VERIFY_SHORT;

	log_start("Verifying results of set_hline() by reading with get_hseg()...\n");

	for (y = 0; y < height; y += step)
		{
		get_hseg(r, pixbuf, seg1, y, SHRTSEG_WIDTH);
		if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
			{
			x = seg1;
			goto HLINE_ERROR;
			}
		get_hseg(r, pixbuf, seg2, y, SHRTSEG_WIDTH);
		if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
			{
			x = seg2;
			goto HLINE_ERROR;
			}
		get_hseg(r, pixbuf, seg3, y, SHRTSEG_WIDTH);
		if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
			{
			x = seg3;
			goto HLINE_ERROR;
			}
		}

	y = height;
	get_hseg(r, pixbuf, seg1, y, SHRTSEG_WIDTH);
	if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
		{
		x = seg1;
		goto HLINE_ERROR;
		}
	get_hseg(r, pixbuf, seg2, y, SHRTSEG_WIDTH);
	if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
		{
		x = seg2;
		goto HLINE_ERROR;
		}
	get_hseg(r, pixbuf, seg3, y, SHRTSEG_WIDTH);
	if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
		{
		x = seg3;
		goto HLINE_ERROR;
		}

	log_progress("...set_hline() short segment verification completed.\n\n");

HLINE_BYPASS_VERIFY_SHORT:

	if (!single_step())
		return;

	/*-----------------------------------------------------------------------
	 * test set_hline using full-line segments...
	 *---------------------------------------------------------------------*/

	x = 0;

	iteration_limit = (tcb.timing_only_run) ? HFLINE_TIME_ITCOUNT : 1;

	log_start("Testing set_hline() using full-line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		color = 1 + iteration_count;
		for (y = 0; y < height; y += step)
			set_hline(r, color, x, y, width);

		set_hline(r, color, x, height, width);
		}

	log_end("...set_hline() full-line segment testing completed.\n\n");

	/*-----------------------------------------------------------------------
	 * verify results of full-line segments...
	 *---------------------------------------------------------------------*/

	if (!verify)
		goto HLINE_BYPASS_VERIFY_FULL;

	log_start("Verifying results of set_hline() by reading with get_hseg()...\n");

	for (y = 0; y < height; y += step)
		{
		get_hseg(r, pixbuf, x, y, width);
		if (!verify_ripple(bmr, width, 1, 0, 0, color, 0))
			goto HLINE_ERROR;
		}

	get_hseg(r, pixbuf, x, height, width);
	if (!verify_ripple(bmr, width, 1, 0, 0, color, 0))
		goto HLINE_ERROR;

	log_progress("...set_hline() full-line segment verification completed.\n\n");

HLINE_BYPASS_VERIFY_FULL:

	if (!single_step())
		return;

	/*-----------------------------------------------------------------------
	 * now we can switch the safe clearscreen to a somewhat faster routine...
	 *---------------------------------------------------------------------*/

	tcb.safe_clear_screen = hline_clearscreen;

	goto HLINE_DONE;

HLINE_ERROR:

	log_verror(x+tcb.error_x, y+tcb.error_y, tcb.error_found, tcb.error_expected);
	single_step();
	return;

HLINE_DONE:

	/*-----------------------------------------------------------------------
	 * vline testing...
	 *	 if the driver didn't provide a routine, and we're not testing via
	 *	 generics, skip the vline testing.
	 *---------------------------------------------------------------------*/

	if (is_generic(rlib,set_vline) && !tcb.test_via_generics)
		{
		log_bypass("set_vline()");
		goto VLINE_DONE;
		}

	set_vline = rlib->set_vline;
	get_vseg  = rlib->get_vseg;

	clear_screen();

	/*-----------------------------------------------------------------------
	 * test set_vline using short segments...
	 *---------------------------------------------------------------------*/

	step = 1 + width / 8;
	seg1 = 0;
	seg2 = (height / 2) - (SHRTSEG_WIDTH / 2);
	seg3 = height - SHRTSEG_WIDTH;

	iteration_limit = (tcb.timing_only_run) ? VSLINE_TIME_ITCOUNT : 1;

	log_start("Testing set_vline() using short line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		color = 1 + iteration_count;
		for (x = 0; x < width; x += step)
			{
			set_vline(r, color, x, seg1, SHRTSEG_WIDTH);
			set_vline(r, color, x, seg2, SHRTSEG_WIDTH);
			set_vline(r, color, x, seg3, SHRTSEG_WIDTH);
			}

		x = width;
		set_vline(r, color, x, seg1, SHRTSEG_WIDTH);
		set_vline(r, color, x, seg2, SHRTSEG_WIDTH);
		set_vline(r, color, x, seg3, SHRTSEG_WIDTH);
		}

	log_end("...set_vline() short segment testing completed.\n\n");

	/*-----------------------------------------------------------------------
	 * verify results of short segments...
	 *---------------------------------------------------------------------*/

	if (!verify)
		goto VLINE_BYPASS_VERIFY_SHORT;

	log_start("Verifying results of set_vline() by reading with get_vseg()...\n");

	for (x = 0; x < width; x += step)
		{
		get_vseg(r, pixbuf, x, seg1, SHRTSEG_WIDTH);
		if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
			{
			y = seg1;
			goto VLINE_ERROR;
			}
		get_vseg(r, pixbuf, x, seg2, SHRTSEG_WIDTH);
		if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
			{
			y = seg2;
			goto VLINE_ERROR;
			}
		get_vseg(r, pixbuf, x, seg3, SHRTSEG_WIDTH);
		if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
			{
			y = seg3;
			goto VLINE_ERROR;
			}
		}

	x = width;
	get_vseg(r, pixbuf, x, seg1, SHRTSEG_WIDTH);
	if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
		{
		y = seg1;
		goto VLINE_ERROR;
		}
	get_vseg(r, pixbuf, x, seg2, SHRTSEG_WIDTH);
	if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
		{
		y = seg2;
		goto VLINE_ERROR;
		}
	get_vseg(r, pixbuf, x, seg3, SHRTSEG_WIDTH);
	if (!verify_ripple(bmr, SHRTSEG_WIDTH, 1, 0, 0, color, 0))
		{
		y = seg3;
		goto VLINE_ERROR;
		}

	log_progress("...set_vline() short segment verification completed.\n\n");

VLINE_BYPASS_VERIFY_SHORT:

	if (!single_step())
		return;

	/*-----------------------------------------------------------------------
	 * test set_vline using full-line segments...
	 *---------------------------------------------------------------------*/

	y = 0;

	iteration_limit = (tcb.timing_only_run) ? VFLINE_TIME_ITCOUNT : 1;

	log_start("Testing set_vline() using full-line segments...\n");

	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		color = 1 + iteration_count;
		for (x = 0; x < width; x += step)
			set_vline(r, color, x, y, height);

		x = width;
		set_vline(r, color, x, y, height);
		}

	log_end("...set_vline() full-line segment testing completed.\n\n");

	/*-----------------------------------------------------------------------
	 * verify results of full-line segments...
	 *---------------------------------------------------------------------*/

	if (!verify)
		goto VLINE_BYPASS_VERIFY_FULL;

	log_start("Verifying results of set_vline() by reading with get_vseg()...\n");

	for (x = 0; x < width; x += step)
		{
		get_vseg(r, pixbuf, x, y, height);
		if (!verify_ripple(bmr, width, 1, 0, 0, color, 0))
			goto VLINE_ERROR;
		}

	x = width;
	get_vseg(r, pixbuf, x, y, height);
	if (!verify_ripple(bmr, width, 1, 0, 0, color, 0))
		goto VLINE_ERROR;

	log_progress("...set_vline() full-line segment verification completed.\n\n");

VLINE_BYPASS_VERIFY_FULL:

	if (!single_step())
		return;

	goto VLINE_DONE;

VLINE_ERROR:

	log_verror(x+tcb.error_x, y+tcb.error_y, tcb.error_found, tcb.error_expected);
	single_step();
	return;

VLINE_DONE:

	return;

}
