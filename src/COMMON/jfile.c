
/* jfile.c - Copyright 1989 Jim Kent; Dancing Flame, San Francisco.
   A perpetual non-exclusive license to use this source code in non-
   commercial applications is given to all owners of the Autodesk Animator.
   If you wish to use this code in an application for resale please
   contact Autodesk Inc., Sausilito, CA  USA  phone (415) 332-2244. */

/* jfile.c is mostly little routines to do simple file i/o under MS-DOS.
   Also a few routines to help work with buffers larger than 64K on the 8086
   without resorting to HUGE model. */

#include <assert.h>
#include "jimk.h"
#include "jfile.h"

FILE *
jopen(const char *title, int mode)
{
	const char *str;
	assert(0 <= mode && mode <= 2);

	str = (mode == 0) ? "rb"
	    : (mode == 1) ? "wb" : "rb+";

	return fopen(title, str);
}

void
jclose(FILE *f)
{
	fclose(f);
}

void
gentle_close(FILE *f)
{
	if (f)
		jclose(f);
}

unsigned int
jread(FILE *f, void *buf, unsigned int size)
{
	return fread(buf, 1, size, f);
}

long
jseek(FILE *f, long offset, int mode)
{
	assert(0 <= mode && mode <= 2);

	mode = (mode == 0) ? SEEK_SET
	     : (mode == 1) ? SEEK_CUR : SEEK_END;

	if (fseek(f, offset, mode) != 0) {
		return -1;
	}
	else {
		return ftell(f);
	}
}
