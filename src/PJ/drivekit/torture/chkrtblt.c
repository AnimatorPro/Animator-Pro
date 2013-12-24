#include "torture.h"

#define TBLIT_TIME_ITCOUNT 25

static void do_blits(Raster *sr, Raster *dr, Pixel tcolor)
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

	pj_tblitrect(sr, XPAT, YPAT,
				 dr, 		   0, 			0,  WPAT, HPAT, tcolor);
	pj_tblitrect(sr, XPAT, YPAT,
				 dr, width-WPAT,   	    0,  WPAT, HPAT, tcolor);
	pj_tblitrect(sr, XPAT, YPAT,
				 dr, width-WPAT, height-HPAT, WPAT, HPAT, tcolor);
	pj_tblitrect(sr, XPAT, YPAT,
				 dr,          0, height-HPAT, WPAT, HPAT, tcolor);

	y = height / 2 - HPAT / 2;
	for (x = (width%WPAT)/2; x+WPAT <= width; x+= WPAT)
		pj_tblitrect(sr, XPAT, YPAT, dr, x, y, WPAT, HPAT, tcolor);

	x = width / 2 - WPAT / 2;
	for (y = (height%HPAT)/2; y+HPAT <= height; y+= HPAT)
		pj_tblitrect(sr, XPAT, YPAT, dr, x, y, WPAT, HPAT, tcolor);
}

void test_rasttblits(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Raster			*bmr = &tcb.bytemap_raster;
Raster			*vbr = &tcb.verification_raster;
Rastlib			*rlib = r->lib;
Coor			x;
Ucoor			width;
Ucoor			height;
short			iteration_count;
short			iteration_limit;

	width = r->width;
	height = r->height;

	/*-----------------------------------------------------------------------
	 * tblit_in_card testing...
	 *   if the driver didn't provide a routine, and we're not testing via
	 *   generics, skip the testing.
	 *---------------------------------------------------------------------*/
	
	if (is_generic(rlib,blitrect[0]) && !tcb.test_via_generics)
		{
		log_bypass("tblit_in_card(), tblit_to_ram(), tblit_from_ram()");
		goto TBLIT_DONE;
		}

	/*------------------------------------------------------------------------
	 * tblit_in_card testing..
	 *  clear the screen, then lay in a striped background before testing
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? TBLIT_TIME_ITCOUNT : 1;

	log_start("Testing tblit_in_card()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,   0);
	pj_set_rast(vbr, 0);
	for (x = 0; x+WPAT/3 <= width; x += 2*(WPAT/3))
		{
		pj_set_rect(r,   1, x, 0, WPAT/3, height);
		pj_set_rect(vbr, 1, x, 0, WPAT/3, height);
		}

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		draw_blitpattern(r, iteration_count, ~iteration_count);
		time_it(
			do_blits(r, r, ~iteration_count);
			);
		}

	draw_blitpattern(vbr, 0, 255);
	do_blits(vbr, vbr, 255);
	verify_raster(r, vbr, TRUE);

	log_end("...tblit_in_card() testing complete.\n\n");

	if (!single_step())
		return;

	/*------------------------------------------------------------------------
	 * tblit_to_ram testing..
	 *  clear the screen, lay in a striped background, blit background to ram
	 *----------------------------------------------------------------------*/


	iteration_limit = (tcb.timing_only_run) ? TBLIT_TIME_ITCOUNT : 1;

	log_start("Testing tblit_to_ram()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,   0);
	pj_set_rast(bmr, 0);
	pj_set_rast(vbr, 0);
	for (x = 0; x+WPAT/3 <= width; x += 2*(WPAT/3))
		{
		pj_set_rect(bmr, 1, x, 0, WPAT/3, height);
		pj_set_rect(vbr, 1, x, 0, WPAT/3, height);
		}

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		draw_blitpattern(r, iteration_count, ~iteration_count);
		time_it(
			do_blits(r, bmr, ~iteration_count);
			);
		}

	pj_blitrect(bmr, 0,0, r, 0,0, width, height);
	draw_blitpattern(r, 0, 255);

	draw_blitpattern(bmr, 0, 255);
	draw_blitpattern(vbr, 0, 255);
	do_blits(vbr, vbr, 255);
	verify_raster(bmr, vbr, TRUE);

	log_end("...tblit_to_ram() testing complete.\n\n");

	if (!single_step())
		return;

	/*------------------------------------------------------------------------
	 * tblit_from_ram testing..
	 *  clear the screen, lay in a striped background
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? TBLIT_TIME_ITCOUNT : 1;

	log_start("Testing tblit_from_ram()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,   0);
	pj_set_rast(bmr, 0);
	pj_set_rast(vbr, 0);
	for (x = 0; x+WPAT/3 <= width; x += 2*(WPAT/3))
		{
		pj_set_rect(r,   1, x, 0, WPAT/3, height);
		pj_set_rect(vbr, 1, x, 0, WPAT/3, height);
		}

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		draw_blitpattern(bmr, iteration_count, ~iteration_count);
		time_it(
			do_blits(bmr, r, ~iteration_count);
			);
		}

	draw_blitpattern(r,   0, 255);
	draw_blitpattern(vbr, 0, 255);
	do_blits(vbr, vbr, 255);
	verify_raster(r, vbr, TRUE);

	log_end("...tblit_from_ram() testing complete.\n\n");

	single_step();

TBLIT_DONE:

	return;
}
