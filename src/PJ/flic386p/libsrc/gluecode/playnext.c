/*****************************************************************************
 * PLAYNEXT.C - Play one frame, but don't wait for inter-frame delay period.
 *
 *	(This is basically a clone of play_frames(), except that looping and
 *	 clock works have been eliminated and it always plays just one frame.)
 *
 *	If the client code has done something special to the system clocks, and
 *	finds that fliclib clock routines interfere with that in some way, this
 *	routine can be used as the core of a client-coded playback system, in
 *	which the client handles event detection and inter-frame delays.  Code
 *	in the pj_flic_play_until() routine (PLAYNTIL.C), together with this
 *	module, demonstrate what's needed in terms of inter-frame processing.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_play_next(Flic *pflic, FlicRaster *playrast)
/*****************************************************************************
 * play the next frame, without delay.	will loop at end of flic.
 * Returns number of next frame, or negative error code.
 ****************************************************************************/
{
	Errcode 		err;
	Fli_frame		*ff = NULL;
	Flifile 		*flif;
	Fli_head		*flihdr;
	FliLibCtl		*libctl;
	FlicRaster		workrast;
	FlicRaster		*therast;

	/*------------------------------------------------------------------------
	 * verify arguments...
	 *----------------------------------------------------------------------*/

	if (NULL == playrast || NULL == pflic || NULL == pflic->flifile)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (JREADONLY != pflic->libctl->iotype)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	/*------------------------------------------------------------------------
	 * do some misc setup before starting the actual playback...
	 *----------------------------------------------------------------------*/

	libctl = pflic->libctl;
	flif   = pflic->flifile;
	flihdr = &flif->hdr;

	if(Success > (err = pj_fli_alloc_cbuf(&ff,flihdr->width,flihdr->height,COLORS))) {
		goto ERROR_EXIT;
	}

	if (0 == libctl->cur_frame)
		pj_seek(flif->fd, flihdr->frame1_oset, JSEEK_START);

	therast = pj_raster_center_virtual(playrast, &workrast, flihdr->width, flihdr->height);

	/*------------------------------------------------------------------------
	 * do the playback...
	 *----------------------------------------------------------------------*/

	if(Success > (err = pj_fli_read_uncomp(NULL,flif,(Rcel *)therast,ff,TRUE)))
		goto ERROR_EXIT;

	++libctl->cur_frame;

	if (libctl->cur_frame > flihdr->frame_count) {
		pj_seek(flif->fd, flihdr->frame2_oset, JSEEK_START);
		libctl->cur_frame = 1;
	}

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

	return err;
}
