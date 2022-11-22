#ifndef MSFILE_H
#define MSFILE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

typedef int Doserr;

#define Dsuccess 0

Errcode pj_mserror(Doserr derr); /* translator to pj error codes */
Doserr pj_dget_err(void); /* - do MS-DOS extended error query. */

/* drive 0 = A: 1 = B: ... */
extern Errcode current_device(char *devstr);
extern Errcode pj_change_device(const char *device);

extern Boolean pj_dis_drive(int drive); /* True if drive present */
extern int pj_dset_drive(int drive); /* returns # of drives installed */
extern int pj_dget_drive(void);
extern int pj_dcount_floppies(void);

/* 0 = current, 1 = A: for drive */
extern Boolean pj_dget_dir(int drive, char *dir);

extern Boolean _lodos_set_dir(char *path);

long pj_dtell(int f);

#endif /* MSFILE_H */
