/*****************************************************************************
 * PLAYTIME.C - Open flic, play it for the specified time, close & return.
 ****************************************************************************/

#include "flicglue.h"

static ULONG expiration_time;

static Boolean until_time_expires(UserEventData *eventdata)
/*****************************************************************************
 * event-detector for pj_flic_play_timed, stops after timer exceeds expiry.
 ****************************************************************************/
{
	if (expiration_time < pj_clock_1000())
		return FALSE;	// clock exceeds expiry time, stop the flic
	else
		return TRUE;	// keep playing
}

Errcode pj_flic_play_timed(char *path, FlicPlayOptions *options,
						   ULONG for_milliseconds)
/*****************************************************************************
 * play a flic for the specified length of time.
 ****************************************************************************/
{
	Errcode err;
	Boolean we_initialized_clock = FALSE;

	we_initialized_clock = !pj_clock_init();

	expiration_time = for_milliseconds + pj_clock_1000();

	err = pj_flic_play_until(path, options, until_time_expires, NULL);

	if (we_initialized_clock)
		pj_clock_cleanup();

	return err;
}
