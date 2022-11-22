
#include "jimk.h"

/* some pointer manipulation routines for the 8086 */
unsigned
ptr_offset(offset, seg)
int offset, seg;
{
return(offset);
}

unsigned
ptr_seg(offset, seg)
int offset, seg;
{
return(seg);
}

long
make_long(l)
long l;
{
return(l);
}


void *
make_ptr(pt)
void *pt;
{
return(pt);
}



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

jclose(f)
int f;
{
union regs reg;

reg.b.ah = 0x3e;
reg.w.bx = f;
sysint(0x21,&reg, &reg);
}

gentle_close(f)
int f;
{
if (f)
	jclose(f);
}


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

