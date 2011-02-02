/*****************************************************************************
 *
 ****************************************************************************/

#include "flicplay.h"

typedef Boolean (EventFunc)(Flic *pflic);

static void rcel_copy(Rcel *d, Rcel *s)
/*****************************************************************************
 * copy a raster.
 ****************************************************************************/
{
	pj_blitrect(s,0,0,d,0,0,d->width,d->height);
	pj_cmap_copy(s->cmap, d->cmap);
}

static Boolean any_user_input(void)
/*****************************************************************************
 * use Poco builtin function to check mouse buttons and keyboard.
 ****************************************************************************/
{
	int x,y,lb,rb,key;

	poePollInput(var2ppt(x), var2ppt(y), var2ppt(lb), var2ppt(rb), var2ppt(key));

	return (key || lb || rb);
}

static Boolean until_keyhit(Flic *notused)
/*****************************************************************************
 * event-detector for flic_play, ends playback when a kit is hit.
 ****************************************************************************/
{
	if (any_user_input())
		return FALSE;	/* stop playback */
	else
		return TRUE;	/* continue playback */
}

static Boolean until_time_expires(Flic *pflic)
/*****************************************************************************
 * event-detector for flic_play_timed, stops after timer exceeds expiry.
 ****************************************************************************/
{
	if (pflic->eventdata < pj_clock_1000())
		return FALSE;	// clock exceeds expiry time, stop the flic
	else
		return TRUE;	// keep playing
}

static Boolean until_once_through(Flic *pflic)
/*****************************************************************************
 * event-detector for flic_play_once, stops after one time through flic.
 ****************************************************************************/
{
	return (pflic->cur_frame < pflic->num_frames-1);
}

static Boolean until_frame_count(Flic *pflic)
/*****************************************************************************
 *
 ****************************************************************************/
{
	return (pflic->frames_played < pflic->eventdata);
}

static Errcode play_until(Flic *pflic, EventFunc *event_detect)
/*****************************************************************************
 * play a flic until the user-specified event routine returns FALSE to stop.
 ****************************************************************************/
{
	Errcode 		err;
	ULONG			clock;
	Flifile 		*flif;
	Fli_head		*flihdr;
	Boolean 		stop_the_playback;

	/*------------------------------------------------------------------------
	 * do some misc setup before starting the actual playback...
	 *----------------------------------------------------------------------*/

	if (pflic->root_raster == GetPicScreen())
		poePicDirtied();

	flif   = pflic->flifile;
	flihdr = &flif->hdr;

	if (pflic->speed < 0)
		pflic->speed = flihdr->speed;

	if (pflic->cur_frame == BEFORE_FIRST_FRAME) {
		pj_seek(flif->fd, flihdr->frame1_oset, JSEEK_START);
	}

	/*------------------------------------------------------------------------
	 * do the playback...
	 *----------------------------------------------------------------------*/

	stop_the_playback = FALSE;

	do	{

		/*--------------------------------------------------------------------
		 * get the current clock now, so that the delta time between frames
		 * includes the time it takes to render the frame.
		 * render the frame.
		 *------------------------------------------------------------------*/

		clock = pflic->speed + pj_clock_1000();

		if(Success > (err = pj_fli_read_uncomp(NULL, flif,
								pflic->playback_raster, pflic->framebuf,TRUE)))
			goto ERROR_EXIT;

		/*--------------------------------------------------------------------
		 * increment the frame counter; if it becomes greater than the frame
		 * count, that means we just displayed the ring frame and have to
		 * seek back to the second frame (not the first (brun'd) frame!).
		 *------------------------------------------------------------------*/

		++pflic->cur_frame;
		++pflic->frames_played;

		if (pflic->cur_frame >= pflic->num_frames) {
			pj_seek(flif->fd, flihdr->frame2_oset, JSEEK_START);
			pflic->cur_frame = 0;
		}

		/*--------------------------------------------------------------------
		 * call the event detector repeatedly, until it requests a stop, or
		 * it's time to display the next frame.  also check the keyboard
		 * after each event_detect() call, if the caller has asked for that.
		 *------------------------------------------------------------------*/

		do	{
			if (FALSE == event_detect(pflic)
			 || (pflic->keyhit_stops_playback && any_user_input())) {
				stop_the_playback = TRUE;
				break;
			}
		} while (clock >= pj_clock_1000());

	} while (stop_the_playback == FALSE);

	err = Success;

ERROR_EXIT:

	return err;
}

void do_rewind(Flic *pflic)
/*****************************************************************************
 * rewind flic; makes next play call start at first frame.
 ****************************************************************************/
{
	pflic->cur_frame = BEFORE_FIRST_FRAME;
	pflic->frames_played = 0;
}

Errcode do_play(Flic *pflic)
/*****************************************************************************
 * play the named flic until a key is hit.
 ****************************************************************************/
{
	return play_until(pflic, until_keyhit);
}

Errcode do_play_timed(Flic *pflic, ULONG for_milliseconds)
/*****************************************************************************
 * play a flic for the specified length of time.
 ****************************************************************************/
{
	pflic->eventdata = for_milliseconds + pj_clock_1000();
	return play_until(pflic, until_time_expires);
}

Errcode do_play_once(Flic *pflic)
/*****************************************************************************
 * play a flic once then stop.
 ****************************************************************************/
{
	return play_until(pflic, until_once_through);
}

Errcode do_play_count(Flic *pflic, int frames_to_play)
/*****************************************************************************
 * play the specified number of frames.
 ****************************************************************************/
{
	if (frames_to_play == 0)
		return Success;

	if (frames_to_play < 0) {
		do_rewind(pflic);
		frames_to_play = -frames_to_play;
	}

	pflic->eventdata = pflic->frames_played + frames_to_play;
	return play_until(pflic, until_frame_count);
}

void do_seek_frame(Flic *pflic, int the_frame)
/*****************************************************************************
 * seek to the specified frame in the flic.  (yuck!  but it's really needed)
 ****************************************************************************/
{
	int 	play_count;
	int 	cur_frame;
	int 	original_speed;
	void	*original_raster;
	Rcel	*seek_raster;
	Boolean seek_raster_used;

	/*------------------------------------------------------------------------
	 * get the current frame into a local var.
	 * if we're already at the requested frame, just punt.
	 *----------------------------------------------------------------------*/

	if (the_frame == (cur_frame = pflic->cur_frame)) {
		return;
	}

	/*------------------------------------------------------------------------
	 * calc the number of frames we have to play to get from where we're at
	 * to where the caller wants to be.  if the requested frame is before
	 * the current frame, we do a rewind before falling into the playback.
	 *----------------------------------------------------------------------*/

	if (the_frame < cur_frame) {
		play_count = ++the_frame;
		do_rewind(pflic);
	} else {
		play_count = the_frame-cur_frame;
	}

	/*------------------------------------------------------------------------
	 * save the current playback speed and raster.
	 *
	 * allocate a temp ram raster to use for the seeking, and set the speed
	 * to zero.  if the raster allocation fails, no problem, we'll just end
	 * up seeking on the playback raster, which may look a bit ugly if the
	 * the playback raster is the video screen, but it's better than failing.
	 *
	 * we skip using the temp raster if the root raster is already a ram
	 * raster, or if we're only going to be playing 1 frame to get to the
	 * requested location.
	 *----------------------------------------------------------------------*/

	original_speed	 = pflic->speed;
	original_raster  = pflic->playback_raster;
	seek_raster_used = FALSE;
	pflic->speed = 0;

	if (pflic->root_raster->type != RT_BYTEMAP && play_count > 1) {
		if (Success <= pj_rcel_bytemap_alloc(original_raster, &seek_raster, COLORS)) {
			rcel_copy(seek_raster, original_raster);
			pflic->playback_raster = seek_raster;
			seek_raster_used = TRUE;
		}
	}

	/*------------------------------------------------------------------------
	 * do the playback to effect the seek.	if the playback is onto the
	 * temp raster, copy the final frame back to the playback raster, then
	 * free the temp raster.  restore the original speed and raster to the
	 * Flic structure.
	 *----------------------------------------------------------------------*/

	do_play_count(pflic, play_count);

	if (seek_raster_used) {
		rcel_copy(original_raster, seek_raster);
		pj_rcel_free(seek_raster);
	}

	pflic->speed = original_speed;
	pflic->playback_raster = original_raster;

	return;
}

