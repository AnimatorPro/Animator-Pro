/** jfile.c - Unbuffered file i/o functions.
 ** All PJ and Convert file i/o goes through the routines contained
 ** here.   There are 3 primary types of files - files which reside
 ** on a normal DOS device,  files that reside on our internal
 ** ramdisk (#: device see rfile.c), and files that reside in the temporary
 ** filing system (=: and <: devices see tfile.c).
 **
 ** The primary control structure - Jfile - is defined in filemode.h.  A
 ** instance of this is returned by pj_open() and pj_create(),  and passed in
 ** as the first parameter to the other jfile related functions including
 ** of course pj_read() and pj_write().   Jfile is constructed such that
 ** routines to implement this system see Jfile as a pointer to a struct jfl
 ** while clients see it as a void *.
 **
 ** The struct jfl itself is contained in "jflstruc.ih" in this directory.
 ** It consists of a pointer to a "device" (which will be either
 ** msdos, ram-disk, or temp file),  and a union for a "handle"
 ** which contains data for that particular device.
 ** The device is primarily a jumptable of functions for open, close, etc.
 ** 
 ** So at this level our open/close/read/write/seek etc. functions
 ** do little but jump through to the corresponding functions in the
 ** attatched device jumptable.   It serves as a way for dealing with
 ** files resident on disparate devices in a uniform fashion.
 **
 ** jfile.c begins with 3 families of glue routines to connect each of
 ** the three types of devices to a jfile device,  and
 ** a corresponding instance of a jfile device.
 **
 **/

#if defined(__WATCOMC__)
#include <dos.h>
#include <io.h>
#else /* __WATCOMC__ */
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif /* __WATCOMC__ */

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include "errcodes.h"
#include "jfile.h"
#include "memory.h"
#include "msfile.h"

Errcode pj_delete(const char *name)
/*
 * Delete a file.
 */
{
#if 0
Tdev *dev;
Errcode err;

dev = dev_for_name(name);	/* Use name to find out which device it's on */
if ((err = dev->ddelete(name)) < Success)
	jerr = err;
return(err);
#else
	int ret;

	ret = unlink(name);
	return (ret == 0) ? Success : Err_stdio;
#endif
}

#if 0
Errcode pj_rename(const char *old, const char *new)
/*
 * Rename a file.  Generally will only work if both old and new name are
 * on the same device.
 */
{
Errcode  err;
Tdev *dev;

dev = dev_for_name(old);
if ((err = dev->drename(old,new)) < Success)
	jerr = err;
return(err);
}
#else
Errcode
pj_rename(const char *old, const char *new)
{
	int ret = rename(old, new);

	return (ret == 0) ? Success : Err_stdio;
}
#endif

Boolean pj_exists(const char *title)
/* Does file exist? Boolean does not handle errors now */
{
	Errcode err;
	XFILE *xf;

	err = xffopen(title, &xf, XREADONLY);
	if (err < Success)
		return FALSE;

	xffclose(&xf);
	return TRUE;
}

long pj_file_size(const char *title)
/*
 * Return size of a (closed) file.
 */
{
	long size;
	Errcode err;
	XFILE *xf;

	err = xffopen(title, &xf, XREADONLY);
	if (err < Success)
		return err;

	size = xffseek_tell(xf, 0, XSEEK_END);
	xffclose(&xf);
	return size;
}

Errcode pj_is_fixed(const char *device)
/* returns 1 if device is fixed 0 if not < 0 if error */
{
char dc;

	dc = toupper(*device);
	if(dc == 'A' || dc == 'B')
		return(0);
	return(1);
}

Boolean is_directory(const char *path)
{
#if defined(__WATCOMC__)
	unsigned attributes;

	if (_dos_getfileattr(path, &attributes) == Success)
		return ((attributes & _A_SUBDIR) != 0);
	else
		return FALSE;
#else
	struct stat s;

	if (stat(path, &s) == 0)
		return S_ISDIR(s.st_mode);
	else
		return FALSE;
#endif
}
