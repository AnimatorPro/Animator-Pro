
/* jfile.c - Copyright 1989 Jim Kent; Dancing Flame, San Francisco.
   A perpetual non-exclusive license to use this source code in non-
   commercial applications is given to all owners of the Autodesk Animator.
   If you wish to use this code in an application for resale please
   contact Autodesk Inc., Sausilito, CA  USA  phone (415) 332-2244. */

/* jfile.c is mostly little routines to do simple file i/o under MS-DOS.
   Also a few routines to help work with buffers larger than 64K on the 8086
   without resorting to HUGE model. */

#include "jimk.h"

/* return the offset half of a pointer.  Pass in a pointer.  (Ignore
   parameters.  Lint will scream!) */
unsigned
ptr_offset(offset, seg)
int offset, seg;
{
return(offset);
}

/* return the segment half of a pointer.  Pass in a pointer.  (Ignore
   parameters.) */
unsigned
ptr_seg(offset, seg)
int offset, seg;
{
return(seg);
}

/* Fool C compiler into thinking 2 shorts are a long. */
long
make_long(l)
long l;
{
return(l);
}


/* Open a file to read/write/read-write depending on mode.  Returns
   0 on failure, otherwise file handle. */
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

/* close file */
jclose(f)
int f;
{
union regs reg;

reg.b.ah = 0x3e;
reg.w.bx = f;
sysint(0x21,&reg, &reg);
}

/* Close a non-zero file handle */
gentle_close(f)
int f;
{
if (f)
	jclose(f);
}


/* Read file to a buffer.   Returns bytes of data successfully transfered. */
unsigned
jread(f,buf,size)
int f;
void *buf;
unsigned size;
{
union regs reg;
long written;

reg.b.ah = 0x3f;
reg.w.bx = f;
reg.w.cx = size;
reg.w.dx = ptr_offset(buf);
reg.w.ds = ptr_seg(buf);
if ((sysint(0x21,&reg,&reg))&1)	/* check carry */
	{
	written = 0;
	}
else
	{
	written = (unsigned)reg.w.ax;
	}
return(written);
}


/* Seek to a long offset.  Return file position on success, -1 on failure. */
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

