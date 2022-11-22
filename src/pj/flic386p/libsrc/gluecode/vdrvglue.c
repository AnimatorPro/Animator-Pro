/*****************************************************************************
 * VDRVGLUE.C - Most all of the glue for managing builtin video drivers.
 *
 *	There's a whole lot of groodah in this one module because:
 *	 1) It's all interrelated anyway, using one of these functions generally
 *		means using most of them (internally, if not explicitly).
 *	 2) It lets us hide some blarf in static vars that the client doesn't
 *		need to worry itself about.
 ****************************************************************************/


#define VDEV_INTERNALS		/* we need to see internals of vdevice structure */

#include "flicglue.h"
#include "vdevice.h"        /* must be #included after flicglue.h! */

static char *modulename = __FILE__;

extern void pj_set_gs(void); // routine to pre-load PHAR_REAL_SEG into GS reg.

/*----------------------------------------------------------------------------
 * some local data for managing the video devices..
 *--------------------------------------------------------------------------*/

static LocalVdevice *pdevlist = NULL;	 // list of added drivers
static LocalVdevice *pldevcur = NULL;	 // driver that's currently open
static Vdevice		*pdevcur  = NULL;	 // driver that's currently open
static int			modecur   = -1; 	 // mode that's currently open

static PjCmap		video_cmap;
static FlicRaster	video_raster;

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

static void device_unload(Vdevice *pdev, Boolean unload_current)
/*****************************************************************************
 * do a psuedo-unload of a builtin device: call its rex-layer cleanup().
 *	this function will only unload the current device if explicitly requested
 *	to do so via the unload_current parm.  this allows us to do sequences of
 *	load/detect/getmodeinfo/unload on devices including the current device
 *	without inadvertantly unloading the current device. only the video_close()
 *	function will pass a TRUE value for unload_current.
 ****************************************************************************/
{
	if (unload_current == FALSE && pdev == pdevcur)
		return;

	if (NULL != pdev->hdr.cleanup)
		pdev->hdr.cleanup(pdev);
}

static Errcode device_load(Vdevice *pdev)
/*****************************************************************************
 * do a psuedo-load of a builtin device: call its rex-layer init().
 ****************************************************************************/
{
	static Boolean	set_gs_done = FALSE;

	if (!set_gs_done) {
		set_gs_done = TRUE;
		pj_set_gs();
	}

	if (pdev == pdevcur)	// don't re-init current device!
		return Success;

	if (NULL != pdev->hdr.init)
		return pdev->hdr.init(pdev);
	else
		return Success;
}

static Errcode device_detect(Vdevice *pdev)
/*****************************************************************************
 * call a device driver's detect() function, if it exists.
 * (it had better exist, because PJ et. al. require it!)
 ****************************************************************************/
{
	if (pdev == pdevcur)	// don't re-check current device, we know it can
		return Success; 	// handle the hardware (it's doing so, after all).

	if (NULL != pdev->lib->detect)
		return pdev->lib->detect(pdev);
	else
		return Success;
}

Errcode pj_video_detect(LocalVdevice **ppldev)
/*****************************************************************************
 * checks all eligible drivers to see if one can handle the video hardware.
 *
 * Returns:
 *	1				the MCGA driver was the only thing found.
 *	Success 		any other driver responded to the hardware.
 *	Err_no_display	no eligible driver was found to handle the hardware.
 ****************************************************************************/
{
	Errcode 		err;
	Vdevice 		*thisdev;
	Boolean 		found_mcga = FALSE;
	LocalVdevice	*pcur;
	LocalVdevice	*theldev = NULL;

	for (pcur = pdevlist; NULL == theldev && NULL != pcur; pcur = pcur->next) {
		thisdev = pcur->device;
		if (Success <= device_load(thisdev)) {
			if (Success <= device_detect(thisdev)) {
				if (pcur == pj_vdev_mcga) {
					found_mcga = TRUE;
				} else {
					theldev = pcur;
				}
			}
			device_unload(thisdev, FALSE);
		}
	}

	if (NULL != theldev) {
		err = Success;
	} else if (found_mcga) {
		err = 1;
		theldev = pj_vdev_mcga;
	} else {
		err = Err_no_display;
	}

	if (NULL != ppldev)
		*ppldev = theldev;

	return err;
}

void pj_video_close(FlicRaster **pprast)
/*****************************************************************************
 * closes any currently-open driver, restores prior video mode.
 * can be called any number of times, whether driver is currently open or not.
 * may be called with pprast==NULL without error.
 ****************************************************************************/
{
	if (NULL != pdevcur) {
		pdevcur->lib->close_graphics(pdevcur);
		device_unload(pdevcur, TRUE);
	}

	pldevcur = NULL;
	pdevcur  = NULL;
	modecur  = -1;

	if (NULL != pprast)
		*pprast = NULL;
}


Errcode pj_video_open(LocalVdevice *pldev, int mode, FlicRaster **pprast)
/*****************************************************************************
 * opens specified driver and mode.
 * will force a close on any driver open at the time of the call.
 *
 * Returns:
 *	Success 		driver & mode opened successfully.
 *	Err_no_display	if driver can't handle hardware.
 *	(other errors)	as appropriate
 *
 *	*pprast 		contains raster handle, or NULL if return != Success.
 ****************************************************************************/
{
	Errcode 	err;
	long		w;
	long		h;
	Vdevice 	*pdev;
	Vdevice_lib *plib;
	Vmode_info	modeinfo;

	if (NULL == pprast || NULL == pldev)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (0 > mode)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	pdev = pldev->device;
	plib = pdev->lib;

	pj_video_close(pprast); 					// close current device (if any).

	if (Success > (err = device_load(pdev)))	// load new device.
		return err;

	if (Success > (err = device_detect(pdev)))	// see if driver can handle
		return err; 							// this hardware, punt if not.

	pdev->first_rtype = RT_FIRST_VDRIVER;
	pdev->grclib = pj_get_grc_lib();

	if (Success > (err = plib->get_modes(pdev, mode, &modeinfo)))
		return err;

	if (NULL != plib->set_max_height)
		plib->set_max_height(pdev, &modeinfo);

	w = modeinfo.width.actual;
	h = modeinfo.height.actual;

	if (Success > (err = plib->open_graphics(pdev, &video_raster, w, h, mode)))
		return err;

	modecur  = mode;
	pldevcur = pldev;
	pdevcur  = pdev;

	video_cmap.num_colors = COLORS;
	video_raster.cmap = &video_cmap;
	pj_set_grc_calls(video_raster.lib);

	pj_set_rast(&video_raster, 0);

	*pprast  = &video_raster;
	return Success;
}

Errcode pj_video_mode_info(LocalVdevice *pldev, int mode,
						   int *pwidth, int *pheight)
/*****************************************************************************
 * returns width/height info about the specified driver and mode.
 *
 * Returns:
 *	Success 		 driver can handle hardware and supports the mode.
 *	Err_no_display	 driver cannot handle the hardware.
 *	Err_no_such_mode driver doesn't recognize the mode you asked for.
 *	(other errors)	 as appropriate.
 *
 * When return == Success:
 *	*pwidth 		contains screen width for mode.
 *	*pheight		contains screen height for mode.
 ****************************************************************************/
{
	Errcode 	err;
	Vdevice 	*pdev;
	Vmode_info	modeinfo;

	if (NULL == pldev || NULL == pwidth || NULL == pheight)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (0 > mode)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	pdev = pldev->device;

	if (Success > (err = device_load(pdev)))
		return err;

	if (Success <= (err = device_detect(pdev))) {
		if (Success <= (err = pdev->lib->get_modes(pdev, mode, &modeinfo))) {
			*pwidth  = modeinfo.width.actual ;
			*pheight = modeinfo.height.actual ;
		}
	}

	device_unload(pdev, FALSE);

	return err;
}

Errcode pj_video_find(LocalVdevice **ppldev, int *pmode, int width, int height)
/*****************************************************************************
 * finds eligible driver that has mode closest to specified width and height.
 *
 * Returns:
 *	2				the screen will be bigger than requested width/height.
 *	1				the screen will be smaller than requested width/height.
 *	Success 		an exact fit was found.
 *	Err_no_display	no eligible driver was found to handle the hardware.
 *	(other errors)	as appropriate
 *
 *	*ppldev 		contains driver handle or NULL if no device found.
 *	*pmode			contains video mode or -1 if no device found.
 ****************************************************************************/
{
	Errcode 		err;
	LocalVdevice	*thisldev;
	Vmode_info		modeinfo;
	Vdevice_lib 	*plib;
	Vdevice 		*thisdev;
	int 			thisxdif;
	int 			thisydif;
	long			thisdif;
	int 			thismode;
	LocalVdevice	*bestldev = NULL;
	int 			bestmode  = -1;
	long			bestdif   = 2*(9999*9999);
	enum {
		NO_FIT	= Err_no_display,
		EXACT	= Success,
		SMALLER = 1,
		LARGER	= 2,
		}			bestfit = NO_FIT;

	if (NULL == ppldev || NULL == pmode)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (0 >= width || 0 >= height)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	/*------------------------------------------------------------------------
	 * loop through all eligible devices until we either find an exact fit
	 * or run out of eligible devices...
	 *----------------------------------------------------------------------*/

	for (thisldev = pdevlist; EXACT != bestfit && NULL != thisldev; thisldev = thisldev->next) {

		thisdev = thisldev->device;
		plib = thisdev->lib;

		if (Success <= device_load(thisdev) && Success <= device_detect(thisdev)) {

			/*----------------------------------------------------------------
			 * loop through all modes for this device, until we either find
			 * an exact fit or run out of modes...
			 *--------------------------------------------------------------*/

			err = Success;
			for (thismode = 0; EXACT != bestfit && Success <= err; ++thismode) {

				if (Success <= (err = plib->get_modes(thisdev, thismode, &modeinfo))) {
					thisxdif = modeinfo.width.actual  - width;
					thisydif = modeinfo.height.actual - height;
					thisdif  = (thisxdif * thisxdif) + (thisydif * thisydif);
					if (thisdif < bestdif) {
						bestdif  = thisdif;
						bestldev = thisldev;
						bestmode = thismode;
						if (0 == thisdif)
							bestfit = EXACT;
						else if (0 > thisxdif || 0 > thisydif)
							bestfit = SMALLER;
						else
							bestfit = LARGER;
					} /* END if (found a better fit than prior devs/modes) */

				} /* END if (device returned info for this mode) */

			} /* END for(modes in this device) */

			device_unload(thisdev, FALSE);

		} /* END if(device loaded and detected okay) */

	} /* END for(device in the list) */

	*ppldev = bestldev;
	*pmode = bestmode;

	return bestfit;

}

Errcode pj_video_find_open(int width, int height, FlicRaster **pprast)
/*****************************************************************************
 * finds & opens eligible driver that has mode closest to width & height.
 * will force a close on any driver open at the time of the call.
 *
 * Returns:
 *	2				the screen is bigger than requested width/height.
 *	1				the screen is smaller than requested width/height.
 *	Success 		an exact fit was found.
 *	Err_no_display	no eligible driver was found to handle the hardware.
 *	(other errors)	as appropriate
 *
 * When return >= Success:
 *	*pprast 		contains raster handle.
 ****************************************************************************/
{
	Errcode 		err;
	LocalVdevice	*bestldev;
	int 			bestmode;
	int 			fit;

	if (Success <= (fit = pj_video_find(&bestldev, &bestmode, width, height)))
		err = pj_video_open(bestldev, bestmode, pprast);
	else
		err = fit;

	if (Success <= err)
		return fit;
	else
		return err;

}

Errcode pj_video_get_current(LocalVdevice **ppldev, int *pmode,
							 FlicRaster **pprast)
/*****************************************************************************
 * returns info about the currently-open video driver & mode (if any).
 *
 * Returns:
 *	Success 		if a driver is open and info is returned.
 *	Err_uninit		if no driver is currently open.
 *
 * When return == Success:
 *	*ppdev			driver handle  (if ppdev  != NULL).
 *	*pmode			current mode   (if pmode  != NULL).
 *	*pprast 		raster handle  (if pprast != NULL).
 ****************************************************************************/
{
	if (NULL == pdevcur)
		return Err_uninit;

	if (NULL != ppldev)
		*ppldev = pldevcur;
	if (NULL != pmode)
		*pmode = modecur;
	if (NULL != pprast)
		*pprast = &video_raster;

	return Success;
}

void pj_video_add(LocalVdevice *pldev)
/*****************************************************************************
 * adds a driver to the list of eligible drivers.
 * - if pdev arg is NULL, nukes entire current eligibility list.
 * - will not add a driver if it's on the list already.
 * - functions which search the list of eligible drivers do so in reverse
 *	 order of how devices were added with calls to this function.
 ****************************************************************************/
{
	LocalVdevice	*pcur;
	LocalVdevice	*pnext;

	if (NULL == pldev) {
		for (pcur = pdevlist; pcur != NULL; pcur = pnext) {
			pnext = pcur->next;
			pcur->next = NULL;
		}
		pdevlist = NULL;
	} else {
		for (pcur = pdevlist; pcur != NULL; pcur = pnext) {
			pnext = pcur->next;
			if (pcur == pldev)
				return;
		}
		pldev->next = pdevlist;
		pdevlist = pldev;
	}
}
