/*****************************************************************************
 * QUICKFLC.C - Standalone simple flic player for SuperVGA.
 *
 * Major Features Demonstrated:
 *
 *	 - pj_video_find_open()
 *	 - pj_play_flic()
 *	 - pj_play_until()
 *	 - Using an event detector routine
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "pjbasics.h"

/*----------------------------------------------------------------------------
 * usage text
 *--------------------------------------------------------------------------*/

static char usage[] =
	"\n"
	"QuickFLC - a program to play back Autodesk Animator and Animator\n"
	"Pro flic files (.FLI and .FLC) on a variety of display devices.\n"
	"\n"
	"QuickFLC will try to find a display resolution supported by the\n"
	"hardware appropriate for the flics.  It will play the flics one\n"
	"after the other, looping through the list continuously until you\n"
	"hit a key to stop the program.\n"
	"\n"
	"Usage:\n"
	"  QUICKFLC flic1 ... flicn\n"
	;

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

Errcode report_error(Errcode err, char *format, ...)
/*****************************************************************************
 * Semi-intelligent error reporting routine.  Prints error message
 * associated with error code, and a (possibly NULL) additional printf
 * formated message.  If the error code indicates there's no real
 * problem (either it's positive or just a user abort) this routine
 * does nothing.  It returns Err_reported if there was an error (to
 * help avoid duplicate error messages), otherwise just what you passed in.
 ****************************************************************************/
{
	va_list args;

	if (err < Success && err != Err_reported && err != Err_abort) {
		va_start(args,format);
		if (format != NULL) {
			vfprintf(stderr, format, args);
			fprintf(stderr,".\n");
		}
		fprintf(stderr, "%s.\n", pj_error_get_message(err), err);
		va_end(args);
		err = Err_reported; 	/* don't report errors twice */
	}
	return err;
}

Errcode biggest_flic_dims(int flic_count, char **flic_list,
						  int *pwidth, int *pheight)
/*****************************************************************************
 * Find the dimensions of the largest flic.
 ****************************************************************************/
{
	Errcode 	err;
	AnimInfo	info;
	char		*flic_name;
	int 		i;
	int 		width  = 0;
	int 		height = 0;

	for (i=0; i<flic_count; ++i) {
		flic_name = *flic_list++;
		if ((err = pj_flic_file_info(flic_name, &info)) < Success)
			return report_error(err, "Couldn't open flic file '%s'", flic_name);
		if (info.width > width)
			width = info.width;
		if (info.height > height)
			height = info.height;
	}
	*pwidth = width;
	*pheight = height;
	return(Success);
}

static Boolean until_key_or_end(UserEventData *eventdata)
/*****************************************************************************
 * event-detector for pj_flic_play, ends playback when a kit is hit.
 ****************************************************************************/
{
	if ((eventdata->cur_frame >= eventdata->num_frames-1) || pj_key_is())
		return FALSE;
	else
		return TRUE;
}


Errcode play_flic_list(FlicRaster *r, int flic_count, char **flic_list)
/*****************************************************************************
 * Loop through flic list playing them one at a time forever.
 ****************************************************************************/
{
	int 	i;
	Errcode err;
	char	*flic_name;

	if (flic_count == 1) {		/* If just one flic call the loop-player */
		flic_name = flic_list[0];
		err = pj_flic_play(flic_name, NULL);
	} else {
		for (;;) {				/* keep going until error condition or abort */
			for (i = 0; i < flic_count; ++i) { /* Play each flic in list once */
				flic_name = flic_list[i];
				pj_raster_clear(r); 			/* clear full hardware screen */
				if ((err = pj_flic_play_until(flic_name, NULL,
								until_key_or_end, NULL)) < Success)
					goto OUT;
				if (pj_key_is()) {
					pj_key_in();
					goto OUT;
				}
			}
		}
	}

OUT:

	return report_error(err, "Trouble playing flic \"%s\"", flic_name);
}


int main(int argc, char **argv)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err = Success;
	int width, height;
	FlicRaster *raster;

	if (argc < 2) {
		puts(usage);
		goto OUT;
	}

	if ((err = biggest_flic_dims(argc-1, argv+1, &width, &height)) < Success)
		goto OUT;

	pj_video_add_all();

	if ((err = pj_video_find_open(width, height, &raster)) < Success) {
		report_error(err, "Couldn't find display device for %d x %d screen",
			width, height);
		goto OUT;
	}

	err =  play_flic_list(raster,argc-1,argv+1);

	pj_video_close(&raster);

OUT:

	return err;
}
