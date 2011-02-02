
/* jfile.c - ask MS-DOS to read,write, and seek without any buffering.
   Arguments are MS-DOS file handle (int), file-name (char *), 
   buffer space pointers (32 bit large model pointers), and
   byte counts/offsets (32 bit long integers).
   */

#include "jimk.h"
#include "commonst.h"
#include "jfile.str"


/* Does file exist? */
jexists(title)
char *title;
{
int f;

if ((f = jopen(title, 0))!=0)
	{
	jclose(f);
	return(1);
	}
return(0);
}

/* Delete a file if possible.  Don't complain if it's not there. */
jdelete(title)
char *title;
{
union regs reg;

reg.b.ah = 0x41;
reg.w.dx = ptr_offset(title);
reg.w.ds = ptr_seg(title);
if (sysint(0x21,&reg,&reg)&1)
	return(0);
return(1);
}


/* Rename a file */
jrename(oldname, newname)
char *oldname, *newname;
{
union regs reg;

reg.b.ah = 0x56;
reg.w.dx = ptr_offset(oldname);
reg.w.ds = ptr_seg(oldname);
reg.w.di = ptr_offset(newname);
reg.w.es = ptr_seg(newname);
if (sysint(0x21,&reg,&reg)&1)
	return(0);
return(1);
}

/* Create a file to read/write */
jcreate(title)
char *title;
{
union regs reg;

reg.b.ah = 0x3c;
reg.w.cx = 0;
reg.w.dx = ptr_offset(title);
reg.w.ds = ptr_seg(title);
if (sysint(0x21,&reg,&reg)&1)	/* check carry */
	return(0);
else
	return(reg.w.ax);
}

/* Open a file that already exists */
jopen(title, mode)
char *title;
int mode;
{
union regs reg;

reg.b.ah = 0x3d;	/* open file */
reg.b.al = mode;		/* read/write etc... */
reg.w.dx = ptr_offset(title);
reg.w.ds = ptr_seg(title);
if (sysint(0x21,&reg,&reg)&1)	/* check carry */
	return(0);
else
	return(reg.w.ax);
}

/* Close file */
jclose(f)
int f;
{
union regs reg;

reg.b.ah = 0x3e;
reg.w.bx = f;
sysint(0x21,&reg, &reg);
}

/* Check for non-zero file handle and then close */
gentle_close(f)
int f;
{
if (f)
	jclose(f);
}


/* Read or write a buffer from file.  Usually you use jread() or jwrite()
   macros rather than calling this one directly */
long 
jreadwrite(f,buf,size,ah)
int f;
void *buf;
long size;
int ah;
{
union regs reg;
long written;
unsigned s1;

written = 0;
while (size > 0)
	{
	reg.b.ah = ah;
	reg.w.bx = f;
	s1 = (size > 0x0c000L ?  0xc000 : size);
	reg.w.cx = s1;
	reg.w.dx = ptr_offset(buf);
	reg.w.ds = ptr_seg(buf);
	if ((sysint(0x21,&reg,&reg))&1)	/* check carry */
		{
		goto OUT;
		}
	else
		{
		written += (unsigned)reg.w.ax;
		size -= (unsigned)reg.w.ax;
		if (s1 != reg.w.ax)
			goto OUT;
		}
	buf = norm_pointer((char *)buf + s1);
	}
OUT:
return(written);
}

/* Move a piece of a file from one place to another.  Used occassionally
   on my indexed temp.flx file */
copy_in_file(file, bytes, soff, doff)
int file;
long bytes, soff, doff;
{
char sbuf[256];	/* static buffer */
char *dbuf;		/* dynamic (large) buffer */
char *buf;
long blocksize;
long size, rsize;
int status;
int backwards;

status = 0;
blocksize = 16*1024;	
blocksize = 1000;	/* DEBUG */
if ((dbuf = laskmem(blocksize)) == NULL)
	{
	blocksize = sizeof(sbuf);
	buf = sbuf;
	}
else
	buf = dbuf;
backwards = (doff > soff);
if (backwards)	/* copy backwards ? */
	{
	soff += bytes;	/* move pointers to end */
	doff += bytes;
	}
for (;;)
	{
	if (bytes <= 0)
		break;
	size = (bytes > blocksize ? blocksize : bytes);
	if (backwards)
		{
		soff -= size;
		doff -= size;
		}
	if (jseek(file, soff, 0) == -1)
		{
		goto EXIT;
		}
	if (jread(file, buf, size) < size)
		{
		goto EXIT;
		}
	if (jseek(file, doff, 0) == -1)
		{
		goto EXIT;
		}
	if (jwrite(file,buf,size) < size)
		{
		goto EXIT;
		}
	bytes -= size;
	if (!backwards)
		{
		soff += size;
		doff += size;
		}
	}
status = 1;
EXIT:
gentle_freemem(dbuf);
return(status);
}

/* Make a copy of a file. */
jcopyfile(source,dest)
char *source,*dest;
{
int in,out;
unsigned size;
char sbuf[256];	/* static buffer */
char *dbuf;		/* dynamic (large) buffer */
char *buf;
long blocksize;
int i;
int success;

blocksize = 32L*1024;	
if ((dbuf = laskmem(blocksize)) == NULL)
	{
	blocksize = sizeof(sbuf);
	buf = sbuf;
	}
else
	buf = dbuf;
out = 0;
success = 0;
if ((in = jopen(source, 0)) == 0)
	goto EXIT;
if ((out = jcreate(dest)) == 0)
	goto EXIT;
for (;;)
	{
	size = jread(in, buf, blocksize);
	if (jwrite(out,buf,size) < size)
		{
		truncated(dest);
		goto EXIT;
		}
	if (size < blocksize)
		{
		success = 1;
		goto EXIT;
		}
	}
EXIT:
gentle_freemem(dbuf);
gentle_close(in);
gentle_close(out);
return(success);
}



/* Reposition file pointer */
long
jseek(f, offset, mode)
int f, mode;
long offset;
{
union regs reg;

reg.b.ah = 0x42;
reg.b.al = mode;
reg.w.bx = f;
reg.w.cx = ptr_seg(offset);
reg.w.dx = ptr_offset(offset);
if (sysint(0x21,&reg,&reg)&1) 
	return(-1);
else
	return(make_long(reg.w.ax, reg.w.dx));
}

long
jtell(f)
int f;
{
return(jseek(f, 0L, 1) );
}


/* Read in a file of known size all at once. */
read_gulp(name, buf, size)
char *name;
void *buf;
long size;
{
int f;

if ((f = jopen(name,0))==0)
	{
	cant_find(name);
	return(0);
	}
if (jread(f, buf, size) < size)
	{
	truncated(name);
	jclose(f);
	return(0);
	}
jclose(f);
return(1);
}

/* Write out a file of known size all at once */
write_gulp(name, buf, size)
char *name;
void *buf;
long size;
{
int f;

if ((f = jcreate(name))==0)
	{
	cant_create(name);
	return(0);
	}
if (jwrite(f, buf, size) < size)
	{
	truncated(name);
	jclose(f);
	jdelete(name);
	return(0);
	}
jclose(f);
return(1);
}

static char net_err[] = "Network error";

static char *dos_err_msg[] =  {
NULL,
NULL,  /* "Internal error - Invalid function", */
"Non-existent or bad file-name.",
"Path not found (wrong directory?)",
"Too many open files.",
"Access denied (file read only?)",
NULL,  /* "Internal error - bad file handle", */
NULL, /* "Memory control blocks destroyed", */
"Not enough memory",
NULL, /* "Invalid memory-block address", */
NULL,		/* "Invalid environment block", */
"Invalid format",
"Invalid file-access code",
"Invalid data",
NULL,		/* reserved */
"Invalid drive",
NULL,		/* attempt to remove current directory */
NULL,		/* not the same device */
"No more files",
"Disk is write protected",
"Unknown disk unit ID",
"Disk drive not ready",
NULL,	/* Unknown disk command */
"Disk data error",
NULL,	/* bad disk request structure length */
"Disk seek error",
"Non-DOS disk",
"Disk sector not found",
NULL, /* Printer out of paper */
"Read error",
"Write error",
"General failure",
"File-sharing violation",
"File-locking violation",
"Invalid disk change",
NULL, /* "No FCB available", */
NULL, /* "Sharing buffer overflow", */
};

static char *more_dos_err_msg[] =  {
net_err,
"Remote computer not listening",
net_err,
net_err,
"Network busy",
net_err,
net_err,
net_err,
net_err,
net_err,
net_err,
NULL, /* "Print queue full", */
NULL, /* "Not enough space for print file", */
NULL, /* "Print file was deleted", */
net_err,
"Access denied",
net_err,
net_err,
net_err,
net_err,
"Sharing temporarily paused",
net_err,
"Print or disk redirection is paused",	/* #72 */
NULL,NULL,NULL,NULL,NULL,NULL,NULL,				/* #73-79 */
"File already exists",
NULL,
"Cannot create directory entry",
NULL,		/* critical error */
net_err,
net_err,
NULL, 	/* "Invalid password", */
NULL, 	/* "Invalid parameter", */
net_err,
};

extern int crit_errval;
get_dos_err_line(char *errbuf)
{
union regs reg;
char *errs;
int err;

reg.b.ah = 0x30;	/* get dos version */
sysint(0x21,&reg,&reg);
if (reg.b.al < 3)		/* extended err info only version 3+ of DOS */
	errbuf[0] = 0;
else
	{
	reg.b.ah = 0x59;
	reg.w.bx = 0;
	sysint(0x21,&reg,&reg);
	err = reg.w.ax;
	if (err == 83)		/* critical error fail */
		{
		err = crit_errval+19;
		errs = dos_err_msg[err];
		}
	else if (err < 0)
		errs = NULL;
	else if (err < Array_els(dos_err_msg))
		errs = dos_err_msg[err];
	else if (err >= 50 &&  err <= 88)
		errs = more_dos_err_msg[err-50];
	else
		errs = NULL;
	if (errs == NULL)
		sprintf(errbuf, "error code %d", err);
	else
		strcpy(errbuf, errs);
	}
}

/* Some standard file error error messages. Generally you'll have to
   replace them with something made up out of printf's or call to your
   own error handlers if  you're trying to reuse this code. */
cant_create(name)
char *name;
{
char *bufs[4];
char ebuf[80];

bufs[0] = jfile_100 /* "Sorry Autodesk Animator can't create:" */,
bufs[1] = name;
get_dos_err_line(ebuf);
bufs[2] = ebuf;
bufs[3] = NULL;
continu_box(bufs);
}

cant_find(name)
char *name;
{
char *bufs[3];

bufs[0] = jfile_102 /* "Sorry Autodesk Animator can't find:" */,
bufs[1] = name;
bufs[2] = NULL;
continu_box(bufs);
}

/* Announce file isn't as big as it's supposed to be.  Checks for NULL
   name argument in case we forgot who we're writing somewhere along
   the line... */
truncated(filename)
char *filename;
{
char *bufs[4];

see_cmap();		/* sometimes will have set color regs... */
bufs[0] = (filename ? filename : cst_);
bufs[1] = jfile_104 /* "File truncated!" */;
bufs[2] = NULL;
continu_box(bufs);
}

/* Make sure use knows they're overwriting an old file and give 'em a
   chance to abort it. */
overwrite_old(name)
char *name;
{
char bud[90];
char *bufs[3];
int len;

if (!jexists(name) )
	return(1);
bufs[0] = jfile_105 /* "Overwrite old" */;
sprintf(bud, "%s?", name);
len = strlen(bud);	/* chop long file name */
if (len > 40)
	bufs[1] = bud+len-40;
else
	bufs[1] = bud;
bufs[2] = NULL;
return(yes_no_box(bufs));
}

/* Indicate miscelanious file damage (or internal software brain damage
   sometimes.) */
mangled(name)
char *name;
{
char *bufs[3];

bufs[0] = jfile_107 /* "File corrupted:" */;
bufs[1] = name;
bufs[2] = NULL;
continu_box(bufs);
}

/* delete file and squawk about errors */
jdelete_rerr(title)
char *title;
{
char ebuf[80];
char *bufs[4];

if (!jdelete(title))
	{
	bufs[0] = "Can't delete file:";
	bufs[1] = title;
	get_dos_err_line(ebuf);
	bufs[2] = ebuf;
	bufs[3] = NULL;
	continu_box(bufs);
	return(0);
	}
return(1);
}


/* Poll input to see if they'd like to call it off... */
check_abort(frame, of)
unsigned frame, of;
{
char *bufs[5];
char buf[40];

check_input();
if (RJSTDN || key_hit)
	{
	bufs[0] = jfile_108 /* "Abort multiple-frame rendering" */;
	sprintf(buf, jfile_109 /* "on frame %u of %u?" */, frame, of);
	bufs[1] = buf;
	bufs[2] = NULL;
	return(yes_no_box(bufs) );
	}
else
	return(0);
}


really_delete(name)
char *name;
{
char *bufs[3];

bufs[0] = name;
bufs[1] = jfile_110 /* "Really delete file permanently?" */;
bufs[2] = NULL;
return(yes_no_box(bufs));
}

