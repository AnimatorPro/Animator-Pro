/*****************************************************************************
 * FLICSPEE.C - Set the speed in a flic header, input or output flics.
 *
 *	When used with an input flic, this will set the playback speed when the
 *	mid-level playback routines (play_next(), play_frames()) are used.
 *
 *	When used with an output flic, this will change the speed specified in
 *	the AnimInfo when pj_flic_create() was called.	The file header won't
 *	actually be updated until pj_flic_finish() is called.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_set_speed(Flic *pflic, int speed)
/*****************************************************************************
 * set a new speed into the flihdr of an open (input or output) flic.
 * if the specified speed is negative, the system default speed is used.
 ****************************************************************************/
{

	if (NULL == pflic)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (NULL == pflic->flifile)
		return Err_file_not_open;

	pflic->flifile->hdr.speed = (speed < 0) ? DEFAULT_AINFO_SPEED : speed;

	return Success;
}
