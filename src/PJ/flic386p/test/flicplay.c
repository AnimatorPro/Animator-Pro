/*****************************************************************************
 * FLICPLAY.C - Demo of the simplest use of the high-level playback routines.
 *	This will play one or more flics specified on the command line.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "pjbasics.h"

void main(int argc, char **argv)
{
	Errcode err;
	FlicPlayOptions options;

	if (argc < 2) {
		printf("Usage: FLICPLAY filename [filename ...] \n");
		exit(1);
	}

	pj_video_add_all();

	if (argc == 2)	/* One file in command line, loop it forever. */
		err = pj_flic_play(argv[1], NULL);
	else			/* More than one file in command line.	Go through each */
		{
		int i = 1;
		pj_playoptions_init(&options);
		options.keyhit_stops_playback = TRUE;
		for (;;)
			{
			err = pj_flic_play_once(argv[i], &options);
			if (err < Success || err == 1) /* on error or keyhit, stop */
				break;
			if (++i >= argc)
				i = 1;
			}
		}

	if (Success > err) {
		printf("Error playing flic, status = %d\n%s\n", err,
			pj_error_get_message(err));
	}

	exit(err);
}
