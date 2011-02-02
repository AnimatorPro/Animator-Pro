#include <limits.h>
#include "torture.h"

#define DOT_TIME_ITCOUNT  50
#define CDOT_TIME_ITCOUNT 30

void dot_clearscreen(void)
/*****************************************************************************
 * clear screen using calls to put_dot (go get some coffee while this runs...)
 ****************************************************************************/
{
Coor		x;
Coor		y;
Raster  	*r;
Ucoor		width;
Ucoor		height;
Pdotfunc_t 	put_dot;

	r		= &tcb.display_raster;
	width 	= r->width;
	height  = r->height;
	put_dot	= r->lib->put_dot;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			put_dot(r, 0, x, y);
}

static Boolean inraster(Raster *r, Coor x, Coor y)
/*****************************************************************************
 * determine whether an xy point is inside or outside of a raster.
 ****************************************************************************/
{
	if (x >= 0 && x < r->width && y >= 0 && y < r->height)
		return TRUE;
	else
		return FALSE;
}


static void test_put_dot(Raster *r, 
						 Pixel color_start, short color_incr, 
						 short steps,
						 Ucoor width,  Ucoor height,
						 Coor  xstart, Coor  ystart,
						 Pdotfunc_t put_dot
						)
/*****************************************************************************
 * put a pattern on the screen using the driver's put_dot routine.
 *  note that this routine writes a pattern to be verified by test_get_dot(),
 *  changes to these routines must be made in sync.
 ****************************************************************************/
{
Coor  	x;
Coor	y;
Ucoor	xstep;
Ucoor	ystep;
Pixel 	color;

	--width;
	--height;
	xstep  = 1 + width / steps;
	ystep  = 1 + height / steps;
	color  = color_start;

	/*-----------------------------------------------------------------------
	 * draw a box that outlines the full screen...
	 *---------------------------------------------------------------------*/

	for (x = xstart, y = ystart; x < width; x++, color += color_incr)
		put_dot(r, color, x, y);

	for (y = ystart, x = width; y < height; y++, color += color_incr)
		put_dot(r, color, x, y);

	for (x = width-1, y = height; x > xstart; x--, color += color_incr)
		put_dot(r, color, x, y);
						  
	for (y = height-1, x = xstart; y > ystart; y--, color += color_incr)	
		put_dot(r, color, x, y);

	/*-----------------------------------------------------------------------
	 * draw a crosshatch pattern in the box...
	 *---------------------------------------------------------------------*/

	 for (x = xstep; x < width; x += xstep)
		for (y = 1; y < height; y++, color += color_incr)
			put_dot(r, color, x, y);

	 for (y = ystep; y < height; y += ystep)
		for (x = 1; x < width; x++, color += color_incr)
			put_dot(r, color, x, y);

}

static void test_get_dot(Raster *r, 
						 Pixel color_start, short color_incr, 
						 short steps,
						 Ucoor width,  Ucoor height,
						 Coor  xstart, Coor  ystart,
						 Gdotfunc_t get_dot
						)
/*****************************************************************************
 * read and verify a pattern from the screen using the driver's get_dot.
 *  note that this routine verifies the pattern written by test_put_dot(),
 *  changes to these routines must be made in sync.
 *
 *  also note that we have some special logic to handle testing of 
 *  cget_dot() -- if the failing xy location is offscreen, we make sure
 *  the return value is a zero instead of the expected value.
 ****************************************************************************/
{
Coor  	x;
Coor	y;
Ucoor	xstep;
Ucoor	ystep;
Pixel 	expected_color;
Pixel	obtained_color;
short	verify;

	--width;
	--height;
	xstep  = 1 + width / steps;
	ystep  = 1 + height / steps;
	expected_color  = color_start;
	verify	= !tcb.timing_only_run;

	/*-----------------------------------------------------------------------
	 * verify the box that outlines the full screen...
	 *---------------------------------------------------------------------*/

	for (x = xstart, y = ystart; x < width; x++, expected_color += color_incr)
		{
		obtained_color = get_dot(r, x, y);
		if (verify)
			{
			if (expected_color != obtained_color)
				{
				if (inraster(r, x, y))
					goto ERROR;
				else if (obtained_color != 0)
					goto CLIPERROR;
				}
			else
				{
				if (expected_color != 0 && !inraster(r, x, y))
					goto CLIPERROR;
				}
			}
		}

	for (y = ystart, x = width; y < height; y++, expected_color += color_incr)
		{
		obtained_color = get_dot(r, x, y);
		if (verify)
			{
			if (expected_color != obtained_color)
				{
				if (inraster(r, x, y))
					goto ERROR;
				else if (obtained_color != 0)
					goto CLIPERROR;
				}
			else
				{
				if (expected_color != 0 && !inraster(r, x, y))
					goto CLIPERROR;
				}
			}
		}

	for (x = width-1, y = height; x > xstart; x--, expected_color += color_incr)
		{
		obtained_color = get_dot(r, x, y);
		if (verify)
			{
			if (expected_color != obtained_color)
				{
				if (inraster(r, x, y))
					goto ERROR;
				else if (obtained_color != 0)
					goto CLIPERROR;
				}
			else
				{
				if (expected_color != 0 && !inraster(r, x, y))
					goto CLIPERROR;
				}
			}
		}
						  
	for (y = height-1, x = xstart; y > ystart; y--, expected_color += color_incr)	
		{
		obtained_color = get_dot(r, x, y);
		if (verify)
			{
			if (expected_color != obtained_color)
				{
				if (inraster(r, x, y))
					goto ERROR;
				else if (obtained_color != 0)
					goto CLIPERROR;
				}
			else
				{
				if (expected_color != 0 && !inraster(r, x, y))
					goto CLIPERROR;
				}
			}
		}

	/*-----------------------------------------------------------------------
	 * verify the crosshatch pattern in the box...
	 *  (note that we can't check the intersections of the cross hatch 
	 *   pattern because with the incrementing colors it screws up bigtime...)
	 *---------------------------------------------------------------------*/

	 for (x = xstep; x < width; x += xstep)
		{
		for (y = 1; y < height; y++, expected_color += color_incr)
			{
			obtained_color = get_dot(r, x, y);
			if (verify)
				{
				if ((y % ystep) == 0)
					continue;
				if (expected_color != obtained_color)
					{
					if (inraster(r, x, y))
						goto ERROR;
					else if (obtained_color != 0)
						goto CLIPERROR;
					}
				else
					{
					if (expected_color != 0 && !inraster(r, x, y))
						goto CLIPERROR;
					}
				}
			}
		}

	 for (y = ystep; y < height; y += ystep)
		{
		for (x = 1; x < width; x++, expected_color += color_incr)
            {
			obtained_color = get_dot(r, x, y);
			if (verify)
				{
				if ((x % xstep) == 0)
					continue;
				if (expected_color != obtained_color)
					{
					if (inraster(r, x, y))
						goto ERROR;
					else if (obtained_color != 0)
						goto CLIPERROR;
					}
				else
					{
					if (expected_color != 0 && !inraster(r, x, y))
						goto CLIPERROR;
					}
				}
			}
		}

	goto OUT;

ERROR:

	log_error("get_dot() returned %hu, expected to find %hu, at x=%hd y=%hd.\n",
			  (USHORT)obtained_color, (USHORT)expected_color, 
			  (SHORT)x, (SHORT)y);
	goto OUT;

CLIPERROR:

	log_error("cget_dot() returned %hu, expected 0 for read at x=%hd y=%hd.\n",
			  (USHORT)obtained_color, 
			  (SHORT)x, (SHORT)y);
	goto OUT;

OUT:

	return;
}

static void test_cput_dot(Raster *r, Boolean write_out_of_bounds, short color_start)
/*****************************************************************************
 * test the driver's cput_dot routine.
 *  based on the write_out_of_bounds flag, this routine will write data
 *  either entirely on the screen, or both on and off of it.
 *
 *  note that this routine writes a pattern to be verified by test_cget_dot(),
 *  changes to these routines must be made in sync.
 ****************************************************************************/
{
Coor  		x;
Coor  		y;
Ucoor 		width;
Ucoor 		height;
short 		steps;
Pdotfunc_t 	cput_dot;

	/*-----------------------------------------------------------------------
	 * set up for the test, depending on whether we should try drawing out
	 * of bounds or not, and whether we're using a driver or generic routine.
	 *---------------------------------------------------------------------*/

	cput_dot = r->lib->cput_dot;

	if (write_out_of_bounds)
		{
		x 		= -2 * r->width;
		y  		= -2 * r->height;
		width  	=  2 * r->width;
		height 	=  2 * r->width;
		steps	= 20;
		}
	else
		{
		x 		= 0;
		y  		= 0;
		width  	= r->width;
		height 	= r->height;
		steps	= 10;
		}

	/*-----------------------------------------------------------------------
	 * draw the pattern...
	 *---------------------------------------------------------------------*/

	test_put_dot(r, color_start, 1, steps, 
				 width, height, x, y, 
				 cput_dot
				);

	/*-----------------------------------------------------------------------
	 * a little extra stess testing for drawing out of bounds...
	 *---------------------------------------------------------------------*/

	if (write_out_of_bounds)
		{
		cput_dot(r, 255, SHRT_MIN, SHRT_MIN);
		cput_dot(r, 255, SHRT_MIN, SHRT_MAX);
		cput_dot(r, 255, SHRT_MAX, SHRT_MIN);
		cput_dot(r, 255, SHRT_MAX, SHRT_MAX);
		}

}

static void test_cget_dot(Raster *r, Boolean read_out_of_bounds, short color_start)
/*****************************************************************************
 * test the driver's cget_dot routine.
 *  based on the read_out_of_bounds flag, this routine will read data
 *  either entirely on the screen, or entirely off of it.
 *
 *  note that this routine verifies the pattern written by test_cput_dot(),
 *  changes to these routines must be made in sync.
 ****************************************************************************/
{
Coor  		x;
Coor  		y;
Ucoor 		width;
Ucoor 		height;
short 		steps;
Pixel 		obtained_color;
Gdotfunc_t	cget_dot;

	/*-----------------------------------------------------------------------
	 * set up for the test, depending on whether we should try reading out
	 * of bounds or not, and whether we're using a driver or generic routine.
	 *---------------------------------------------------------------------*/

	cget_dot = r->lib->cget_dot;

	if (read_out_of_bounds)
		{
		x 		= -2 * r->width;
		y  		= -2 * r->height;
		width  	=  2 * r->width;
		height 	=  2 * r->width;
		steps	= 20;
		}
	else
		{
		x 		= 0;
		y  		= 0;
		width  	= r->width;
		height 	= r->height;
		steps	= 10;
		}

	/*-----------------------------------------------------------------------
	 * verify the pattern...
	 *---------------------------------------------------------------------*/

	test_get_dot(r, color_start, 1, steps, 
				 width, height, x, y, 
				 cget_dot
				);

	/*-----------------------------------------------------------------------
	 * a little extra stess testing for reading out of bounds...
	 *---------------------------------------------------------------------*/

	if (read_out_of_bounds)
		{
		if (0 != (obtained_color = cget_dot(r, x=SHRT_MIN, y=SHRT_MIN)))
			goto CLIPERROR;
		if (0 != (obtained_color = cget_dot(r, x=SHRT_MIN, y=SHRT_MAX)))
			goto CLIPERROR;
		if (0 != (obtained_color = cget_dot(r, x=SHRT_MAX, y=SHRT_MIN)))
			goto CLIPERROR;
		if (0 != (obtained_color = cget_dot(r, x=SHRT_MAX, y=SHRT_MAX)))
			goto CLIPERROR;
		}

	goto OUT;

CLIPERROR:

	log_error("cget_dot() returned %hu, expected 0 for read at x=%hd y=%hd.\n",
			  (USHORT)obtained_color, 
			  (SHORT)x, (SHORT)y);

OUT:

	return;

}

void test_dots(Raster *r)
/*****************************************************************************
 * drive the testing of put_dot, get_dot, cput_dot, cget_dot.
 ****************************************************************************/
{
Rastlib	*rlib = r->lib;
SHORT	iteration_count;
SHORT	iteration_limit;

	/*-----------------------------------------------------------------------
	 * test the basic put_dot function, using solid color & single crosshatch.
	 * if put_dot works out, test get_dot by reading back the same pattern
	 * that we wrote using put_dot (verifying the dots read as we go).
	 *---------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? DOT_TIME_ITCOUNT : 1;
		
	log_start("Testing put_dot() with solid color...\n");
	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		test_put_dot(r, 1+iteration_count, 0, 7,
					 r->width, r->height, r->x, r->y,
					 rlib->put_dot);
		}
	log_end("...put_dot() testing completed.\n\n");

	log_start("Testing get_dot() for solid color...\n");
	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		test_get_dot(r, 1+iteration_count, 0, 7,
					 r->width, r->height, r->x, r->y,
					 rlib->get_dot);
		}
	log_end("...get_dot() testing completed.\n\n");

	if (!single_step())
		goto DOTS_DONE;

	/*-----------------------------------------------------------------------
	 * another test of the put_dot function, using incrementing colors and 
	 * an 8x8 crosshatch pattern.  again, verify the pattern using get_dot.
	 *---------------------------------------------------------------------*/

	log_start("Testing put_dot() with a changing color value...\n");
	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		test_put_dot(r, 1+iteration_count, 1, 7,
					 r->width, r->height, r->x, r->y,
					 rlib->put_dot);
		}
	log_end("...put_dot() testing completed.\n\n");

	log_start("Testing get_dot() with a changing color value...\n");
	iteration_count = iteration_limit;
	while (iteration_count--)
		{
		test_get_dot(r, 1+iteration_count, 1, 7,
					 r->width, r->height, r->x, r->y,
					 rlib->get_dot);
		}
	log_end("...get_dot() testing completed.\n\n");

	if (!single_step())
		goto DOTS_DONE;

	if (tcb.got_stop)
		return;

	/*-----------------------------------------------------------------------
	 * test cput_dot (clipped put dot) if the driver has provided the 
	 * function, or if we are verifying missing functions via the generics.
	 *---------------------------------------------------------------------*/

	if (!is_generic(rlib,cput_dot) || tcb.test_via_generics)
		{

		iteration_limit = (tcb.timing_only_run) ? CDOT_TIME_ITCOUNT : 1;

		log_start("Testing cput_dot() writing only in-bounds data...\n");
		iteration_count = iteration_limit;
		while (iteration_count--)
			test_cput_dot(r, FALSE, 1+iteration_count);
		log_end("...cput_dot() in-bounds testing completed.\n\n");

		log_start("Testing cget_dot() reading only in-bounds data...\n");
		iteration_count = iteration_limit;
		while (iteration_count--)
			test_cget_dot(r, FALSE, 1+iteration_count);
		log_end("...cget_dot() in-bounds testing completed.\n\n");

		if (!single_step())
			goto DOTS_DONE;

		log_start("Testing cput_dot() writing out-of-bounds data...\n");
		iteration_count = iteration_limit;
		while (iteration_count--)
			test_cput_dot(r, TRUE, 1+iteration_count);
		log_end("...cput_dot() out-of-bounds testing completed.\n\n");

		log_start("Testing cget_dot() reading out-of-bounds data...\n");
		iteration_count = iteration_limit;
		while (iteration_count--)
			test_cget_dot(r, TRUE, 1+iteration_count);
		log_end("...cget_dot() out-of-bounds testing completed.\n\n");

		if (!single_step())
			goto DOTS_DONE;
		}
	else
		log_bypass("cput_dot() and cget_dot()");

DOTS_DONE:

	return;
}
