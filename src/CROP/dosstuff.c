/* Dosstuff.c - MS-DOS routines (also see jfile.c) to figure out
   about directories, etc. */

#include <ctype.h>

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

extern int device;
extern char devices[26];
extern int dev_count;

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
   		C:\VPAINT\FISHIES  */
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

char devices[26];
int dev_count;

/* get list of devices we believe to be real (for drive buttons on
   browse menu and other uses) by doing a request for info DOS call
   on each letter of the alphabet.  Since this is a little slow on 
   floppies, we consult the BIOS equipment list for a count of # of
   floppies to fill in the potential A: and B: buttons. */
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
#endif /* OLD */
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
