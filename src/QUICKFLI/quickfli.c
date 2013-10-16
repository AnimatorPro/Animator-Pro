
/* quickfli.c - Copyright 1989 Jim Kent; Dancing Flame, San Francisco.
   A perpetual non-exclusive license to use this source code in non-
   commercial applications is given to all owners of the Autodesk Animator.
   If you wish to use this code in an application for resale please
   contact Autodesk Inc., Sausilito, CA  USA  phone (415) 332-2244. */

/* This file contains most of the logic for the quickfli.exe program.
   The routines that actually decompress the flic onto the screen and
   color registers are in the assembler files unbrun.asm and comp.asm.
   The stuff that reads the flic from disk is in fli.c and jfiles.c.

   To play a Flic we have to get into VGA 320x200 256 color mode and
   crank up the 18Hz system clock to about 70Hz, the monitor frame rate.
   This involves a BIOS int #11 call for the VGA mode, and installing
   an interrupt routine for the clock.  Most of the details of initialization
   are in int.asm and peekpoke.asm.

   Next we have to find the flic file and make sure it's valid.  If all
   is well can go to the main playback loop.  Each frame we read the next
   frame into memory, wait until vertical blank to reduce screen tear and
   color register snow, and then decompress.  The last frame of a flic gives
   us the transition back to the first frame.  (So an n frame flic has
   n+1 frames of data.  The first frame is read only the 1st time the flic
   plays.)

   */



#include <stdio.h>
#include "jimk.h"
#include "fli.h"

/* These are the scancodes for the escape key and spacebar. */
#define ESC 283
#define SPACE 14624

/* sys_cmap - a software copy of the hardware color map.  It is formatted
   as R G B  R G B ... with each color component ranging from 0 to 63. */
UBYTE sys_cmap[COLORS*3];
/* This is a structure to describe a vga-format screen.  That is a
   64000 byte 320x200 array of pixels with a color map in the format above. */
struct video_form vf = 
	{
	0,0,XMAX,YMAX,
	XMAX, VGA_SCREEN,
	sys_cmap,
	};

/* Points to a big buffer to hold 1 frame of a flic. */
char *cbuf;

int flifile; /* Current FLI file */
long frame1off;		/* Byte offset to frame 1 (not frame 0) in file */

/* Get memory.  Complain if it's not there. */
void *
begmem(size)
unsigned size;
{
void *p;

if ((p = malloc(size)) == NULL)
	{
	bailout("Out of Memory");
	}
return(p);
}

/* Print out an error message and terminate program. */
bailout(message)
char *message;
{
cleanup();
puts(message);
exit(-1);
}

/* Open a file to read.  Complain if it's not there. */
want_jopen(name)
char *name;
{
int f;

if ((f = jopen(name, 0)) == 0)
	printf("Couldn't find %s\n", name);
return(f);
}

/* Squawk about file not all being there. */
truncated(name)
char *name;
{
char buf[200];

sprintf(buf, "Couldn't read all of %s\n", name);
bailout(buf);
}

/* Read in a block of a certain size.  Complain and return 0 if it's not
   all there. */
need_read(name, file,buf,size)
char *name;
int file;
void *buf;
unsigned size;
{
if (jread(file,buf,size) < size)
	{
	truncated(name);
	return(0);
	}
return(1);
}


static char int_in;	/* keep track if interrupt is installed */
int ivmode;	/* keep track of startup video mode */

/* Tell the VGA what resolution to be in */
set_vmode(mode)
int mode;
{
union regs r;

r.b.ah = 0;
r.b.al = mode;
sysint(0x10,&r,&r);
}

/* get current video mode */
get_vmode()
{
union regs r;

r.b.ah = 0xf;
sysint(0x10, &r, &r);
return(r.b.al);
}

/* return 0 if no key, key scan code if there is a key */
strobe_keys()
{
union regs r;
#define ZEROFLAG	64

r.b.ah = 0x1;
if (!(sysint(0x16,&r,&r)&ZEROFLAG))
	{
	r.b.ah = 0;
	sysint(0x16,&r,&r);
	return(r.w.ax);
	}
else
	return(0);
}

/* Switch into 256 color 320x200 mode, co-opt the  18.2 hz timer to force it
   to something close to video rate, and get buffer big enough to hold a frame.
   */
init()
{
ivmode = get_vmode();
set_vmode(0x13);
if (get_vmode() != 0x13)
	{
	puts("Couldn't get a 320x200 256 color VGA screen");
	return(0);
	}
cbuf = begmem((unsigned)CBUF_SIZE);
setint();
fastint();
int_in = 1;
return(1);
}

/* Go back to old video mode and take out our clock interrupt handler. */
cleanup()
{
set_vmode(ivmode);
if (int_in)
	{
	Restoreint();
	}
}


/* Clear a screen */
clear_form(f)
Video_form *f;
{
stuff_words(0, f->p, (f->h>>1)*f->bpr);
}


/* Open file, make sure it's got FLI_MAGIC, decompress first frame */
open_fli(name, fli, screen,cbuf)
char *name;
struct fli_head *fli;
struct video_form *screen;
void *cbuf;
{
int file;

if ((file = want_jopen(name)) == 0)
	return(0);
if (!need_read(name, file, fli, (unsigned)sizeof(*fli)))
	{
	jclose(file);
	goto BADCLOSEOUT;
	}
if (fli->type != FLIH_MAGIC)
	{
	printf("%s doesn't seem to be a FLI file, sorry\n", name);
	goto BADCLOSEOUT;
	}
flifile = file;
if (!gb_read_next_frame(name,file,screen,cbuf,1))
	goto BADCLOSEOUT;
frame1off = jseek(file, 0L, 1);	/* relative seek to nowhere */
return(1);
BADCLOSEOUT:
jclose(file);
return(0);
}

close_fli(fli)
struct fli_head *fli;
{
gentle_close(flifile);
flifile = 0;
}


/* Do nothing until time has come. */
long
wait_til(time)
long time;
{
long t;
for (;;)
	{
	t = get80hz();
	if (t >= time)
		break;
	}
return(t);
}

/* Play a fli a couple of times.  Assumes you've already loaded up the first
   frame and set things up with a successful open_fli call. */
loop_fli(name, fli, loops)
char *name;
struct fli_head *fli;
long loops;
{
long dtime;
int i;
int count;
int key;

dtime = get80hz() + fli->speed;
do
	{
	if (jseek(flifile, frame1off, 0) < 0)
		return(0);
	count = fli->frame_count;
	if (loops == 0)
		count -= 1;		/* last loop dont cycle back to 1st frame */
	for (i=1; i<=count; i++)
		{
		dtime = wait_til(dtime) + fli->speed;
		if (!gb_read_next_frame(name,flifile,&vf,cbuf,1))
			return(0);
		key = strobe_keys();
		if (key)
			{
			if (key == ESC)	/* escape?? then quit*/
				return(0);
			else if (key == SPACE)	/* space?? go on to next fli */
				return(1);
			}
		}
	}
while (loops);
return(1);
}


/* Play each file in list one after the other.  When through do it
   again.  If there's a problem with a FLI bail out. */
play(count, list)
int count;
char *list[];
{
struct fli_head fh;
char *name;
char **npt;
int i;
int ok;
long loops;

if (count == 1)	/* just one file so go to optimized loop mode */
	loops = 1;
else
	loops = 0;
ok = 1;
while (ok)
	{
	npt = list;
	for (i=0; i<count; i++)
		{
		name = *npt++;
		if (open_fli(name, &fh, &vf, cbuf))
			{
			ok = loop_fli(name, &fh, loops);
			close_fli(&fh);
			if (!ok)
				{
				goto OUT;
				}
			}
		else 
			{
			goto OUT;
			}
		}
	}
OUT:
return;
}


main(argc,argv)
int argc;
char *argv[];
{
int i;

if (argc < 2)  /* no argument, just say what we do and split. */
	{
	puts("QuickFli - A simple player for 256 color VGA animations");
	puts("generated by the Autodesk Animator.");
	puts("Copyright 1989 Dancing Flame, San Francisco.");
	puts("");
	puts("Give QuickFli a list of FLI's to play.");
	puts("<Esc> to abort playback.");
	puts("<space> to go to next FLI");
	exit(0);
	}
if (init())
	{
	play(argc-1, argv+1);
	cleanup();
	}
}

