/*****************************************************************************
 * PLAYONCE.C - Open flic, play it through once, close it before returning.
 ****************************************************************************/

#include "flicglue.h"

static Boolean until_once_through(UserEventData *eventdata)
/*****************************************************************************
 * event-detector for pj_flic_play_once, stops after one time through flic.
 ****************************************************************************/
{
	return (eventdata->cur_frame < eventdata->num_frames-1);
}

Errcode pj_flic_play_once(char *path, FlicPlayOptions *options)
/*****************************************************************************
 * play a flic once then stop.
 ****************************************************************************/
{
	return pj_flic_play_until(path, options, until_once_through, NULL);
}
