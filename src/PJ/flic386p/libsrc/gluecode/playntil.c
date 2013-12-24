/*****************************************************************************
 * PLAYNTIL.C - Open flic, play it until event-detect, close the flic.
 ****************************************************************************/

#include "flicglue.h"
#include "asm.h"

static char *modulename = __FILE__;

Errcode pj_flic_play_until(char *path, FlicPlayOptions *requested_options,
						   UserEventFunc *event_detect, void *udata)
/*****************************************************************************
 * play a flic until the user-specified event routine returns FALSE to stop.
 *
 * Returns:
 *	Success if ended due to user's event detect routine.
 *	   1	if ended due to a keyhit and options.keyhit_stops_playback==TRUE.
 *	 (neg)	if ended due to error.
 ****************************************************************************/
{
	Errcode 		err;
	Boolean 		we_opened_video = FALSE;
	Boolean 		we_initialized_clock = FALSE;
	ULONG			clock;
	ULONG			num_frames;
	FlicRaster		virt_raster;
	FlicRaster		*root_raster = NULL;
	FlicRaster		*playback_raster = NULL;
	Fli_frame		*ff = NULL;
	Flic			theflic = {0};
	Flifile 		*flif;
	Fli_head		*flihdr;
	FliLibCtl		*libctl;

	UserEventData	event_data;
	FlicPlayOptions options;

	/*------------------------------------------------------------------------
	 * validate parms, init options, etc.
	 *----------------------------------------------------------------------*/

	if (NULL == path || NULL == event_detect)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (NULL == requested_options) {
		pj_playoptions_init(&options);
	} else {
		options = *requested_options;
	}

	/*------------------------------------------------------------------------
	 * open the flic.
	 *----------------------------------------------------------------------*/

	err = pj_flic_open(path, &theflic);
	if (Success > err)
		goto ERROR_EXIT;

	flif   = theflic.flifile;
	flihdr = &flif->hdr;
	libctl = theflic.libctl;

	/*------------------------------------------------------------------------
	 * open the video raster, or use the caller-provided raster.
	 *----------------------------------------------------------------------*/

	if (NULL != options.playback_raster) {
		root_raster = (FlicRaster *)options.playback_raster;
		we_opened_video = FALSE;
	} else {
		err = pj_video_get_current(NULL, NULL, &root_raster);
		if (Success > err) {
			err = pj_video_find_open(flihdr->width, flihdr->height, &root_raster);
			if (Success > err)
				goto ERROR_EXIT;
			we_opened_video = TRUE;
		}
	}

	/*------------------------------------------------------------------------
	 * setup placement of the playback raster based on the caller-provided
	 * options, or the default option of centering the flic on the raster.
	 *----------------------------------------------------------------------*/

	if (0 == options.x && 0 == options.y) {
		playback_raster = pj_raster_center_virtual(root_raster, &virt_raster,
												 flihdr->width, flihdr->height);
	} else {
		playback_raster = pj_raster_clip_virtual(root_raster, &virt_raster,
							options.x, options.y, flihdr->width, flihdr->height);
		if (NULL == playback_raster) {
			err = Err_clipped;
			goto ERROR_EXIT;	/* totally clipped, just punt */
		}
	}

	pj_raster_clear(playback_raster);

	/*------------------------------------------------------------------------
	 * do some misc setup before starting the actual playback...
	 *----------------------------------------------------------------------*/

	if(Success > (err = pj_fli_alloc_cbuf(&ff,flihdr->width,flihdr->height,COLORS))) {
		goto ERROR_EXIT;
	}

	num_frames				= flihdr->frame_count;
	theflic.userdata		= udata;
	event_data.userdata 	= udata;
	event_data.flic 		= &theflic;
	event_data.cur_loop 	= 0;
	event_data.cur_frame	= 0;
	event_data.num_frames	= num_frames;

	if (options.speed < 0)
		options.speed = flihdr->speed;

	we_initialized_clock = !pj_clock_init();

	/*------------------------------------------------------------------------
	 * do the playback...
	 *----------------------------------------------------------------------*/

	for (;;) {

		/*--------------------------------------------------------------------
		 * the first time through, and if the event detector has called
		 * pj_flic_rewind(), libctl->cur_frame is zero, and we have to seek
		 * back to frame 1 (the brun frame).
		 *------------------------------------------------------------------*/

		if (0 == libctl->cur_frame)
			pj_seek(flif->fd, flihdr->frame1_oset, JSEEK_START);

		/*--------------------------------------------------------------------
		 * if the frame we're about to display is the ring frame, we increment
		 * the cur_loop counter, and set the cur_frame to zero.  that's
		 * because displaying the ring frame is really a fast way to display
		 * frame zero without un-brun'ing it again.
		 *------------------------------------------------------------------*/

		if (num_frames == libctl->cur_frame) {
			event_data.cur_frame = 0;
			++event_data.cur_loop;
		} else {
			event_data.cur_frame = libctl->cur_frame;
		}

		/*--------------------------------------------------------------------
		 * get the current clock now, so that the delta time between frames
		 * included the time it takes to render the frame.
		 * render the frame.
		 *------------------------------------------------------------------*/

		clock = options.speed + pj_clock_1000();
		if(Success > (err = pj_fli_read_uncomp(path,flif,(Rcel *)playback_raster,ff,TRUE)))
			goto ERROR_EXIT;

		/*--------------------------------------------------------------------
		 * call the event detector repeatedly, until it requests a stop, or
		 * it's time to display the next frame.  also check the keyboard
		 * after each event_detect() call, if the caller has asked for that.
		 *------------------------------------------------------------------*/

		do	{
			if (FALSE == event_detect(&event_data)) {
				err = Success;
				goto ERROR_EXIT;
			}
			if (options.keyhit_stops_playback) {
				if (pj_key_is()) {
					pj_key_in();
					err = 1;
					goto ERROR_EXIT;
				}
			}
		} while (clock >= pj_clock_1000());

		/*--------------------------------------------------------------------
		 * increment the frame counter; if it becomes greater than the frame
		 * count, that means we just displayed the ring frame and have to
		 * seek back to the second frame (not the first (brun'd) frame!).
		 *------------------------------------------------------------------*/

		if (++libctl->cur_frame > num_frames) {
			pj_seek(flif->fd, flihdr->frame2_oset, JSEEK_START);
			libctl->cur_frame = 1;
		}
	}

ERROR_EXIT:

	pj_flic_close(&theflic);

	if (NULL != ff)
		pj_free(ff);

	if (we_opened_video)
		pj_video_close(&root_raster);

	if (we_initialized_clock)
		pj_clock_cleanup();

	return err;
}
