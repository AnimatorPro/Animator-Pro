#include "torture.h"

#define XRAST_TIME_ITCOUNT 5

void test_rastxors(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Raster			*bmr = &tcb.bytemap_raster;
Raster			*vbr = &tcb.verification_raster;
Rastlib			*rlib = r->lib;
Coor			x;
Coor			y;
Pixel			color;
Ucoor			width;
Ucoor			height;
short			iteration_count;
short			iteration_limit;

	width = r->width;
	height = r->height;

	/*-----------------------------------------------------------------------
	 * xor_in_card testing...
	 *   if the driver didn't provide a routine, and we're not testing via
	 *   generics, skip the testing.
	 *---------------------------------------------------------------------*/
	
	if (is_generic(rlib,xor_rast[0]) && !tcb.test_via_generics)
		{
		log_bypass("xor_in_card(), xor_to_ram(), xor_from_ram()");
		goto XRAST_DONE;
		}

	/*------------------------------------------------------------------------
	 * xor_in_card testing..
	 *----------------------------------------------------------------------*/

	if (r == &tcb.display_raster)
		{
		log_progress("Testing bypassed for xor_in_card(), current raster is "
					 "primary display raster.\n\n");
		goto XRAST_IN_CARD_DONE;
		}

	iteration_limit = (tcb.timing_only_run) ? XRAST_TIME_ITCOUNT : 1;

	log_start("Testing xor_in_card()...\n");

	iteration_count = iteration_limit;
	clear_screen();
	while (iteration_count--)
		{
		}

	log_end("...xor_in_card() testing complete.\n\n");

	if (!single_step())
		return;

XRAST_IN_CARD_DONE:

	/*------------------------------------------------------------------------
	 * xor_to_ram testing..
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? XRAST_TIME_ITCOUNT : 1;

	log_start("Testing xor_to_ram()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,   0);
	pj_set_rast(bmr, 0);
	pj_set_rast(vbr, 0);

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		color = iteration_count + 1;
		for (x = 0; x < width; x += 2*(WPAT/3))
			pj_set_rect(bmr, color, x, 0, WPAT/3, height);
		for (y = 0; y < height; y += 2*(HPAT/3))
			pj_set_rect(r, ~color, 0, y, width, HPAT/3);
		time_it(
			pj_xor_rast(r, bmr);
			);
		}

	pj_blitrect(bmr, 0, 0, r, 0, 0, width, height);

	color = 1;
	for (x = 0; x < width; x += 2*(WPAT/3))
		pj_set_rect(vbr, color, x, 0, WPAT/3, height);
	for (y = 0; y < height; y += 2*(HPAT/3))
		pj_xor_rect(vbr, ~color, 0, y, width, HPAT/3);
	verify_raster(bmr, vbr, TRUE);

	log_end("...xor_to_ram() testing complete.\n\n");

	if (!single_step())
		return;

	/*------------------------------------------------------------------------
	 * xor_from_ram testing..
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? XRAST_TIME_ITCOUNT : 1;

	log_start("Testing xor_from_ram()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,   0);
	pj_set_rast(bmr, 0);
	pj_set_rast(vbr, 0);

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		color = iteration_count + 1;
		for (x = 0; x < width; x += 2*(WPAT/3))
			pj_set_rect(bmr, color, x, 0, WPAT/3, height);
		for (y = 0; y < height; y += 2*(HPAT/3))
			pj_set_rect(r, ~color, 0, y, width, HPAT/3);
		time_it(
			pj_xor_rast(bmr, r);
			);
		}

	color = 1;
	for (x = 0; x < width; x += 2*(WPAT/3))
		pj_set_rect(vbr, color, x, 0, WPAT/3, height);
	for (y = 0; y < height; y += 2*(HPAT/3))
		pj_xor_rect(vbr, ~color, 0, y, width, HPAT/3);
	verify_raster(r, vbr, TRUE);

	log_end("...xor_from_ram() testing complete.\n\n");

	single_step();

XRAST_DONE:

	return;
}
