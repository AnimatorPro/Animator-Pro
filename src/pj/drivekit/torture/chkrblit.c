#include "torture.h"

#define RBLIT_TIME_ITCOUNT 40

static void do_blits(Raster *sr, Raster *dr)
/*****************************************************************************
 *
 ****************************************************************************/
{
Coor			x;
Coor			y;
Coor			width;
Coor			height;

	width  = dr->width;
	height = dr->height;

	if (sr != dr)
		pj_blitrect(sr, XPAT, YPAT, dr, XPAT, YPAT, WPAT, HPAT);

	pj_blitrect(sr, XPAT, YPAT, dr, 		  0, 		   0, WPAT, HPAT);
	pj_blitrect(sr, XPAT, YPAT, dr, width-WPAT,   	   	   0, WPAT, HPAT);
	pj_blitrect(sr, XPAT, YPAT, dr, width-WPAT, height-HPAT, WPAT, HPAT);
	pj_blitrect(sr, XPAT, YPAT, dr,          0, height-HPAT, WPAT, HPAT);

	y = height / 2 - HPAT / 2;
	for (x = (width%WPAT)/2; x+WPAT <= width; x+= WPAT)
		pj_blitrect(sr, XPAT, YPAT, dr, x, y, WPAT, HPAT);

	x = width / 2 - WPAT / 2;
	for (y = (height%HPAT)/2; y+HPAT <= height; y+= HPAT)
		pj_blitrect(sr, XPAT, YPAT, dr, x, y, WPAT, HPAT);
}

void test_rastblits(Raster *r)
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
	 * blit_in_card testing...
	 *   if the driver didn't provide a routine, and we're not testing via
	 *   generics, skip the testing.
	 *---------------------------------------------------------------------*/
	
	if (is_generic(rlib,blitrect[0]) && !tcb.test_via_generics)
		{
		log_bypass("blit_in_card(), blit_to_ram(), blit_from_ram()");
		goto RBLIT_DONE;
		}

	/*------------------------------------------------------------------------
	 * blit_in_card testing..
	 *  clear the screen, then lay in a striped background before testing
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RBLIT_TIME_ITCOUNT : 1;

	log_start("Testing blit_in_card()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,  0);
	pj_set_rast(vbr,0);
	for (x = 0; x+WPAT/3 <= width; x += 2*(WPAT/3))
		{
		pj_set_rect(r,   1, x, 0, WPAT/3, height);
		pj_set_rect(vbr, 1, x, 0, WPAT/3, height);
		}

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		draw_blitpattern(r, iteration_count, 0);
		time_start();
		do_blits(r, r);
		time_end();
		}

	draw_blitpattern(vbr, 0, 0);
	do_blits(vbr, vbr);
	verify_raster(r, vbr, TRUE);

	log_end("...blit_in_card() testing complete.\n\n");

	if (!single_step())
		goto RBLIT_DONE;

	/*------------------------------------------------------------------------
	 * blit_to_ram testing..
	 *  clear the screen, lay in a striped background, blit background to ram
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RBLIT_TIME_ITCOUNT : 1;

	log_start("Testing blit_to_ram()...\n");

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
		draw_blitpattern(r, iteration_count, 0);
		time_start();
		do_blits(r, bmr);
		time_end();
		}

	draw_blitpattern(vbr, 0, 0);
	do_blits(vbr, vbr);
	verify_raster(bmr, vbr, TRUE);

	log_end("...blit_to_ram() testing complete.\n\n");

	if (!single_step())
		goto RBLIT_DONE;

	/*------------------------------------------------------------------------
	 * blit_from_ram testing..
	 *  clear the screen, lay in a striped background
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RBLIT_TIME_ITCOUNT : 1;

	log_start("Testing blit_from_ram()...\n");

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
		draw_blitpattern(bmr, iteration_count, 0);
		time_start();
		do_blits(bmr, r);
		time_end();
		}

	draw_blitpattern(vbr, 0, 0);
	do_blits(vbr, vbr);
	verify_raster(r, vbr, TRUE);

	log_end("...blit_from_ram() testing complete.\n\n");

	single_step();

RBLIT_DONE:

	return;
}
