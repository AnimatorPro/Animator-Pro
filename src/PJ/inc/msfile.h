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

long pj_dtell(int f);

#endif /* MSFILE_H */
