/*****************************************************************************
 * PLAYFRAM.C - An example of using mid-level playback to play a few frames.
 *
 * Major Features Demonstrated:
 *	- Using mid-level playback functions.
 *	- Using a centered virtual raster for playback.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "pjbasics.h"

void main(int argc, char **argv)
{
	Errcode 	 err;
	Flic		 flic;
	FlicRaster	 *hardware_raster;
	FlicRaster	 *playback_raster;
	AnimInfo	 info;
	int 		 num_frames = 0;
	int 		 speed = -1;

	if (argc < 2) {
		printf("Usage: PLAYFRAM filename [num_frames] [speed]\n");
		exit(1);
	}

	if (argc >= 3)
		num_frames = atoi(argv[2]);

	if (num_frames <= 0)
		num_frames = 10;

	if (argc >= 4)
		speed = atoi(argv[3]);

	pj_video_add_all(); 

	if ((err = pj_flic_open_info(argv[1], &flic, &info)) < Success) {
		printf("Cannot open flic, status = %d\n(%s)\n", err, pj_error_get_message(err));
		exit(err);
	}

	if ((err = pj_video_find_open(info.width, info.height, &hardware_raster)) < Success) {
		printf("Cannot open video, status = %d\n(%s)\n", err, pj_error_get_message(err));
		exit(err);
	}

	pj_raster_make_centered(&playback_raster, hardware_raster, &flic);
	pj_flic_set_speed(&flic, speed);

	err = pj_flic_play_frames(&flic, playback_raster, num_frames);
	pj_flic_close(&flic);

	pj_raster_free_centered(&playback_raster);
	pj_video_close(&hardware_raster);

	if (Success > err) {
		printf("Error playing flic, status = %d\n(%s)\n", err, pj_error_get_message(err));
	}

	exit(err);
}
