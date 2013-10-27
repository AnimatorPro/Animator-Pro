/* Dosstuff.c - MS-DOS routines (also see jfile.c) to figure out
   about directories, how much disk space is left, etc. */

#include "jimk.h"
#include "dosstuff.h"
#include <ctype.h>

extern WORD device;
extern char devices[26];
extern int dev_count;
extern char drawer[];

char devices[26];
int dev_count;

/*** start new */
#ifdef EVER
struct byte_regs 
	{
	unsigned char al, ah, bl, bh, cl, ch, dl, dh;
	unsigned int si, di, ds, es;
	};

struct word_regs
	{
	unsigned ax,bx,cx,dx;
	unsigned int si, di, ds, es;
	};

union regs
	{
	struct byte_regs b;
	struct word_regs w;
	};
#endif EVER

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

/* fool C into thinking a pointer is a long */
long
make_long(l)
long l;
{
return(l);
}


/* fool C into thinking a long is a pointer */
void *
make_ptr(pt)
void *pt;
{
return(pt);
}


#ifdef OLD
change_dev(newdev)
int newdev;
{
union regs r;

r.b.ah = 0xE;	/* select disk function */
r.b.dl = newdev;
return(!(sysint(0x21,&r,&r)&1) );
}


/* Hey dos - I want to go to this directory.  Actually this changes
   both device and directory at once.  eg name could be
   		C:\VPAINT\FISHIES  B:B:B: */
change_dir(name)
char *name;
{
union regs r;
int d;

if (!name[0])
	return(1);
if (name[1] == ':')	/* got a device... */
	{
	d = name[0];
	if (islower(d))
		d = _toupper(d);
	d -= 'A';
	if (!valid_device(d))
		return(0);
	if (!change_dev(d))
		return(0);
	name += 2;
	}
if (!name[0])
	return(1);
r.b.ah = 0x3B;	/* change current directory function */
r.w.dx = ptr_offset(name);
r.w.ds = ptr_seg(name);
if (sysint(0x21,&r,&r)&1)	/* check carry flag for error... */
	{
	change_dev(device);
	return(0);
	}
device = d;
return(1);
}



get_device()
{
union regs reg;

reg.b.ah = 0x19;	/* get device function */
if (sysint(0x21,&reg,&reg)&1)	/* check carry flag for error... */
	return(-1);
return(reg.b.al);
}


/* get list of devices we believe to be real (for drive buttons on
   browse menu and other uses) by doing a request for info DOS call
   on each letter of the alphabet.  Since this is a little slow on 
   floppies, we consult the BIOS equipment list for a count of # of
   floppies to fill in the potential A: and B: buttons. 
   B:B:B: */
get_devices()
{
int i, floppies;
int od;
union regs r;

dev_count = 0;
/* do dos equipment list function.  Use this to check for floppies, since
   can CD to B: even if it's not there.... */
sysint(0x11, &r, &r);
if (r.w.ax&1)
	{
	floppies = ((r.w.ax>>6)&3)+1;
	for (i=0; i<floppies; i++)
		devices[dev_count++] = i;
	}
#ifdef OLD
od = get_device();
for (i=2; i<26; i++)
	{
	change_dev(i);
	if (get_device() == i)
		{
		devices[dev_count++] = i;
		}
	}
change_dev(od);
#endif OLD
for (i=3; i<=26; i++)
	{
	r.b.ah = 0x1c;
	r.b.dl = i;
	sysint(0x21, &r, &r);
	if (r.b.al != 0xff)
		{
		devices[dev_count++] = i-1;
		}
	}
}

/* Is device really there?  Check device list to see. B:B:B: */
valid_device(d)
int d;
{
int i;

for (i=0; i<dev_count; i++)
	{
	if (devices[i] == d)
		return(1);
	}
return(0);
}



mcurrent_drawer()
{
union regs reg;
char buf[65];

if ((device = get_device()) < 0)
	return(0);
reg.b.ah = 0x47;
reg.b.dl = 0;	/* default device ... */
reg.b.si = ptr_offset(buf);
reg.b.ds = ptr_seg(buf);
if (sysint(0x21, &reg, &reg)&1)	/* check carry for error */
	return(0);
sprintf(drawer,"%c:\\%s", device+'A', buf);
return(1);
}

extern char init_drawer[];

make_current_drawer()
{
if (!mcurrent_drawer())
	{
	change_dir(init_drawer);
	return(mcurrent_drawer());
	}
return(1);
}
#endif OLD
/*** end new */


/* Tell dos it's time to go to another drive mon.  1 = A:, 2 = B: ...
   you get the idea */
change_dev(newdev)
int newdev;
{
union regs r;

r.b.ah = 0xE;	/* select disk function */
r.b.dl = newdev;
return(!(sysint(0x21,&r,&r)&1) );
}


/* Hey dos - I want to go to this directory.  Actually this changes
   both device and directory at once.  eg name could be
   		C:\VPAINT\FISHIES  B:B:B: */
change_dir(name)
char *name;
{
union regs r;
int d;

if (!name[0])
	return(1);
if (name[1] == ':')	/* got a device... */
	{
	d = name[0];
	if (islower(d))
		d = _toupper(d);
	d -= 'A';
	if (!valid_device(d))
		return(0);
	if (!change_dev(d))
		return(0);
	name += 2;
	}
if (!name[0])
	return(1);
r.b.ah = 0x3B;	/* change current directory function */
r.w.dx = ptr_offset(name);
r.w.ds = ptr_seg(name);
if (sysint(0x21,&r,&r)&1)	/* check carry flag for error... */
	{
	change_dev(device);
	return(0);
	}
device = d;
return(1);
}



/* Hey DOS, what drive is we on? */
get_device()
{
union regs reg;

reg.b.ah = 0x19;	/* get device function */
if (sysint(0x21,&reg,&reg)&1)	/* check carry flag for error... */
	return(-1);
return(reg.b.al);
}

/* get list of devices we believe to be real (for drive buttons on
   browse menu and other uses) by doing a request for info DOS call
   on each letter of the alphabet.  Since this is a little slow on 
   floppies, we consult the BIOS equipment list for a count of # of
   floppies to fill in the potential A: and B: buttons. 
   B:B:B: */
get_devices()
{
int i, floppies;
int od;
union regs r;

dev_count = 0;
/* do dos equipment list function.  Use this to check for floppies, since
   can CD to B: even if it's not there.... */
sysint(0x11, &r, &r);
if (r.w.ax&1)
	{
	floppies = ((r.w.ax>>6)&3)+1;
	for (i=0; i<floppies; i++)
		devices[dev_count++] = i;
	}
#ifdef OLD
od = get_device();
for (i=2; i<26; i++)
	{
	change_dev(i);
	if (get_device() == i)
		{
		devices[dev_count++] = i;
		}
	}
change_dev(od);
#endif OLD
for (i=3; i<=26; i++)
	{
	r.b.ah = 0x1c;
	r.b.dl = i;
	sysint(0x21, &r, &r);
	if (r.b.al != 0xff)
		{
		devices[dev_count++] = i-1;
		}
	}
}

/* Is device really there?  Check device list to see. B:B:B: */
valid_device(d)
int d;
{
int i;

for (i=0; i<dev_count; i++)
	{
	if (devices[i] == d)
		return(1);
	}
return(0);
}


/* Figure out what drive we're in and what directory.  Save
   result in 'device' and 'vs.drawer' */
mcurrent_drawer()
{
union regs reg;
char buf[65];

if ((device = get_device()) < 0)
	return(0);
reg.b.ah = 0x47;
reg.b.dl = 0;	/* default device ... */
reg.b.si = ptr_offset(buf);
reg.b.ds = ptr_seg(buf);
if (sysint(0x21, &reg, &reg)&1)	/* check carry for error */
	return(0);
sprintf(drawer,"%c:\\%s", device+'A', buf);
return(1);
}

extern char init_drawer[];

/* Do a little error handling if current directory looks bad.  Change
   back to start-up directory.  Otherwise just set device and vs.drawer
   variables to reflect where MS-DOS thinks we are in the filing system */
make_current_drawer()
{
if (!mcurrent_drawer())
	{
	change_dir(init_drawer);
	return(mcurrent_drawer());
	}
return(1);
}

/* Create a directory */
make_dir(name)
char *name;
{
union regs r;

r.b.ah = 0x39;	/* make dir code */
r.w.ds = ptr_seg(name);
r.w.dx = ptr_offset(name);
if (sysint(0x21,&r,&r)&1)	/* check carry flag for error... */
	return(0);
return(1);
}


/* Figure how much free space is on a device */
long
dfree(d)
int d;
{
union regs r;

r.b.ah = 0x36;
r.b.dl = d;
sysint(0x21,&r,&r);
if (r.w.ax == 0xffff)
	return(0L);
else
	{
	return((long)r.w.cx*(long)r.w.ax*(long)r.w.bx);
	}
}

