/*****************************************************************************
 * RTLGLU.C - Some misc glue to bind our -3s code to a -3r client.
 *
 *	This module can be compiled with either the -3r or -3s options.
 *
 ****************************************************************************/

#include <stddef.h>
#include <string.h>
#include <time.h>
#include "pjstypes.h"

#pragma aux (FLICLIB3S) pj_time;

time_t pj_time(time_t *timer)
{
	return time(timer);
}
