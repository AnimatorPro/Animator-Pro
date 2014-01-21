#ifndef PROGIDS_H
#define PROGIDS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

extern USHORT program_id; 	   /* declared in main of each program */
extern USHORT program_version; /* declared in scodes.c in pjhigh */

LONG get_userid();
void get_relvers(char *buf);   /* gets compound release version number */
void get_userid_string(char *buf);

#endif
