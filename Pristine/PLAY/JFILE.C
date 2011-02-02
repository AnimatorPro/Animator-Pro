
#include "jimk.h"
#include "jfile.str"


#ifdef SLUFFED
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
#endif SLUFFED

#ifdef SLUFFED
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
#endif SLUFFED

#ifdef SLUFFED
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
#endif SLUFFED

#ifdef SLUFFED
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
#endif SLUFFED

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

#ifdef SLUFFED
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
if ((dbuf = askmem((unsigned)blocksize)) == NULL)
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
		continu_line("copy_in_file() truncated 1");
		goto EXIT;
		}
	if (jread(file, buf, size) < size)
		{
		continu_line("copy_in_file() truncated 2");
		goto EXIT;
		}
	if (jseek(file, doff, 0) == -1)
		{
		continu_line("copy_in_file() truncated 3");
		goto EXIT;
		}
	if (jwrite(file,buf,size) < size)
		{
		continu_line("copy_in_file() truncated 4");
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
#endif /* SLUFFED */



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


#ifdef SLUFFED
read_gulp(name, buf, size)
char *name;
char *buf;
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
#endif SLUFFED

#ifdef SLUFFED
write_gulp(name, buf, size)
char *name;
char *buf;
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
#endif SLUFFED

#ifdef SLUFFED
cant_create(name)
char *name;
{
char *bufs[3];

bufs[0] = "Sorry Animator can't create:",
bufs[1] = name;
bufs[2] = NULL;
continu_box(bufs);
}
#endif SLUFFED

cant_find(name)
char *name;
{
char *bufs[3];

bufs[0] = jfile_100 /* "Sorry AAPlay can't find:" */,
bufs[1] = name;
bufs[2] = NULL;
continu_box(bufs);
}

truncated(filename)
char *filename;
{
char *bufs[3];

bufs[0] = (filename ? filename : "");
bufs[1] = jfile_102 /* "File truncated!" */;
bufs[2] = NULL;
continu_box(bufs);
}

#ifdef SLUFFED
overwrite_old(name)
char *name;
{
char bud[90];
char *bufs[3];
int len;

if (!jexists(name) )
	return(1);
bufs[0] = "Overwrite old";
sprintf(bud, "%s?", name);
len = strlen(bud);	/* chop long file name */
if (len > 40)
	bufs[1] = bud+len-40;
else
	bufs[1] = bud;
bufs[2] = NULL;
return(yes_no_box(bufs));
}
#endif SLUFFED

mangled(name)
char *name;
{
char *bufs[2];

bufs[0] = jfile_103 /* "File mangled:" */;
bufs[1] = name;
continu_line(bufs);
}

#ifdef SLUFFED
check_abort(frame, of)
unsigned frame, of;
{
char *bufs[5];
char buf[40];

check_input();
if (key_hit || RJSTDN)
	{
	bufs[0] = "Abort multiple-frame rendering";
	sprintf(buf, "on frame %u of %u?", frame, of);
	bufs[1] = buf;
	bufs[2] = NULL;
	return(yes_no_box(bufs) );
	}
else
	return(0);
}
#endif SLUFFED
