
#include <stdio.h>
#include "jimk.h"
#include "fli.h"

/* PC keyboard codes for abort and next fli keys */
#define ESC 283
#define SPACE 14624

UBYTE sys_cmap[COLORS*3];
struct video_form vf = 
	{
	0,0,XMAX,YMAX,
	XMAX, VGA_SCREEN,
	sys_cmap,
	};

Names *playlist;
char *cbuf;


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

/* change to startup-resolution, print error message, and exit */
bailout(message)
char *message;
{
cleanup();
puts(message);
exit(-1);
}

main(argc,argv)
int argc;
char *argv[];
{
int i;

if (argc < 2)  /* no argument */
	{
	puts("QuickFli - A simple FLI player with source.");
	puts("Copyright 1989 Dancing Flame, San Francisco.");
	puts("");
	puts("Give QuickFli a list of FLI's to play.");
	puts("<Esc> to abort playback.");
	puts("<space> to go to next FLI");
	exit(0);
	}
if (init())
	{
	play(argc - 1, argv+1);
	cleanup();
	}
}


need_jopen(name)
char *name;
{
int f;
char buf[200];

if ((f = jopen(name, 0)) == 0)
	{
	sprintf(buf, "Couldn't find %s\n", name);
	bailout(buf);
	}
return(f);
}

truncated(name)
char *name;
{
char buf[200];

sprintf(buf, "Couldn't read all of %s\n", name);
bailout(buf);
}

need_read(name, file,buf,size)
char *name;
int file;
void *buf;
long size;
{
if (jread(file,buf,size) < size)
	{
	truncated(name);
	}
return(1);
}


play(count,names)
int count;
char *names[];
{
struct fli_head fh;
char *name;
int i;
int ok;
long loops;

if (count == 1)	/* just one file so go to optimized loop mode */
	loops = 1000000;	/* do it a million times at a shot */
else
	loops = 1;
for (;;)
	{
	for (i=0; i<count; i++)
		{
		name = names[i];
		open_fli(name, &fh, &vf, cbuf);
		ok = loop_fli(name, &fh, loops);
		close_fli(&fh);
		if (!ok)
			return;
		}
	}
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

/* Fill lots of memory with zeros. */
zero_lots(pt, size)
register char *pt;
long size;
{
int lsize;

size >>=1;	/* convert to word count */
while (size > 0)
	{
	if (size > 32000)
		lsize = 32000;
	else
		lsize = size;
	stuff_words(0, pt, lsize);
	pt = norm_pointer(pt+lsize);
	pt = norm_pointer(pt+lsize);
	size -= lsize;
	}
}

/* Clear a screen */
clear_form(f)
Video_form *f;
{
zero_lots(f->p, (long)f->bpr*f->h);
}

/* Wait for clock to reach dtime */
long
wait_til(dtime)
long dtime;
{
long time;

for (;;)
	{
	time = get80hz();
	if (time >= dtime)
		break;
	wait_vblank();
	}
return(time);
}

/* Play a fli a couple of times.  Assumes you've already loaded up the first
   frame and set things up with a successful open_fli call. */
loop_fli(name, fli, loops)
char *name;
struct fli_head *fli;
long loops;
{
long time, dtime;
int i;
int count;
int key;

dtime = get80hz() + fli->speed;
while (--loops >= 0)
	{
	if (jseek(fli->file, fli->frame1_off, 0) < 0)
		return(0);
	count = fli->frame_count;
	if (loops == 0)
		count -= 1;		/* last loop dont cycle back to 1st frame */
	for (i=1; i<=count; i++)
		{
		dtime = wait_til(dtime) + fli->speed;
		key = strobe_keys();
		if (key)
			{
			if (key == ESC)	/* escape?? then quit*/
				return(0);
			else if (key == SPACE)	/* space?? go on to next fli */
				return(1);
			}
		if (!gb_read_next_frame(name,fli->file,&vf,cbuf,1))
			return(0);
		}
	dtime = wait_til(dtime) + fli->speed;
	}
return(1);
}

/* Open file, make sure it's got FLI_MAGIC, decompress first frame */
open_fli(name, fli, screen,cbuf)
char *name;
struct fli_head *fli;
struct video_form *screen;
void *cbuf;
{
int file;
char buf[200];

file = need_jopen(name);
need_read(name, file, fli, (long)sizeof(*fli));
if (fli->type != FLIH_MAGIC)
	{
	sprintf(buf,"%s doesn't seem to be a FLI file, sorry\n", name);
	bailout(buf);
	}
fli->file = file;
gb_read_next_frame(name,file,screen,cbuf,1);
fli->frame1_off = jseek(file, 0L, 1);	/* relative seek to nowhere */
}

close_fli(fli)
struct fli_head *fli;
{
gentle_close(fli->file);
fli->file = 0;
}

