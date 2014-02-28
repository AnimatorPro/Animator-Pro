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
#endif /* __WATCOMC__ */

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include "jfile.ih"
#include "memory.h"
#include "msfile.h"

/********** Glue from ms-dos files to jfile *****************/

static Errcode get_doserror(void)
/*
 * Get latest an error code from MS-DOS
 */
{
	return(pj_mserror(pj_dget_err()));
}

static Errcode ddos_open(Jfl *result, const char *name, int mode)
{
	int flags;

	mode &= MSOPEN_MODES;	/* Mask out any wierd mode requests */

	switch (mode) {
		case JREADONLY:
			flags = O_RDONLY;
			break;
		case JWRITEONLY:
			flags = O_WRONLY;
			break;
		case JREADWRITE:
			flags = O_RDWR;
			break;
		default:
			return Err_nogood;
	}

	result->handle.j = open(name, flags);

	if (result->handle.j == -1) {
		return Err_nogood;
	}
	else {
		return Success;
	}
}

static Errcode ddos_create(Jfl *result, const char *name, int mode)
{
	int flags;

	switch (mode) {
		case JWRITEONLY:
			flags = O_CREAT|O_WRONLY|O_TRUNC;
			break;
		case JREADWRITE:
			flags = O_CREAT|O_RDWR|O_TRUNC;
			break;
		default:
			return Err_nogood;
	}

	result->handle.j = open(name, flags);

	if (result->handle.j == -1) {
		return Err_nogood;
	}
	else {
		return Success;
	}
}

static Errcode ddos_close(Jfl *in)
{
	return close(in->handle.j);
}

static long ddos_write(Jfl *f, void *buf, long count)
{
#if defined(__WATCOMC__)
	/* Not sure why watcom read is acting up... */
	unsigned int size;
	int err = _dos_write(f->handle.j, buf, count, &size);

	if (err != 0) {
		return 0;
	}
	else {
		return size;
	}
#else /* __WATCOMC__ */
	return write(f->handle.j, buf, count);
#endif /* __WATCOMC__ */
}

static long ddos_read(Jfl *f, void *buf, long count)
{
#if defined(__WATCOMC__)
	/* Not sure why watcom read is acting up... */
	unsigned int size;
	int err = _dos_read(f->handle.j, buf, count, &size);

	if (err != 0) {
		return 0;
	}
	else {
		return size;
	}
#else /* __WATCOMC__ */
	return read(f->handle.j, buf, count);
#endif /* __WATCOMC__ */
}

static long ddos_seek(Jfl *f, long offset, int mode)
{
	return lseek(f->handle.j, offset, mode);
}

static long ddos_tell(Jfl *f)
{
	return lseek(f->handle.j, 0, SEEK_CUR);
}

static Errcode ddos_ddelete(const char *name)
{
	if (unlink(name) != 0) {
		/* TODO: improve error code. */
		return Err_nogood;
	}
	else {
		return Success;
	}
}

static Errcode ddos_rename(const char *old, const char *new)
{
	if (rename(old, new) != 0) {
		/* TODO: improve error code. */
		return Err_nogood;
	}
	else {
		return Success;
	}
}

static Tdev msd_dev =
/**
 ** This is the jfile type device for files that reside on MS-DOS
 **/
	{
	ddos_open,
	ddos_create,
	ddos_close,
	ddos_read,
	ddos_write,
	ddos_seek,
	ddos_tell,
	ddos_ddelete,
	NULL,
	get_doserror,
	ddos_rename,
	};


/****** Glue from internal resizable ramdisk to jfile ********/

static Errcode dropen(Jfl *result, const char *name, int mode)
/*
 * Pass open request through to internal ramdisk and store result
 * in handle.
 */
{
if ((result->handle.r = ropen(name, mode)) == NULL)
	return(rerror());
return(Success);
}

static Errcode drcreate(Jfl *result, const char *name, int mode)
/*
 * Pass create request through to internal ramdisk and store result
 * in handle.
 */
{
if ((result->handle.r = rcreate(name, mode)) == NULL)
	return(rerror());
return(Success);
}

static Errcode drclose(Jfl *in)
/*
 * Pass close request to internal ramdisk.
 */
{
return(rclose(in->handle.r));
}

static long drwrite(Jfl *f, void  *buf, long count)
/*
 * Pass write request to internal ramdisk.
 */
{
return(rwrite(f->handle.r,  buf, count));
}

static long drread(Jfl *f, void *buf, long count)
/*
 * Pass read request to internal ramdisk.
 */
{
return(rread(f->handle.r, buf, count));
}

static long drseek(Jfl *f, long offset, int mode)
/*
 * Pass seek request to internal ramdisk.
 */
{
return(rseek(f->handle.r,offset,mode));
}

static long drtell(Jfl *f)
/*
 * Pass tell (whereami) request to internal ramdisk.
 */
{
return(rtell(f->handle.r));
}

static Tdev trd_dev =
/**
 ** This is the jfile type device for files that reside on internal
 ** resizable ramdisk #:.
 **/
	{
	dropen,
	drcreate,
	drclose,
	drread,
	drwrite,
	drseek,
	drtell,
	rdelete,
	NULL,
	rerror,
	rrename,
	};

Boolean is_ram_file(Jfile fd)
/*
 * Determine if a file is on ramdisk device.  (A file may be on 
 * tempfile device presently residing on ramdisk.  This test will
 * fail on such a file.  Only determines if file was opened with #:
 */
{
	return(fd->dev == &trd_dev);
}

/****** Glue from temp file manager (which will swap from device to
  device as necessary using all storage in temp path) to jfile ********/

static Errcode dtopen(Jfl *result, const char *name, int mode)
/*
 * Pass open request to temp-file-device and store result in handle.
 */
{
if ((result->handle.t = topen(name, mode)) == NULL)
	return(terror());
return(Success);
}

static Errcode dtcreate(Jfl *result, const char *name, int mode)
/*
 * Pass create request to temp-file-device and store result in handle.
 */
{
if ((result->handle.t = tcreate(name, mode)) == NULL)
	return(terror());
return(Success);
}

static Errcode dtclose(Jfl *in)
/*
 * Pass close request to temp-file-device.
 */
{
return(tclose(in->handle.t));
}

static long dtwrite(Jfl *f, void  *buf, long count)
/*
 * Pass write request to temp-file-device.
 */
{
return(twrite(f->handle.t,  buf, count));
}

static long dtread(Jfl *f, void *buf, long count)
/*
 * Pass read request to temp-file-device.
 */
{
return(tread(f->handle.t, buf, count));
}

static long dtseek(Jfl *f, long offset, int mode)
/*
 * Pass seek request to temp-file-device.
 */
{
return(tseek(f->handle.t,offset,mode));
}

static long dttell(Jfl *f)
/*
 * Pass tell (whereami) request to temp-file-device.
 */
{
return(ttell(f->handle.t));
}

static Tdev tdev_dev =
/**
 ** This is the jfile type device for files that reside on the
 ** temporary filing system - files which will be move from ram
 ** to various partitions of the hard disk as space is needed.
 **/
	{
	dtopen,
	dtcreate,
	dtclose,
	dtread,
	dtwrite,
	dtseek,
	dttell,
	tdelete,
	NULL,
	terror,
	trename,
	};


/******
 ****** Stuff concerned with the jfile functions accessable outside ****
 ******/


static Errcode jerr;

static Tdev *dev_for_name(const char *name)
/*
 * Given the device name, figure out from it's first character which
 * device it belongs to.
 */
{
Tdev *dev = &msd_dev;	/* By default it's just MS-DOS */

if (name[1] == ':')		/* If an explicit device name included... */
	{
	switch (name[0])	/* Look at the first letter */
		{
		case TRD_CHAR:	/* #: */
			dev = &trd_dev;
			break;
		case TDEV_MED:  /* =: */
		case TDEV_LO:	/* <: */
			dev = &tdev_dev;
			break;
		default:
			break;
		}
	}
return(dev);
}


static Jfl *jnew_file(Tdev *dev)
/*
 * Allocate memory for new Jfl struct and initialize it with magic number,
 * device and zeroes.
 */
{
Jfl *new;

if ((new = trd_askcmem(sizeof(*new))) == NULL)
	{
	jerr = Err_no_memory;
	return(NULL);
	}
new->dev = dev;
new->jfl_magic = JFL_MAGIC;
return(new);
}

static Jfl *pj_open_or_create(Boolean do_create, const char *name, int mode)
/*
 * Open or create a jfile.  (which one depends on do_create passed
 * in.)   Since open are identical except for whether the device
 * specific open or the device specific create is called - we have this
 * function.
 */
{
Tdev *dev;
Errcode err;
Jfl *tf;
Errcode (*o_or_c)(Jfl *tf, const char *name, int mode);

dev = dev_for_name(name);	/* Name says which device to put file on. */
o_or_c = (do_create ? dev->dcreate : dev->dopen);
if ((tf = jnew_file(dev)) == NULL)	/* Allocate structure for file */
	return(NULL);
if ((err = (*o_or_c)(tf,name,mode)) < Success)	/* Do device specific open */
	{
	jerr = err;				/* Store error and */
	trd_freemem(tf);		/* cleanup if there's a problem */
	return(NULL);			/* and return NULL */
	}
tf->rwmode = mode;	/* Store open mode for later use. */
return(tf);
}

Jfl *pj_open(const char *name, int mode)
/*
 * Open a previously existing file.  Mode indicates whether it's
 * read or write or both.  (See filemode.h for definitions of
 * acceptable modes.)
 */
 {
 return(pj_open_or_create(FALSE,name,mode));
 }

Jfl *pj_create(const char *name, int mode)
/*
 * Create a new file.  Can be write or read/write as determined by the
 * mode parameter (see filemode.h for acceptible mode values).  
 */
{
 return(pj_open_or_create(TRUE,name,mode));
}

Errcode pj_close(Jfl *f)
/*
 * Close a file handle 
 */
{
Errcode err;

/* do some error checking first... */
if (f == NULL)
	return(jerr = Err_null_ref);
if (f->jfl_magic != JFL_MAGIC)
	return(jerr = Err_file_not_open);
f->jfl_magic = 0;
if ((err = f->dev->dclose(f)) < Success)
	jerr =  err;
trd_freemem(f);
return(err);
}

long pj_read(Jfl *f, void *buf, long count)
/*
 * Read in a bunch of bytes from a file 
 */
{
long rd;

if ((rd = f->dev->dread(f,buf,count)) < count)
	jerr = f->dev->derror();
return(rd);
}

long pj_write(Jfl *f, void *buf, long count)
/*
 * Write out a bunch of bytes to a file 
 */
{
long rd;

	if ((rd = f->dev->dwrite(f,buf,count)) < count)
	{
		if (rd >= 0)
			jerr = Err_no_space;
		else	/* Negative return means a real error */
			{
			jerr = f->dev->derror();
			rd = 0;
			}
	}
	return(rd);
}


long pj_seek(Jfl *f, long offset, int mode)
/*
 * Move to a new place in a file.  Offset can be from current location
 * beginning or end of file depending on mode.  See filemode.h.
 */
{
long pos;

if ((pos = f->dev->dseek(f,offset,mode)) < Success)
	jerr = pos;
return(pos);
}

long pj_tell(Jfl *f)
/*
 * Return current position in file
 */
{
long pos;

if ((pos = f->dev->dtell(f)) < Success)
	jerr = pos;
return(pos);
}

Errcode pj_delete(const char *name)
/*
 * Delete a file.
 */
{
Tdev *dev;
Errcode err;

dev = dev_for_name(name);	/* Use name to find out which device it's on */
if ((err = dev->ddelete(name)) < Success)
	jerr = err;
return(err);
}

Errcode pj_ioerr(void)
/*
 * Return last i/o error code.
 */
{
return(jerr);
}

int get_jmode(Jfl *t)
/*
 * Return mode file was opened with.
 */
{
	if(t)
		return(t->rwmode);
	return(JUNDEFINED);
}

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

Boolean pj_exists(const char *title)
/* Does file exist? Boolean does not handle errors now */
{
Jfile f;

	if ((f = pj_open(title, 0)) == JNONE)
		return(0);
	pj_close(f);
	return(1);
}

long pj_file_size(const char *title)
/*
 * Return size of a (closed) file.
 */
{
Jfile f;
long size;

if ((f = pj_open(title, 0)) == JNONE)
	return(pj_ioerr());
size = pj_seek(f, 0L, JSEEK_END);
pj_close(f);
return(size);
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

static struct jfl _jstdout = 
	{
	JFL_MAGIC,
	JWRITEONLY,
	&msd_dev,
	{ 0 },
	};
static struct jfl _jstderr = 
	{
	JFL_MAGIC,
	JWRITEONLY,
	&msd_dev,
	{ 2 },
	};

void *get_jstdout(void)
/*
 * Return pointer to standard output.
 */
{
return(&_jstdout);
}

void *get_jstderr(void)
/*
 * Return pointer to standard input.
 */
{
return(&_jstderr);
}
