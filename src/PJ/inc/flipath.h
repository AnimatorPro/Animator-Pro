#ifndef FLIPATH_H
#define FLIPATH_H

#ifndef FLI_H
	#include "fli.h"
#endif

#define FLIPATH_VERSION 0

typedef struct flipath {
	Fat_chunk id; 
	Fli_id fid;			    /* id of file path should point to */
	char path[PATH_SIZE]; 	/* null terminated path string
							 * memory only guaranteed to be to NULL */
} Flipath;

Boolean flipaths_same(Flipath *pa, Flipath *pb);

#endif
