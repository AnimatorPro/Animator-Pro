/*****************************************************************************
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#define _SIZE_T_DEFINED
#include "pjbasics.h"


void main(int argc, char **argv)
{
	Errcode 		err;

	if (argc < 2) {
		printf("Usage: FLIPLAY filename.fli \n");
		exit(1);
	}

	pj_video_add(NULL);
	pj_video_add_all();

	pj_set_gs();
	err = pj_flic_play(argv[1], NULL); 

	if (Success > err) {
		printf("Error playing flic, status = %d\n", err);
	}

	exit(err);
}
