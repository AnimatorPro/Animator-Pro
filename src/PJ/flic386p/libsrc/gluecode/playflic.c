/*****************************************************************************
 * PLAYFLIC.C - Open flic, play until keyhit, close flic before returning.
 ****************************************************************************/

#include "flicglue.h"
#include "asm.h"

static Boolean until_keyhit(UserEventData *notused)
/*****************************************************************************
 * event-detector for pj_flic_play, ends playback when a kit is hit.
 ****************************************************************************/
{
	if (pj_key_is()) {
		pj_key_in();
		return FALSE;
	}
	return TRUE;
}

Errcode pj_flic_play(char *path, FlicPlayOptions *options)
/*****************************************************************************
 * play the named flic until a key is hit.
 ****************************************************************************/
{
	return pj_flic_play_until(path, options, until_keyhit, NULL);
}
