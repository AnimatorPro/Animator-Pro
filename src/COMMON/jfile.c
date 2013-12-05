
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

int
jexists(const char *title)
{
	FILE *f;

	f = jopen(title, 0);
	if (f != NULL) {
		jclose(f);
		return 1;
	}
	else {
		return 0;
	}
}

int
jdelete(const char *title)
{
	return (unlink(title) == 0);
}

int
jrename(const char *oldname, const char *newname)
{
	return (rename(oldname, newname) == 0);
}

FILE *
jcreate(const char *title)
{
	return jopen(title, JWRITEONLY);
}

FILE *
jopen(const char *title, enum JReadWriteMode mode)
{
	const char *str;

	if (mode == JREADWRITE) {
		FILE *f = fopen(title, "rb+");
		if (f != NULL)
			return f;
	}

	str = (mode == JREADONLY) ? "rb"
	    : (mode == JWRITEONLY) ? "wb" : "wb+";

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

long
jread(FILE *f, void *buf, long size)
{
	return fread(buf, 1, size, f);
}

long
jwrite(FILE *f, const void *buf, long size)
{
	return fwrite(buf, 1, size, f);
}

long
jseek(FILE *f, long offset, enum JSeekMode mode)
{
	if (fseek(f, offset, mode) != 0) {
		return -1;
	}
	else {
		return ftell(f);
	}
}

long
jtell(FILE *f)
{
	return ftell(f);
}
