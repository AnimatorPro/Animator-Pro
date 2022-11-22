/*****************************************************************************
 * COPYFLIC.C - An example program that copies a flic file frame-by-frame.
 *
 * Major Features Demonstrated:
 *	- Reading a flic file with the low-level playback routines.
 *	- Creating a flic file.
 *	- Working with RAM rasters.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "pjbasics.h"

static Errcode do_the_copy(Flic *inflic, Flic *outflic, AnimInfo *ainfo)
/*****************************************************************************
 * Copy a flic a frame at a time.
 *	There's no especially good reason to copy a flic file like this, except
 *	for purposes of demonstration.
 ****************************************************************************/
{
	Errcode 	err;
	int 		width, height;
	int 		counter;
	int 		numframes;
	FlicRaster	*thisframe	= NULL;
	FlicRaster	*priorframe = NULL;

	width	  = ainfo->width;
	height	  = ainfo->height;
	numframes = ainfo->num_frames;

	/*------------------------------------------------------------------------
	 * get a couple of RAM rasters to serve as the current and prior frames.
	 *----------------------------------------------------------------------*/

	if (Success > (err = pj_raster_make_ram(&thisframe, width, height)))
		goto ERROR_EXIT;
	if (Success > (err = pj_raster_make_ram(&priorframe, width, height)))
		goto ERROR_EXIT;

	/*------------------------------------------------------------------------
	 * copy the first frame of the flic.
	 *----------------------------------------------------------------------*/

	fprintf(stdout, "Copying frame 1\r");
	fflush(stdout);

	if (Success > (err = pj_flic_play_next(inflic, thisframe)))
		goto ERROR_EXIT;
	if (Success > (err= pj_flic_write_first(outflic, thisframe)))
		goto ERROR_EXIT;

	/*------------------------------------------------------------------------
	 * copy the rest of the frames...
	 *	 we copy the contents of thisframe to the priorframe raster,
	 *	 then read the next input frame into the thisframe raster, then
	 *	 write the output deltas based on the differences between priorframe
	 *	 and thisframe.
	 *----------------------------------------------------------------------*/

	for (counter = 2; counter <= numframes; ++counter) {

		pj_raster_copy(thisframe, priorframe);

		fprintf(stdout, "Copying frame %d\r", counter);
		fflush(stdout);

		if (Success > (err = pj_flic_play_next(inflic, thisframe)))
			goto ERROR_EXIT;
		if (Success > (err= pj_flic_write_next(outflic, thisframe, priorframe)))
			goto ERROR_EXIT;
	}

	/*------------------------------------------------------------------------
	 * write the ring frame to finish off the output flic.
	 *	the ring frame is built from the contents of the last frame we wrote
	 *	(which are still in the thisframe raster)
	 *----------------------------------------------------------------------*/

	fprintf(stdout, "Making ring frame       \r", counter);
	fflush(stdout);
	if (Success > (err= pj_flic_write_finish(outflic, thisframe)))
		goto ERROR_EXIT;

	fprintf(stdout, "All done, %d frames copied. \n", counter);
	fflush(stdout);

	err = Success;

ERROR_EXIT:

	pj_raster_free_ram(&thisframe);
	pj_raster_free_ram(&priorframe);

	return err;
}

void main(int argc, char **argv)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode 	err;
	Flic		inflic;
	Flic		outflic;
	AnimInfo	flicinfo;

	if (argc != 3) {
		fprintf(stdout, "Usage: COPYFLIC infile outfile\n");
		exit(1);
	}

	if (Success <= (err = pj_flic_open_info(argv[1], &inflic, &flicinfo))) {
		if (Success <= (err = pj_flic_create(argv[2], &outflic, &flicinfo))) {
			err = do_the_copy(&inflic, &outflic, &flicinfo);
			pj_flic_close(&outflic);
		}
		pj_flic_close(&inflic);
	}
	if (Success > err)
		fprintf(stderr, "Error copying flic '%s' to '%s'\n%s.\n"
			   , argv[1], argv[2], pj_error_get_message(err));
	exit(err);
}
