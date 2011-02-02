/*****************************************************************************
 * PLAYFRAM.C - Play one or more frames from a pre-opened flic.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_play_frames(Flic *pflic, FlicRaster *playrast, int count)
/*****************************************************************************
 * play the specified number of frames.  will loop at end of flic.
 *
 * if the count value is zero, the routine returns immediately (no-op).
 * if the count value is negative, the absolute value is used as the count,
 * and the flic is rewound to the first frame before starting playback.
 *
 * Returns number of next frame to be displayed, or negative error code.
 ****************************************************************************/
{
	Errcode 		err;
	Boolean 		we_initialized_clock = FALSE;
	ULONG			clock;
	Fli_frame		*ff = NULL;
	Flifile 		*flif;
	Fli_head		*flihdr;
	FliLibCtl		*libctl;
	FlicRaster		workrast;
	FlicRaster		*therast;

	/*------------------------------------------------------------------------
	 * verify arguments...
	 *----------------------------------------------------------------------*/

	if (0 == count)
		return Success;

	if (NULL == playrast || NULL == pflic || NULL == pflic->flifile)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (JREADONLY != pflic->libctl->iotype)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	/*------------------------------------------------------------------------
	 * do some misc setup before starting the actual playback...
	 *----------------------------------------------------------------------*/

	we_initialized_clock = !pj_clock_init();

	libctl = pflic->libctl;
	flif   = pflic->flifile;
	flihdr = &flif->hdr;

	if(Success > (err = pj_fli_alloc_cbuf(&ff,flihdr->width,flihdr->height,COLORS))) {
		goto ERROR_EXIT;
	}

	if (0 > count) {
		count = -count;
		libctl->cur_frame = 0;
	}

	if (0 == libctl->cur_frame)
		pj_seek(flif->fd, flihdr->frame1_oset, JSEEK_START);

	therast = pj_raster_center_virtual(playrast, &workrast, flihdr->width, flihdr->height);

	/*------------------------------------------------------------------------
	 * do the playback...
	 *----------------------------------------------------------------------*/

	do	{
		clock = flihdr->speed + pj_clock_1000();
		if(Success > (err = pj_fli_read_uncomp(NULL,flif,(Rcel *)therast,ff,TRUE)))
			goto ERROR_EXIT;
		while (clock >= pj_clock_1000())
			continue;

		++libctl->cur_frame;
		if (libctl->cur_frame > flihdr->frame_count) {
			pj_seek(flif->fd, flihdr->frame2_oset, JSEEK_START);
			libctl->cur_frame = 1;
		}
	} while (--count);

	/*------------------------------------------------------------------------
	 * set the return value to the number of the next frame we're gonna play.
	 * if the next frame is the ring frame, that's really the first frame
	 * (frame 0), so that's what we return.
	 *----------------------------------------------------------------------*/

	if (libctl->cur_frame == flihdr->frame_count)
		err = 0;
	else
		err = libctl->cur_frame;

ERROR_EXIT:

	if (NULL != ff)
		pj_free(ff);

	if (we_initialized_clock)
		pj_clock_cleanup();

	return err;
}
