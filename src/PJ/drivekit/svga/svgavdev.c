/*****************************************************************************
 * DEVICE.C - Device-layer interface for SVGA driver.
 *
 *	05/25/91 -	Ian
 *				New driver, handles most SVGA.
 ****************************************************************************/

/******************************************************************************
*																			  *
*		   Copyright (C) 1991 by Autodesk, Inc. 							  *
*																			  *
*	Permission to use, copy, modify, and distribute this software and		  *
*	its documentation for the purpose of creating applications for			  *
*	AutoCAD, is hereby granted in accordance with the terms of the			  *
*	License Agreement accompanying this product.							  *
*																			  *
*	Autodesk makes no warrantees, express or implied, as to the 			  *
*	correctness of this code or any derivative works which incorporate		  *
*	it.  Autodesk provides the code on an ''as-is'' basis and                 *
*	explicitly disclaims any liability, express or implied, for 			  *
*	errors, omissions, and other problems in the code, including			  *
*	consequential and incidental damages.									  *
*																			  *
*	Use, duplication, or disclosure by the U.S.  Government is				  *
*	subject to restrictions set forth in FAR 52.227-19 (Commercial			  *
*	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)		  *
*	(1) (ii) (Rights in Technical Data and Computer Software, as			  *
*	applicable. 															  *
*																			  *
******************************************************************************/

#include <syslib.h>
#include <drvcomn.h>
#include "svga.h"

/*----------------------------------------------------------------------------
 * the following block does some code tweaks depending on whether we're
 * building the REX-loadable driver or the FLILIB version of the driver.
 *
 * The main thrust of these tweaks is to use the standard C functions when
 * compiled as a standalone driver, and to avoid using standard C functions
 * when compiled as part of the fliclib.  The fliclib needs to be compatible
 * with both -3r and -3s calling standards, and mixing our library (compiled
 * with 3s) and the standard 3r library is difficult.
 *
 * When building the FLILIB version, the FLILIB_CODE symbol is defined via
 * a -D compiler switch in the makefile.  (The switch is actually defined
 * in the MAKE.INC file, so that it's automatically used when the compile
 * is being done in the FLILIB subdirs.
 *--------------------------------------------------------------------------*/

#ifndef FLILIB_CODE
	#define svgadev 	rexlib_header		// rex code needs Vdevice named
	extern Vdevice		rexlib_header;		// rexlib_header for loader.
	#define pj_malloc	malloc				// rex code uses malloc() and
	#define pj_free 	free				// free() from hostlib interface.
#else
	#define svgadev 	svga_vdevice		// flilib code Vdevice name must
	extern Vdevice		svga_vdevice;		// be a static name, not global.
	extern void 		*pj_malloc(size_t); // flilib code must go through
	extern void 		pj_free(void *);	// library memory management.
#endif

/*----------------------------------------------------------------------------
 * the skeleton vmode_info structure.
 *	due to the dynamic nature of our modes, this single instance of the
 *	structure is used for all modes; the get_modes() call will simply
 *	replace the '0' values in this structure to refelect the requested mode.
 *--------------------------------------------------------------------------*/

static Vmode_info vminf = {
		sizeof(Vmode_info),
		0,						/* mode index */
		"",                     /* mode title text (filled in later) */
		8,						/* bits */
		1,						/* planes */
		{0,0,0,1},				/* fixed width */
		{0,0,0,1},				/* fixed height */
		TRUE, TRUE, 			/* read and write */
		TRUE,					/* Has viewable screen suitable for menus etc */
		1,						/* fields per frame (ie, interlace) */
		1,						/* display pages */
		1,						/* store pages */
		0,						/* display bytes */
		0,						/* storage bytes */
		TRUE,					/* Palette set's only allowed during vblank */
		FALSE,					/* Swap screen during vsync. (We can't swap screens even)*/
		1000000U/70,			/* field rate */
		0						/* vertical blanking period */
	};

/*----------------------------------------------------------------------------
 * a few data items...
 *--------------------------------------------------------------------------*/

static long    old_mode_data;
static int	   smodecount		= -1;
static Boolean device_is_open	= FALSE;
static Boolean device_is_loaded = FALSE;

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

static Rastlib *get_rlib(void)
/*****************************************************************************
 * build the raster library structure, return a pointer to it.
 ****************************************************************************/
{
	memset(&pj_vdrv_raster_library, 0, sizeof(pj_vdrv_raster_library));

	pj_vdrv_raster_library.wait_vsync	 = pj_vdrv_wait_vblank;

	pj_vdrv_raster_library.put_dot		 = pj_vdrv_put_dot;
	pj_vdrv_raster_library.get_dot		 = pj_vdrv_get_dot;
	pj_vdrv_raster_library.cput_dot 	 = pj_vdrv_cput_dot;
	pj_vdrv_raster_library.cget_dot 	 = pj_vdrv_cget_dot;

	pj_vdrv_raster_library.set_hline	 = pj_vdrv_set_hline;
	pj_vdrv_raster_library.set_vline	 = pj_vdrv_set_vline;

	pj_vdrv_raster_library.put_hseg 	 = pj_vdrv_put_hseg;
	pj_vdrv_raster_library.get_hseg 	 = pj_vdrv_get_hseg;
	pj_vdrv_raster_library.put_vseg 	 = pj_vdrv_put_vseg;
	pj_vdrv_raster_library.get_vseg 	 = pj_vdrv_get_vseg;

	pj_vdrv_raster_library.put_rectpix	 = pj_vdrv_put_rectpix;
	pj_vdrv_raster_library.get_rectpix	 = pj_vdrv_get_rectpix;

	pj_vdrv_raster_library.set_rast 	 = pj_vdrv_set_rast;
	pj_vdrv_raster_library.set_rect 	 = pj_vdrv_set_rect;
	pj_vdrv_raster_library.xor_rect 	 = pj_vdrv_xor_rect;

	pj_vdrv_raster_library.mask1blit	 = pj_vdrv_mask1blit;
	pj_vdrv_raster_library.mask2blit	 = pj_vdrv_mask2blit;

	pj_vdrv_raster_library.blitrect[0]	 = pj_vdrv_blit_in_card;
	pj_vdrv_raster_library.blitrect[1]	 = pj_vdrv_blit_to_ram;
	pj_vdrv_raster_library.blitrect[2]	 = pj_vdrv_blit_from_ram;

	pj_vdrv_raster_library.tblitrect[0]  = pj_vdrv_tblit_in_card;
	pj_vdrv_raster_library.tblitrect[1]  = pj_vdrv_tblit_to_ram;
	pj_vdrv_raster_library.tblitrect[2]  = pj_vdrv_tblit_from_ram;

	pj_vdrv_raster_library.zoomblit[0]	 = pj_vdrv_zoom_in_card;
	pj_vdrv_raster_library.zoomblit[1]	 = pj_vdrv_zoom_to_ram;
	pj_vdrv_raster_library.zoomblit[2]	 = pj_vdrv_zoom_from_ram;

	pj_vdrv_raster_library.swaprect[0]	 = pj_vdrv_swap_in_card;
	pj_vdrv_raster_library.swaprect[1]	 = pj_vdrv_swap_to_ram;
	pj_vdrv_raster_library.swaprect[2]	 = pj_vdrv_swap_from_ram;

	pj_vdrv_raster_library.xor_rast[1]	 = pj_vdrv_xor_to_ram;
	pj_vdrv_raster_library.xor_rast[2]	 = pj_vdrv_xor_from_ram;

	pj_vdrv_raster_library.unss2_rect	 = pj_vdrv_unss2;
	pj_vdrv_raster_library.unlccomp_rect = pj_vdrv_unlccomp_rect;
	pj_vdrv_raster_library.unbrun_rect	 = pj_vdrv_unbrun_rect;

	if (pj_vdrv_has_8bitdac) {
		pj_vdrv_raster_library.set_colors = pj_vdrv_8bit_set_colors;
		pj_vdrv_raster_library.uncc64	  = pj_vdrv_8bit_uncc64;
		pj_vdrv_raster_library.uncc256	  = pj_vdrv_8bit_uncc256;
	} else {
		pj_vdrv_raster_library.set_colors = pj_vdrv_set_colors;
		pj_vdrv_raster_library.uncc64	  = pj_vdrv_uncc64;
		pj_vdrv_raster_library.uncc256	  = pj_vdrv_uncc256;
	}

	return(&pj_vdrv_raster_library);
}

static Errcode svga_init(void)
/*****************************************************************************
 * detect presence of known svga, return Success or appropriate error status.
 * rex-layer init routine, automatically executed when the driver is loaded.
 *
 * this routine will call the detect-and-init functions and return their
 * status to the rex loader.  the svga driver must have this detection
 * and init done at load time because it is the init processing which tells
 * us what svga modes are available.  before the host gets a look at our
 * VDevice structure, it has to contain the number of modes available. by
 * hooking into the rex-layer init vector, we get a chance to tweak the
 * VDevice structure to reflect reality before the host can get a look at it.
 ****************************************************************************/
{
	static	Errcode err;

	if (!device_is_loaded) {
		device_is_loaded = TRUE;
		smodecount = pj_svga_init(pj_vdrv_modeinfo);
		if (smodecount < 0) {
			err = smodecount;
		} else if (smodecount == 0) {
			err = Err_no_display;
		} else {
			err = Success;
			svgadev.num_rtypes = smodecount;
			svgadev.mode_count = smodecount;
		}
	}

	return err;
}

static void svga_cleanup(void)
/*****************************************************************************
 * rex-layer cleanup routine, automatically executed when driver is unloaded.
 *
 * this routine just calls the assembler-level cleanup routine. right now,
 * that routine only does anything if we were using the vesa bios.
 ****************************************************************************/
{
	pj_svga_cleanup();

	device_is_open	 = FALSE;
	device_is_loaded = FALSE;
	smodecount		 = -1;
}

static Errcode svga_get_modes(Vdevice *driver, USHORT mode, Vmode_info *pvm)
/*****************************************************************************
 * fill in the skeleton vminfo structure and return it to caller.
 ****************************************************************************/
{
	Errcode err = Success;
	ULONG	xsize, ysize;

	if (mode >= smodecount) 		// If init/detect failed or caller asked
		return Err_no_such_mode;	// for bad mode, this catches it.

	strcpy(vminf.mode_name, pj_svga_getname());

	xsize = pj_vdrv_modeinfo[mode].width;
	ysize = pj_vdrv_modeinfo[mode].height;

	vminf.mode_ix		= mode;
	vminf.width.min 	= xsize;
	vminf.width.max 	= xsize;
	vminf.width.actual	= xsize;
	vminf.height.min	= ysize;
	vminf.height.max	= ysize;
	vminf.height.actual = ysize;
	vminf.display_bytes = xsize * ysize;

	*pvm = vminf;

	return Success;
}

static char *svga_mode_text(Vdevice *driver, USHORT mode)
/*****************************************************************************
 * return the text description of the requested mode.
 *	 we have one message that covers all modes.
 ****************************************************************************/
{
#define svga_msg  "Auto-detect Super VGA driver, " \
				  "works with most SVGA cards.  "  \
				  "Current hardware is: "

	static struct {
		char thetext[sizeof(svga_msg)-1];
		char dev_name[25];
		} msgbuf = {svga_msg,"unknown hardware"};

	if (mode >= smodecount) 		// If init/detect failed or caller asked
		return NULL;				// for bad mode, this catches it.

	strcpy(msgbuf.dev_name, pj_svga_getname());

	return msgbuf.thetext;
}

static Errcode svga_close_graphics(Vdevice *driver)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (pj_vdrv_wcontrol.localbuf != NULL) {
		pj_free(pj_vdrv_wcontrol.localbuf);
		pj_vdrv_wcontrol.localbuf = NULL;
	}

	if (!device_is_open)
		return Success;

	pj_svga_clrmode(old_mode_data);
	device_is_open = FALSE;

	return Success;
}

static Errcode svga_open_graphics(Vdevice *dev, SVGARast *r,
		LONG width, LONG height, USHORT mode)
/*****************************************************************************
 * open the driver in the requested mode.
 ****************************************************************************/
{
	Errcode err;
	SMInfo	*sminf;

	if (mode >= smodecount) 			// If init/detect failed or caller
		return Err_no_such_mode;		// asked for bad mode, this catches it.

	sminf = &pj_vdrv_modeinfo[mode];

	if (width  != sminf->width ||		// Docs say to do this; I wanna
		height != sminf->height)		// know how it could ever happen
		return Err_wrong_res;			// short of a totally buggy host!

	if (height > 1536)					// ytable size in drvcomn.asm is
		return Err_too_big; 			// hardcodes to this many entries.

	if (width+width+4 > LCLBUF_SIZE)	// blit code assumes it can fit 2 full
		return Err_too_big; 			// lines plus a dword in local buffer.

	if (NULL == (pj_vdrv_wcontrol.localbuf = pj_malloc(LCLBUF_SIZE))) // get blit buffer
		return Err_no_memory;

	r->lib	  = get_rlib(); 						// fill in details in
	r->type   = mode + dev->first_rtype;			// the caller's Raster
	r->width  = width;								// structure.  do it before
	r->height = height; 							// calling setmode() in
	r->pdepth = 8;									// case setmode wants to
	r->grclib = dev->grclib;						// change an rlib vector.

	if (Success > (err = pj_svga_setmode(sminf)))	// fire up requested mode,
		goto ERROR_EXIT;							// punt if error, else
	else											// save longword return val
		old_mode_data = err & 0xFFFFFF7F;			// for passing to clrmode().

	/*
	 * bogus aspect ratio calcs...these are wrong, but at least they don't
	 * break the PJ circle tool...
	 */

	if (r->height < 480) {
		r->aspect_dx = 6;
		r->aspect_dy = 5;
	} else {
		r->aspect_dx = 1;
		r->aspect_dy = 1;
	}

	device_is_open = TRUE;
	return Success;

ERROR_EXIT:

	svga_close_graphics(dev);
	return err;
}

/*****************************************************************************
 * Rex and Vdriver interface stuff...
 ****************************************************************************/

static struct vdevice_lib svga_device_library = {
	svga_init,				/* detect - Is our hardware attached? */
	svga_get_modes, 		/* get modes */
	svga_mode_text, 		/* get extra info */
	NOFUNC, 				/* set max width for height. (not used) */
	svga_open_graphics, 	/* open_graphics */
	svga_close_graphics,	/* close_graphics */
	NOFUNC, 				/* open_cel */
	NOFUNC, 				/* show_rast */
	};

/*----------------------------------------------------------------------------
 * if we're compiling the REX-loadable driver, we need to request the host
 * library support we need, and fill in a rexlib_header structure.
 *--------------------------------------------------------------------------*/

#ifndef FLILIB_CODE

	#define HLIB_TYPE_1 AA_SYSLIB		// we use malloc/free from this lib
	#include "hliblist.h"               // auto-build hostlib linked list

	Vdevice rexlib_header = {
		  { REX_VDRIVER, VDEV_VERSION, svga_init, svga_cleanup, HLIB_LIST },
		  0,									  // first_rtype
		  0,									  // num_rtypes
		  0,									  // mode_count
		  sizeof(Vdevice_lib)/sizeof(int(*)()),   // dev_lib_count
		  &svga_device_library, 				  // lib
		  NULL, 								  // grclib
		  NUM_LIB_CALLS,						  // rast_lib_count
	  };

/*----------------------------------------------------------------------------
 * if we're building the FLILIB driver, we don't need host libraries, and
 * our Vdevice structure can't be named rexlib_header, it has to have its
 * own name.  we also need a local_vdevice structure that points to the
 * Vdevice structure.
 *--------------------------------------------------------------------------*/

#else

	static Vdevice svga_vdevice = {
		  { NULL, NULL, svga_init, svga_cleanup, NULL },
		  0,									  // first_rtype
		  0,									  // num_rtypes
		  0,									  // mode_count
		  sizeof(Vdevice_lib)/sizeof(int(*)()),   // dev_lib_count
		  &svga_device_library, 				  // lib
		  NULL, 								  // grclib
		  NUM_LIB_CALLS,						  // rast_lib_count
	  };

	static struct local_vdevice localdev = {NULL, &svga_vdevice};

	struct local_vdevice *pj_vdev_supervga = &localdev;

#endif	/* FLILIB_CODE */
