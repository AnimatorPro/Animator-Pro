#include "torture.h"

#define RSWAP_TIME_ITCOUNT 20

static void do_swaps(Raster *sr, Raster *dr, short color)
/*****************************************************************************
 *
 ****************************************************************************/
{
Coor			x;
Coor			y;
Ucoor			width;
Ucoor			height;

	width  = dr->width;
	height = dr->height;

	draw_blitpattern(sr, color, 0);
	time_it(
		pj_swaprect(sr, XPAT, YPAT, dr, 		  0, 		   0, WPAT, HPAT);
		);

	draw_blitpattern(sr, color, 0);
	time_it(
		pj_swaprect(sr, XPAT, YPAT, dr, width-WPAT,   	       0, WPAT, HPAT);
		);

	draw_blitpattern(sr, color, 0);
	time_it(
		pj_swaprect(sr, XPAT, YPAT, dr, width-WPAT, height-HPAT, WPAT, HPAT);
		);

	draw_blitpattern(sr, color, 0);
	time_it(
		pj_swaprect(sr, XPAT, YPAT, dr,          0, height-HPAT, WPAT, HPAT);
		);

	y = height / 2 - HPAT / 2;
	for (x = (width%WPAT)/2; x+WPAT <= width; x+= WPAT)
		{
		draw_blitpattern(sr, color, 0);
		time_it(
			pj_swaprect(sr, XPAT, YPAT, dr, x, y, WPAT, HPAT);
			);
		}

	x = width / 2 - WPAT / 2;
	for (y = (height%HPAT)/2; y+HPAT <= height; y+= HPAT)
		{
		draw_blitpattern(sr, color, 0);
		time_it(
			pj_swaprect(sr, XPAT, YPAT, dr, x, y, WPAT, HPAT);
			);
		}

}

void test_rastswaps(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Raster			*bmr = &tcb.bytemap_raster;
Raster			*vbr = &tcb.verification_raster;
Rastlib			*rlib = r->lib;
PLANEPTR		pixbuf = bmr->hw.bm.bp[0];
Ucoor			width;
Ucoor			height;
short			do_verify;
short			iteration_count;
short			iteration_limit;

	width = r->width;
	height = r->height;
	do_verify = !tcb.timing_only_run;

	/*-----------------------------------------------------------------------
	 * swap_in_card testing...
	 *   if the driver didn't provide a routine, and we're not testing via
	 *   generics, skip the testing.
	 *---------------------------------------------------------------------*/
	
	if (is_generic(rlib,swaprect[0]) && !tcb.test_via_generics)
		{
		log_bypass("swap_in_card(), swap_to_ram(), swap_from_ram()");
		goto RSWAP_DONE;
		}

	/*------------------------------------------------------------------------
	 * swap_in_card testing..
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RSWAP_TIME_ITCOUNT : 1;

	log_start("Testing swap_in_card()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,  0);
	pj_set_rast(vbr,0);

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		do_swaps(r, r, iteration_count);
		}

	if (iteration_limit > 1) /* if looping for timing, we must clean out start*/
		pj_set_rect(r, 0, XPAT, YPAT,
					WPAT, HPAT); /* area before verification */
	do_swaps(vbr, vbr, 0);
	verify_raster(r, vbr, TRUE);

	log_end("...swap_in_card() testing complete.\n\n");

	if (!single_step())
		return;

	/*------------------------------------------------------------------------
	 * swap_to_ram testing..
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RSWAP_TIME_ITCOUNT : 1;

	log_start("Testing swap_to_ram()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,   0);
	pj_set_rast(bmr, 0);
	pj_set_rast(vbr, 0);

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		do_swaps(r, bmr, iteration_count);
		}

	pj_blitrect(bmr, 0,0, r, 0,0, width, height);

	do_swaps(vbr, vbr, 0);
	verify_raster(bmr, vbr, TRUE);

	log_end("...swap_to_ram() testing complete.\n\n");

	if (!single_step())
		return;

	/*------------------------------------------------------------------------
	 * swap_from_ram testing..
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RSWAP_TIME_ITCOUNT : 1;

	log_start("Testing swap_from_ram()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,  0);
	pj_set_rast(bmr,0);
	pj_set_rast(vbr,0);

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		do_swaps(bmr, r, iteration_count);
		}

	do_swaps(vbr, vbr, 0);
	verify_raster(r, vbr, TRUE);

	log_end("...swap_from_ram() testing complete.\n\n");

	single_step();

RSWAP_DONE:

	return;
}
