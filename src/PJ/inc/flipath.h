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

extern Errcode set_flipath(char *fliname, Fli_id *flid, Flipath *fp);
extern void clear_flipath(Flipath *fp);
extern void copy_flipath(Flipath *a, Flipath *b);
extern Errcode alloc_flipath(char *fliname, Flifile *flif, Flipath **pfpath);
extern void free_flipath(Flipath **fp);
extern Boolean flipaths_same(Flipath *pa, Flipath *pb);

#endif
