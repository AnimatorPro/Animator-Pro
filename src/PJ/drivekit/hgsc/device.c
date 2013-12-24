/*****************************************************************************
 * DEVICE.C - Device and rexlib layer routines for PJ HGSC SVGA driver.
 ****************************************************************************/

#include "hgsc.h"
#include <errcodes.h>

#undef	LOG_CALLS	/* define to log open/close calls, etc, to a file */

#ifndef LOG_CALLS
  #define INITFUNC	NULL
  #define CLEANFUNC NULL
#else
  #define INITFUNC	open_log
  #define CLEANFUNC close_log

  #include <stdio.h>

  static FILE *logfile = NULL;

  static Errcode open_log(void)
  /***************************************************************************
   * debugging only - open a file that we can log things to later.
   *  if the LOG_CALLS symbol is defined, this routine will be used as the
   *  rexlib-interface INIT routine, allowing us to log the loading and
   *  unloading of the driver, as well as the open/close graphics calls.
   **************************************************************************/
  {
	  if (NULL == (logfile = fopen("hgsclog.txt", "a")))
		  return pj_errno_errcode();
	  fprintf(logfile, "Driver loaded, logfile open...\n");
	  return Success;
  }

  static void close_log(void)
  /***************************************************************************
   * debugging only - close the logging file.
   *  if the LOG_CALLS symbol is defined, this routine will be used as the
   *  rexlib-interface CLEANUP routine.
   **************************************************************************/
  {
	  fprintf(logfile,"...driver being unloaded, logfile closed.\n");
	  fclose(logfile);
  }

#endif /* LOG_CALLS */

/*****************************************************************************
 * static data used by the device-layer interface...
 ****************************************************************************/

static Boolean device_is_open = FALSE;

static Vmode_info hgsc_infos[] =
{
	/*------------------------------------------------------------------------
	 * 640x480 data...
	 *----------------------------------------------------------------------*/
	{
	sizeof(Vmode_info),
	0,						 /* mode ix */
	"HGSC low rez",          /* Driver name for selection menu */
	8,						 /* bits */
	1,						 /* planes */
	{640,640,640,1},		 /* fixed width */
	{480,480,480,1},		 /* fixed height */
	TRUE, TRUE, 			 /* read and write */
	TRUE,					 /* Has viewable screen suitable for menus etc */
	1,						 /* fields_per_frame */
	1,						 /* display pages */
	0,						 /* store pages */
	640*480,				 /* display bytes */
	640*480,				 /* storage bytes */
	TRUE,					 /* Palette set only allowed during vblank */
	0,						 /* Swap screen during vsync. (We can't swap screens even)*/
	60, 					 /* Vsync rate */
	},
	/*------------------------------------------------------------------------
	 * 800x600 data...
	 *----------------------------------------------------------------------*/
	{
	sizeof(Vmode_info),
	1,						 /* mode ix */
	"HGSC medium rez",       /* Driver name for selection menu */
	8,						 /* bits */
	1,						 /* planes */
	{800,800,800,1},		 /* fixed width */
	{600,600,600,1},		 /* fixed height */
	TRUE, TRUE, 			 /* read and write */
	TRUE,					 /* Has viewable screen suitable for menus etc */
	1,						 /* fields_per_frame */
	1,						 /* display pages */
	0,						 /* store pages */
	800*600,				 /* display bytes */
	800*600,				 /* storage bytes */
	TRUE,					 /* Palette set's only allowed during vblank */
	0,						 /* Swap screen during vsync. (We can't swap screens even)*/
	56, 					 /* Vsync rate */
	},
	/*------------------------------------------------------------------------
	 * 1024x768 interlaced data...
	 *----------------------------------------------------------------------*/
	{
	sizeof(Vmode_info),
	2,						 /* mode ix */
	"HGSC hi interlaced",    /* Driver name for selection menu */
	8,						 /* bits */
	1,						 /* planes */
	{1024,1024,1024,1}, 	 /* fixed width */
	{768,768,768,1},		 /* fixed height */
	TRUE, TRUE, 			 /* read and write */
	TRUE,					 /* Has viewable screen suitable for menus etc */
	1,						 /* fields_per_frame */
	1,						 /* display pages */
	0,						 /* store pages */
	1024*768,				 /* display bytes */
	1024*768,				 /* storage bytes */
	TRUE,					 /* Palette set's only allowed during vblank */
	0,						 /* Swap screen during vsync. (We can't swap screens even)*/
	43,/*.5*/				 /* Vsync rate */
	},
	/*------------------------------------------------------------------------
	 * 1024x768 non-interlaced data
	 *----------------------------------------------------------------------*/
	{
	sizeof(Vmode_info),
	3,						 /* mode ix */
	"HGSC NON-i'laced",      /* Driver name for selection menu */
	8,						 /* bits */
	1,						 /* planes */
	{1024,1024,1024,1}, 	 /* fixed width */
	{768,768,768,1},		 /* fixed height */
	TRUE, TRUE, 			 /* read and write */
	TRUE,					 /* Has viewable screen suitable for menus etc */
	1,						 /* fields_per_frame */
	1,						 /* display pages */
	0,						 /* store pages */
	1024*768,				 /* display bytes */
	1024*768,				 /* storage bytes */
	TRUE,					 /* Palette set's only allowed during vblank */
	0,						 /* Swap screen during vsync. (We can't swap screens even)*/
	60, 					 /* Vsync rate */
	},
};

/*----------------------------------------------------------------------------
 * Our long-description text for each mode...
 *--------------------------------------------------------------------------*/

static char *modetext[] = {
	"Hercules Graphics Station Card, models GB1024 and GB1024+2, low resolution.",

	"Hercules Graphics Station Card, models GB1024 and GB1024+2, medium resolution.\n\n"
	  "Use this mode only if your monitor supports it!  See your HGSC "
	  "documentation for more details.",

	"Hercules Graphics Station Card, models GB1024 and GB1024+2, high rez interlaced.\n\n"
	  "Use this mode only if your monitor supports it!  See your HGSC "
	  "documentation for more details.",

	"Hercules Graphics Station Card, models GB1024 and GB1024+2, high rez non-interlaced.\n\n"
	  "Use this mode only if your monitor supports it!  See your HGSC "
	  "documentation for more details.",
	};

/*----------------------------------------------------------------------------
 * Data table we use to convert our PJ (0-3) mode numbers to the mode numbers
 * expected by the hgs_setmode() routine.
 *--------------------------------------------------------------------------*/

static int mode_xlate[] = {
	TM_640x480x8,
	TM_800x600,
	TM_1024x768i,
	TM_1024x768n,
	};

static Errcode hgsc_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
/*****************************************************************************
 * return pointer to the vmode_info structure for a given mode.
 ****************************************************************************/
{
	if (mode >= Array_els(hgsc_infos))
		return Err_no_such_mode;
	*pvm = hgsc_infos[mode];
	return Success ;
}

static char *hgsc_mode_text(Vdevice *driver, USHORT mode)
/*****************************************************************************
 * return pointer to the extended mode text description for a given mode.
 ****************************************************************************/
{
	if (mode >= Array_els(modetext))
		return NULL;

	return modetext[mode];
}

static Errcode hgsc_detect(Vdevice *dev)
/*****************************************************************************
 * tell whether the proper HGSC hardware is attached.
 * (currently unimplemented, waiting for answer on Herc BBS about how to do).
 ****************************************************************************/
{
	return Success;
}

static Errcode hgsc_open_graphics(Vdevice *dev, Hrast *r,
						   LONG width, LONG height, USHORT mode)
/*****************************************************************************
 * open the device, and the primary display raster.
 ****************************************************************************/
{
	Errcode    err;

#ifdef LOG_CALLS
	fprintf(logfile, "entering open_graphics() for mode %d\n", mode);
#endif

	if (mode >= Array_els(hgsc_infos))
		return Err_no_such_mode;

	if (Success == (err = hgsc_detect(dev)))
		{

		/*--------------------------------------------------------------------
		 * make sure PJ isn't confused about what mode it wants...
		 *------------------------------------------------------------------*/

		if (width  < hgsc_infos[mode].width.min  ||
			width  > hgsc_infos[mode].width.max  ||
			height < hgsc_infos[mode].height.min ||
			height > hgsc_infos[mode].height.max
		   )
			return Err_wrong_res;

		/*--------------------------------------------------------------------
		 * fill in the raster structure for the display raster...
		 *------------------------------------------------------------------*/

		r->lib		 = hgsc_get_rlib(dev, mode, r);
		r->type 	 = dev->first_rtype + mode;
		r->width	 = width;
		r->height	 = height;
		r->pdepth	 = hgsc_infos[mode].bits;
		r->aspect_dx = 1;
		r->aspect_dy = 1;

		r->grclib	 = dev->grclib; /* custom field, not part of rasthdr! */

		/*--------------------------------------------------------------------
		 * open the device ('cannot fail'), and clear the screen right away,
		 * since the HGSC tends to have a very glitchy looking screen right
		 * after a mode change.
		 *------------------------------------------------------------------*/

		hgs_setmode(mode_xlate[mode], NULL);
		hgs_setrast(r,0);
		device_is_open = TRUE;
		}

#ifdef LOG_CALLS
	fprintf(logfile, "leaving open_graphics(), err=%d, dev_is_open=%d.\n",
			err, device_is_open);
#endif

	return err;
}

static Errcode hgsc_close_graphics(Vdevice *driver)
/*****************************************************************************
 * close the SVGA-mode screen, reset hardware to normal VGA mode.
 ****************************************************************************/
{

#ifdef LOG_CALLS
	fprintf(logfile, "entering close_graphics(), dev_is_open=%d.\n",
			device_is_open);
#endif

	if (device_is_open)
		{
		hgs_setmode(TM_VGA, NULL);
		device_is_open = FALSE;
		}

#ifdef LOG_CALLS
	fprintf(logfile,"leaving close_graphics(), dev_is_open=%d.\n",
			device_is_open);
#endif

	return Success;
}

/*****************************************************************************
 * The vdevice_lib structure...
 ****************************************************************************/

struct vdevice_lib hgsc_device_library = {
	hgsc_detect,			/* detect - Is our hardware attatched? */
	hgsc_get_modes, 		/* get modes */
	hgsc_mode_text, 		/* get extra info */
	NOFUNC, 				/* set max height for width */
	hgsc_open_graphics, 	/* open_graphics */
	hgsc_close_graphics,	/* close_graphics */
	NOFUNC, 				/* open_cel */
	NOFUNC, 				/* show_rast */
};

/*****************************************************************************
 * Hostlib structure for stdio lib, needed only if LOG_CALLS is defined...
 ****************************************************************************/

#ifdef LOG_CALLS
  Hostlib _a_a_stdiolib = {NULL, AA_STDIOLIB, AA_STDIOLIB_VERSION};
  #define HLIB_LIST &_a_a_stdiolib
#else
  #define HLIB_LIST NULL
#endif

/*****************************************************************************
 * The rexlib_header structure...
 ****************************************************************************/

Vdevice rexlib_header = {
	{ REX_VDRIVER, 0, INITFUNC, CLEANFUNC, HLIB_LIST},
	0,											/* first_rtype - filled in by PJ */
	Array_els(hgsc_infos),						/* num_rtypes */
	Array_els(hgsc_infos),						/* mode_count */
	sizeof(Vdevice_lib)/sizeof(int(*)()),		/* dev_lib_count */
	&hgsc_device_library,						/* device library */
	NULL,										/* grclib - filled in by PJ */
	NUM_LIB_CALLS,								/* rast_lib_count */
};
