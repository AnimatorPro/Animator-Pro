#include "torture.h"

#define RZOOM_TIME_ITCOUNT 50

static void do_zooms(Raster *sr, Raster *dr)
/*****************************************************************************
 *
 ****************************************************************************/
{
Coor	width  = dr->width;
Coor	height = dr->height;
short	factor;

	if (sr != dr)
		pj_blitrect(sr, XPAT, YPAT, dr, XPAT, YPAT, WPAT, HPAT);

	pj_zoomblit(sr, XPAT, YPAT,
				dr, 0, 0, WPAT, HPAT, 1,1);

	pj_zoomblit(sr, XPAT, YPAT,
				dr, width-WPAT*2, 0, WPAT*2, HPAT*2, 2,2);

	factor = 3;
	if (WPAT*factor > width/2 ||
		HPAT*factor > height/2)
		factor = 2;
	pj_zoomblit(sr, XPAT, YPAT,
				dr, width-WPAT*factor, height-HPAT*factor,
				WPAT*factor, HPAT*factor, factor,factor);

	factor= 4;
	if (WPAT*factor > width/2 ||
		HPAT*factor > height/2)
		factor = 2;
	pj_zoomblit(sr, XPAT, YPAT,
				dr, 0, height-HPAT*factor,
				WPAT*factor, HPAT*factor, factor,factor);

	pj_zoomblit(sr, XPAT, YPAT,
				dr, XPAT+WPAT+1, YPAT+HPAT+1, WPAT+1, HPAT+1, 2,2);

}

void test_rastzooms(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Raster			*bmr = &tcb.bytemap_raster;
Raster			*vbr = &tcb.verification_raster;
Rastlib 		*rlib = r->lib;
Ucoor			width;
Ucoor			height;
short			iteration_count;
short			iteration_limit;

	width = r->width;
	height = r->height;

	/*-----------------------------------------------------------------------
	 * zoom_in_card testing...
	 *	 if the driver didn't provide a routine, and we're not testing via
	 *	 generics, skip the testing.
	 *---------------------------------------------------------------------*/

	if (is_generic(rlib,zoomblit[0]) && !tcb.test_via_generics)
		{
		log_bypass("zoom_in_card(), zoom_to_ram(), zoom_from_ram()");
		goto RZOOM_DONE;
		}

	/*------------------------------------------------------------------------
	 * zoom_in_card testing..
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RZOOM_TIME_ITCOUNT : 1;

	log_start("Testing zoom_in_card()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,	 0);
	pj_set_rast(vbr, 0);

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		draw_blitpattern(r, iteration_count, 0);
		time_it(
			do_zooms(r,r);
			);
		}

	draw_blitpattern(vbr, 0, 0);
	do_zooms(vbr, vbr);
	verify_raster(r, vbr, TRUE);

	log_end("...zoom_in_card() testing complete.\n\n");

	if (!single_step())
		return;

	/*------------------------------------------------------------------------
	 * zoom_to_ram testing..
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RZOOM_TIME_ITCOUNT : 1;

	log_start("Testing zoom_to_ram()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,	 0);
	pj_set_rast(vbr, 0);
	pj_set_rast(bmr, 0);

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		draw_blitpattern(r, iteration_count, 0);
		time_it(
			do_zooms(r,bmr);
			);
		}

	pj_blitrect(bmr, 0,0, r, 0,0, width, height);

	draw_blitpattern(vbr, 0, 0);
	do_zooms(vbr, vbr);
	verify_raster(bmr, vbr, TRUE);

	log_end("...zoom_to_ram() testing complete.\n\n");

	if (!single_step())
		return;

	/*------------------------------------------------------------------------
	 * zoom_from_ram testing..
	 *----------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? RZOOM_TIME_ITCOUNT : 1;

	log_start("Testing zoom_from_ram()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,	 0);
	pj_set_rast(vbr, 0);
	pj_set_rast(bmr, 0);

	rlib->wait_vsync(r);
	while (iteration_count--)
		{
		draw_blitpattern(bmr, iteration_count, 0);
		time_it(
			do_zooms(bmr,r);
			);
		}

	draw_blitpattern(vbr, 0, 0);
	do_zooms(vbr, vbr);
	verify_raster(r, vbr, TRUE);

	log_end("...zoom_from_ram() testing complete.\n\n");

	single_step();


RZOOM_DONE:

	return;
}
