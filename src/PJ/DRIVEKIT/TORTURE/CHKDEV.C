#include "torture.h"
#ifdef __TURBOC__
#include <alloc.h>
#else
#include <malloc.h>
#endif

void check_device_sanity(void)
/*****************************************************************************
 * check device structures, ensure that required device-layer functions exist.
 ****************************************************************************/
{
short		vlib_count	= sizeof(Vdevice_lib) / sizeof(void(*)());
Vdevice 	*vd 		= tcb.vd;
Vdevice_lib *vdlib		= vd->lib;

	/*-----------------------------------------------------------------------
	 * check contents of Vdevice structure provided by the driver...
	 *---------------------------------------------------------------------*/

	 log_progress("Verifying contents of Vdevice structure...\n");

	 if (vd->num_rtypes == 0)
		log_error("Vdevice structure element 'num_rtypes' must be greater than zero.\n");

	 if (vd->mode_count == 0)
		log_error("Vdevice structure element 'mode_count' must be greater than zero.\n");

	if (vd->dev_lib_count != vlib_count)
		log_error("Vdevice structure element 'dev_lib_count' should be %hu, found %hu.\n",
				  vlib_count, vd->dev_lib_count);

	if (vd->lib == NULL)
		log_error("Vdevice structure element 'lib' must not be a NULL pointer.\n");

	if (vd->rast_lib_count == 0)
		log_error("Vdevice structure element 'rast_lib_count' should be %hu, found %hu.\n",
				  NUM_LIB_CALLS, vd->rast_lib_count);

	log_data("  Contents of Vdevice structure:\n"
			 "    first_rtype     = %hu\n"
			 "    num_rtypes      = %hu\n"
			 "    mode_count      = %hu\n"
			 "    dev_lib_count   = %hu\n"
			 "    lib             = %p \n"
			 "    grclib          = %p \n"
			 "    rast_lib_count  = %hu\n\n",
			 vd->first_rtype,
			 vd->num_rtypes,
			 vd->mode_count,
			 vd->dev_lib_count,
			 vd->lib,
			 vd->grclib,
			 vd->rast_lib_count
			);

	log_progress("...verification of Vdevice structure complete.\n\n");

	/*-----------------------------------------------------------------------
	 * check contents of Vdevice_lib structure provided by the driver...
	 *---------------------------------------------------------------------*/

	log_progress("Verifying contents of Vdevice_lib structure...\n");

	if (vdlib->get_modes == NULL)
		log_error("Required device function 'get_modes' not provided by driver.\n");

	if (vdlib->open_graphics == NULL)
		log_error("Required device function 'open_graphics' not provided by driver.\n");

	if (vdlib->close_graphics == NULL)
		log_error("Required device function 'close_graphics' not provided by driver.\n");

	if (vdlib->detect == NULL)
		log_error("Required device function 'detect' not provided by driver.\n");

	if (vdlib->close_graphics == NULL)
		log_error("Required device function 'close_graphics' not provided by driver.\n");

	log_data("  Contents of Vdevice_lib structure:\n"
			 "    detect          = %p\n"
			 "    get_modes       = %p\n"
			 "    mode_text       = %p\n"
			 "    set_max_height  = %p\n"
			 "    open_graphics   = %p\n"
			 "    close_graphics  = %p\n"
			 "    open_cel        = %p\n"
			 "    show_rast       = %p\n\n",
			 vdlib->detect,
			 vdlib->get_modes,
			 vdlib->mode_text,
			 vdlib->set_max_height,
			 vdlib->open_graphics,
			 vdlib->close_graphics,
			 vdlib->open_cel,
			 vdlib->show_rast
			);

	log_progress("...verification of Vdevice_lib structure complete.\n\n");
}

void test_device_modeinfo(void)
/*****************************************************************************
 * check device's ability to return mode_info and mode_text.
 *
 * This is really more sanity checking, and is thus neither optional nor
 * limited to the mode the developer requested for testing, because PJ will
 * make all these calls (and expect them to work right) when the Screen Size
 * menu item is selected by the user.
 ****************************************************************************/
{
Vdevice 	*vd 	= tcb.vd;
Vdevice_lib *vdlib	= vd->lib;
Vmode_info	vminf;
USHORT		counter;
Errcode 	err;
char		*mtext;
char		true[]	= "TRUE";
char		false[] = "FALSE";

	log_progress("Testing get_modes() and mode_text() for modes 0 thru %hu...\n",
					vd->mode_count-1);

	/*-----------------------------------------------------------------------
	 * call get_modes() and mode_text() for each mode supported by driver...
	 *---------------------------------------------------------------------*/

	for (counter = 0; counter < vd->mode_count; counter++)
		{
		log_progress("  Calling get_modes() for mode %hu.\n", counter);
		err = vdlib->get_modes(vd, counter, &vminf);
		if (err != Success)
			log_warning("Driver returned Errcode %d\n", err);
		else
			{
			if (vminf.struct_size != sizeof(vminf))
				log_error("Vdevice_info structure element 'struct_size' is %hu, should be %hu.",
							vminf.struct_size, (USHORT)sizeof(vminf));

			if (vminf.mode_ix != counter)
				log_error("Vdevice_info structure element 'mode_ix' is %hu, should be %hu.",
							vminf.mode_ix, counter);

			log_data("  Contents of Vmode_info structure for mode %hu:\n"
					 "    struct_size         = %hu\n"
					 "    mode_ix             = %hu\n"
					 "    mode_name           = %s \n"
					 "    bits                = %hu\n"
					 "    planes              = %hu\n"
					 "    width.min           = %hd\n"
					 "    width.max           = %hd\n"
					 "    width.actual        = %hd\n"
					 "    width.grain         = %hd\n"
					 "    height.min          = %hd\n"
					 "    height.max          = %hd\n"
					 "    height.actual       = %hd\n"
					 "    height.grain        = %hd\n",
					 counter,
					 vminf.struct_size,
					 vminf.mode_ix,
					 vminf.mode_name,
					 vminf.bits,
					 vminf.planes,
					 vminf.width.min,
					 vminf.width.max,
					 vminf.width.actual,
					 vminf.width.grain,
					 vminf.height.min,
					 vminf.height.max,
					 vminf.height.actual,
					 vminf.height.grain
					 );
			log_data("    readable            = %s \n"
					 "    writeable           = %s \n"
					 "    displayable         = %s \n"
					 "    fields_per_frame    = %hu\n"
					 "    display pages       = %hd\n"
					 "    storage pages       = %hd\n"
					 "    display bytes       = %ld\n"
					 "    storage bytes       = %ld\n"
					 "    pallete_vblank      = %s \n"
					 "    screen_swap_vblank  = %s \n"
					 "    field_rate          = %lu\n"
					 "    vblank_period       = %lu\n",
					 vminf.readable    ? true : false,
					 vminf.writeable   ? true : false,
					 vminf.displayable ? true : false,
					 (USHORT)vminf.fields_per_frame,
					 vminf.display_pages,
					 vminf.store_pages,
					 vminf.display_bytes,
					 vminf.store_bytes,
					 vminf.palette_vblank_only	   ? true : false,
					 vminf.screen_swap_vblank_only ? true : false,
					 vminf.field_rate,
					 vminf.vblank_period
					 );
			}

		if (vdlib->mode_text != NULL)
			{
			log_progress("  Calling mode_text() for mode %hu.\n", counter);
			mtext = vdlib->mode_text(vd, counter);
			if (mtext == NULL)
				log_warning("Driver returned NULL pointer for extended mode text.\n");
			else
				{
				log_data("----- Extended mode text: -----\n");
				log_data(mtext);
				log_data("\n-------------------------------\n\n");
				}
			}
		log_progress("  Mode info/text testing completed for mode %hu\n", counter);
		}

	log_progress("...mode info/text testing completed for modes 0 thru %hu\n\n", vd->mode_count-1);

	/*-----------------------------------------------------------------------
	 * if exercising error logic, call mode info/text for bad mode number...
	 *---------------------------------------------------------------------*/

	if (!tcb.exercise_error_handling)
		return;

	counter = (vd->mode_count+1) * 2;

	log_progress("Attempting to get mode_info for non-existant mode %hu...\n",
					counter);

	err = vdlib->get_modes(vd, counter, &vminf);
	if (err != Err_no_such_mode)
		log_warning("Driver returned %d, should have returned Err_no_such_mode\n",
						err);

	if (vdlib->mode_text != NULL)
		{
		log_progress("  Attempting to get mode_text() for mode %hu.\n", counter);
		mtext = vdlib->mode_text(vd, counter);
		if (mtext != NULL)
			log_warning("Driver returned %p, should have returned NULL pointer.\n",
							mtext);
		}

	log_progress("...testing of mode info/text error handling completed.\n\n");
}

void test_device_open(void)
/*****************************************************************************
 * call the driver open_graphics() function; do some sanity checks.
 *
 * Note to self:  Need error exercise code added to this function.
 ****************************************************************************/
{
Vdevice 	*vd 	= tcb.vd;
Vdevice_lib *vdlib	= vd->lib;
USHORT		mode	= tcb.test_vmode;
Raster		*vbr	= &tcb.verification_raster;
Raster		*bmr	= &tcb.bytemap_raster;
Raster		*hwr	= &tcb.display_raster;
Vmode_info	vminf;
Errcode 	err;
long		width;
long		height;
PLANEPTR	bm_pptr;
static char no_memory[] = "fatal: Unable to alloc screen buffer of "
						  "%ld bytes in main memory.\n\n";

	/*-----------------------------------------------------------------------
	 * test the detect() function if supplied, but don't whine too much if
	 * it returns a bad status; we'll go on and see how the open_graphics()
	 * function reacts even if detect() says we have no hardware.
	 *---------------------------------------------------------------------*/

	if (vdlib->detect != NULL)
		{
		log_progress("Testing detect() call...\n");
		err = vdlib->detect(vd);
		if (err != Success)
			log_warning("The detect() function returned Errcode = %d.\n", err);
		log_progress("...testing of detect() function completed.\n\n");
		}

	/*-----------------------------------------------------------------------
	 * call get_modes() to get the info we need to call open_graphics()...
	 *---------------------------------------------------------------------*/

	log_progress("Testing open_graphics() call for mode %hu...\n", mode);

	err = vdlib->get_modes(vd, mode, &vminf);
	if (err != Success)
		{
		log_error("Device cannot be opened in requested mode %hu:\n"
				  "        get_modes() returned Errcode = %d.\n",
					mode, err);
		return;
		}

	width  = vminf.width.actual  = vminf.width.max;
	height = vminf.height.actual = vminf.height.max;

	if ((vminf.width.min != vminf.width.max ||
		 vminf.height.min != vminf.height.max) &&
		vdlib->set_max_height == NULL)
		{
		 log_warning("Mode %hu supports variable resolution (width/height min != max),\n"
				  "  therefore, driver must supply a set_max_height() function.\n",
				  mode);
		 }

	/*-----------------------------------------------------------------------
	 * get a screen-sized bytemap raster to use in later testing...
	 *---------------------------------------------------------------------*/

	 if (NULL == (bm_pptr= malloc(width * height)))
		{
		log_error(no_memory, width*height);
		printf(no_memory, width*height);
		return;
		}
	else
		{
		init_bytemap_raster(bmr, vd, &vminf, bm_pptr);
		}

	/*-----------------------------------------------------------------------
	 * get another screen-sized bytemap raster to use in verification...
	 *---------------------------------------------------------------------*/

	 if (NULL == (bm_pptr= malloc(width * height)))
		{
		log_error(no_memory, width*height);
		printf(no_memory, width*height);
		return;
		}
	else
		{
		init_bytemap_raster(vbr, vd, &vminf, bm_pptr);
		}

	/*-----------------------------------------------------------------------
	 * attempt to open the device in the requested mode...
	 *---------------------------------------------------------------------*/

	init_raster(hwr);
	err = vdlib->open_graphics(vd, hwr, width, height, mode);
	if (err != Success)
		{
		log_error("Device cannot be opened in requested mode %hu:\n"
				  "          open_graphics() returned Errcode = %d.\n",
					mode, err);
		return;
		}

	tcb.mode_has_changed = TRUE;

	log_progress("...open_graphics() testing completed successfully.\n\n");

	/*-----------------------------------------------------------------------
	 * check sanity of the primary display raster...
	 *---------------------------------------------------------------------*/

	check_raster_sanity(hwr, RASTER_PRIMARY);

	/*-----------------------------------------------------------------------
	 * a extra little sanity check...
	 *---------------------------------------------------------------------*/

	if ((vminf.palette_vblank_only || vminf.screen_swap_vblank_only) &&
		hwr->lib->wait_vsync == NULL)
		{
		log_error("The wait_vsync() raster function is required when the Vmode_info\n"
				  "        elements 'palette_vblank' or 'screen_swap_vblank' are set to TRUE.\n"
				 );
		}
}

void test_device_close(void)
/*****************************************************************************
 * call close_device() function, report return code if not Success.
 ****************************************************************************/
{
Errcode err;

	log_progress("Testing close_device()...\n");

	err = tcb.vd->lib->close_graphics(tcb.vd);
	if (err != Success)
		{
		log_error("close_device() returned Errcode = %d\n", err);
		return;
		}

	log_progress("...close_device() testing completed successfully.\n\n");
}

void test_device_cels(void)
/*****************************************************************************
 * test the opening, sanity, and closing of secondary rasters.
 * Note to self: This could be a lot more rigorous; worry about it later.
 ****************************************************************************/
{
Vdevice 	*vd 	= tcb.vd;
Vdevice_lib *vdlib	= vd->lib;
Raster		*rp 	= &tcb.display_raster;
Raster		*rs 	= &tcb.offscrn_raster;
Errcode 	err;

	if (vdlib->open_cel == NULL)
		{
		log_bypass("open_cel()");
		return;
		}

	/*-----------------------------------------------------------------------
	 * test DISPLAYABLE raster...
	 *---------------------------------------------------------------------*/

	log_progress("Testing open_cel() to obtain secondary DISPLAYABLE raster...\n");

	init_raster(rs);
	err = vdlib->open_cel(vd, rs, rp->width, rp->height, 8, TRUE);
	if (err == Success)
		{
		if (vdlib->show_rast == NULL)
			log_error("A device supporting displayable rasters must provide show_rast() function.\n");
		check_raster_sanity(rs, RASTER1);
		test_close_raster(rs);
		}
	else
		log_progress("  Device does not support secondary displayable rasters.\n");

	log_progress("...testing of open_cel() for displayable raster complete.\n\n");

	/*-----------------------------------------------------------------------
	 * test non-displayable raster...
	 *
	 *	just for grins, we try a small 50x50 raster and a full screen-sized
	 *	raster.  if the screen-sized raster is a win, we keep it around for
	 *	testing other offscreen raster functions.
	 *---------------------------------------------------------------------*/

	log_progress("Testing open_cel() for small (50x50) offscreen raster...\n");
	init_raster(rs);
	err = vdlib->open_cel(vd, rs, 50, 50, 8, FALSE);
	if (err == Success)
		{
		check_raster_sanity(rs, RASTER1);
		test_close_raster(rs);
		}
	else
		log_warning("Driver returned Errcode = %d.\n", err);

	log_progress("Testing open_cel() for fullscreen-sized offscreen raster...\n");
	init_raster(rs);
	err = vdlib->open_cel(vd, rs, rp->width, rp->height, 8, FALSE);
	if (err == Success)
		{
		check_raster_sanity(rs, RASTER1);
		log_progress("...Fullscreen-sized raster will be left open for further testing.\n\n");
		}
	else
		log_warning("Driver returned Errcode = %d.\n", err);


}
