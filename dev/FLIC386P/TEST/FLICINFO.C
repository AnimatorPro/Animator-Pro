/*****************************************************************************
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "pjbasics.h"

void main(int argc, char **argv)
{
	Errcode  err;
	AnimInfo flic_info;

	if (argc != 2) {
		printf("Usage: FLI_INFO filename.fli\n");
		exit(1);
	}

	if (Success > (err = pj_flic_file_info(argv[1], &flic_info))) {
		printf("Cannot get Flic info, status = %d\n", err);
		exit(err);
	}

	printf("Flic info is:\n"
		   "width      = %d\n"
		   "height     = %d\n"
		   "x          = %d\n"
		   "y          = %d\n"
		   "num_frames = %d\n"
		   "depth      = %d\n"
		   "millisec   = %d\n"
		   "aspect_dx  = %d\n"
		   "aspect_dy  = %d\n",
		   flic_info.width,
		   flic_info.height,
		   flic_info.x,
		   flic_info.y,
		   flic_info.num_frames,
		   flic_info.depth,
		   flic_info.millisec_per_frame,
		   flic_info.aspect_dx,
		   flic_info.aspect_dy
		   );

	exit(0);
}
