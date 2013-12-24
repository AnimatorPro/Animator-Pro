/*****************************************************************************
 * stepflic.c - Single-step through frames in a flic.
 *				Demo's low-level playback function.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "pjbasics.h"

extern Boolean	pj_key_is(void);	/* FlicLib util to key keyboard. */
extern int		pj_key_in(void);	/* FlicLib util to get keystroke.*/

void fatal(Errcode err, char *fmt, ...)
/*****************************************************************************
 * report an error and die.
 ****************************************************************************/
{
	va_list args;

	pj_video_close(NULL);	/* force video back to text mode. */

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fprintf(stderr, pj_error_get_message(err));

	exit(err);

}

Errcode pj_error_internal(Errcode err, char *module, int line)
{
	fatal(err, "FlicLib module %s detected error at line %d\n", module, line);
	return err; /* just to keep compiler from whining. */
}

void main(int argc, char **argv)
{
	Errcode 	 err;
	Flic		 flic;
	int 		 next_frame;
	FlicRaster	 *video_raster;
	FlicRaster	 *playback_raster;
	AnimInfo	 info;

	if (argc < 2) {
		printf("Usage: STEPFLIC filename\n");
		exit(1);
	}

	pj_video_add_all();

	if ((err = pj_flic_open_info(argv[1], &flic, &info)) < Success)
		fatal(err, "Cannot open flic '%s'\n", argv[1]);

	if ((err = pj_video_find_open(info.width, info.height, &video_raster)) < Success)
		fatal(err, "Cannot open video\n");

	pj_raster_make_centered(&playback_raster, video_raster, &flic);

	for (;;) {
		next_frame = pj_flic_play_next(&flic, playback_raster);
		if (next_frame < Success)
			fatal(next_frame, "Error playing flic\n");
		if (next_frame == 0)
			break;
		while (!pj_key_is())
			continue;
		pj_key_in();

	}

	pj_flic_close(&flic);

	pj_raster_free_centered(&playback_raster);
	pj_video_close(&video_raster);

	exit(0);
}
