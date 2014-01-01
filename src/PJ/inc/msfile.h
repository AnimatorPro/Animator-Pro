#ifndef MSFILE_H
#define MSFILE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef FILEMODE_H
	#include "filemode.h"
#endif

typedef int Doserr;

#define Dsuccess 0

Errcode pj_mserror(Doserr derr); /* translator to pj error codes */
Doserr pj_dget_err(); /* - do MS-DOS extended error query. */

Doserr pj_dcreate(int *phandle, char *path, int fmode);
Doserr pj_dopen(int *phandle, char *path, int fmode);
Errcode pj_dclose(int f);
Errcode pj_ddelete(char *name);
long pj_dread(int f, void *buf, long count);
long pj_dwrite(int f, void *buf, long count);
long pj_dseek(int f, long offset, int mode);
long pj_dtell(int f);
Doserr pj_dtruncate(int f); /* truncate file at current position */
Doserr pj_drename(char *oldname, char *newname);

#endif /* MSFILE_H */
