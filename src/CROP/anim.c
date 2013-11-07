
/* anim.c - read in frames from an Amiga ANIM file such as may be produced
   by Deluxe Paint III, Aegis Video Titler, and many other pieces of Amiga
   software. */

#include <fcntl.h>
#include <stdio.h>
#include "jimk.h"
#include "jiff.h"
#include "anim.h"
#include "anim.str"

#define ANIM_BPR 40
#define YMAX 200
#define WIDTH 320
#define HEIGHT 200
#define ANIM_DEPTH 5
#define PLANE_SIZE (ANIM_BPR*HEIGHT)
#define ANIM_SSZ ((long)ANIM_DEPTH*(long)PLANE_SIZE)
#define MAX_SCREENS (4000)

extern UBYTE sys_cmap[];


static load_fd;
static char *anim_name;
static PLANEPTR screens[2], cur_screen;
static int anim_err = -1;

static struct form_chunk animf_h;
static AnimationHeader ah;
static struct BitMapHeader bh;
static frame_count;
static int anim_frame;
static int anim_speed;
static int got_bh;





static 
open_verify_anim(name)
char *name;
{
if ((load_fd = jopen(name, 0)) <= 0)
	{
	cant_find(name);
	return(0);
	}
if (jread(load_fd, &animf_h, sizeof(animf_h)) < sizeof(animf_h) )
	{
	truncated(name);
	goto BADEXIT;
	}
if (strncmp(&animf_h.fc_type, "FORM", 4))
	{
	if (strncmp(&animf_h.fc_type, "RIFF", 4))
		{
		continu_line(anim_102 /* "Oops, not Amiga ANIM type." */);
		}
	else
		{
		continu_line(anim_103 /* "Oops, looks like an Amiga RIF." */);
		}
	goto BADEXIT;
	}
if (strncmp(&animf_h.fc_subtype, "ANIM", 4))
	{
	if (strncmp(&animf_h.fc_subtype, "ILBM", 4))
		{
		continu_line(anim_106 /* "File's IFF, but not ANIM." */);
		}
	else
		{
		continu_line(anim_107 /* "Oops, looks like an Amiga PIC." */);
		}
	goto BADEXIT;
	}
long_intel_swap(&animf_h.fc_length);
return(1);
BADEXIT:
jclose(load_fd);
load_fd = 0;
return(0);
}



extern unsigned WORD ytable[YMAX];

static 
make_ytable()
{
register unsigned WORD *pt;
register unsigned WORD acc, bpr;
register WORD i;

acc = 0;
pt = ytable;
bpr = ANIM_BPR;
i = YMAX;
while (--i >= 0)
	{
	*pt++ = acc;
	acc += bpr;
	}
}



/* fake amiga... */
static
put_ea_cmap(cmap,count)
UBYTE *cmap;
int count;
{
int i;
UBYTE *ct;

if (count > 256)
	count = 256;
ct = vf.cmap;
for (i=0; i<3*count; i++)
	{
	*ct++ = *cmap++ >> 2;
	}
wait_sync();
jset_colors(0, count, sys_cmap);
}


/* close file, free buffers */
close_anim()
{
if (load_fd != 0)
	{
	jclose(load_fd);
	load_fd = 0;
	}
gentle_freemem(screens[0]);
screens[0] = NULL;
gentle_freemem(screens[1]);
screens[1] = NULL;
anim_err = -1;
}

static
verify_frame(fc)
struct form_chunk *fc;
{
if (jread(load_fd, fc, sizeof(*fc)) != sizeof(*fc) )
	{
	truncated(anim_name);
	return(0);
	}
if (strncmp(&fc->fc_type, "FORM", 4))
	{
	continu_line(anim_109 /* "Not FORM" */);
	return(0);
	}
if (strncmp(&fc->fc_subtype, "ILBM", 4))
	{
	continu_line(anim_111 /* "Not ILBM" */);
	return(0);
	}
long_intel_swap(&fc->fc_length);
if (fc->fc_length < 0 || fc->fc_length > animf_h.fc_length)
	{
	continu_line(anim_112 /* "Bad SIZE" */);
	return(0);
	}
return(1);
}


/* count up frames in anim and incidentally set the speed. */
static
make_frame_count()
{
static struct form_chunk ff;
long fpos;

anim_frame = 0;
frame_count = 0;
fpos = 0;
while (fpos < animf_h.fc_length)
	{
	if (!verify_frame(&ff))
		{
		break;
		}
	fpos += sizeof(ff) + ff.fc_length;
	jseek(load_fd, ff.fc_length-4, SEEK_REL);
	frame_count += 1;
	anim_frame += 1;
	}
}


open_anim(name)
char *name;
{
/* paranoid error checking for internal errors */
if (anim_err != -1)
	{
	continu_line(anim_113 /* "ANIM file already open" */);
	return(0);
	}
/* if it's an anim file go allocate 2 amiga sized screens */
anim_name = name;
if (open_verify_anim(name))
	{
	if ((screens[0] = lbegmem(ANIM_SSZ)) == NULL)
		{
		close_anim();
		return(0);
		}
	if ((screens[1] = lbegmem(ANIM_SSZ)) == NULL)
		{
		close_anim();
		return(0);
		}
	zero_lots(screens[0], ANIM_SSZ);
	zero_lots(screens[1], ANIM_SSZ);
	}
else
	return(0);
anim_err = 0;
make_ytable();
make_frame_count();
got_bh = 0;
return(1);
}

UBYTE *
put_pline(cbuf, p)
UBYTE *cbuf;
PLANEPTR p;
{
int size;
char op, d;
int count;

size = ANIM_BPR;
while (size > 0)
	{
	op = *cbuf++;
	if (op < 0)	/* it's a run */
		{
		d = *cbuf++;
		count = -op+1;
		size -= count;
		while (--count >= 0)
			*p++ = d;
		}
	else
		{
		count = op+1;
		size -= count;
		while (--count >= 0)
			*p++ = *cbuf++;
		}
	}
return(cbuf);
}

UBYTE *
put_line(cbuf, line)
UBYTE *cbuf;
PLANEPTR line;
{
int d;

d = bh.nPlanes;
while (--d >= 0)
	{
	cbuf = put_pline(cbuf, line);
	line += PLANE_SIZE;
	}
return(cbuf);
}

put_body(cbuf, screen)
UBYTE *cbuf;
PLANEPTR screen;
{
int y;

y = YMAX;
while (--y >= 0)
	{
	cbuf = put_line(cbuf, screen);
	screen += ANIM_BPR;
	}
}

put_dlta(cbuf, screen)
UBYTE *cbuf;
PLANEPTR screen;
{
long *offsets;
long off;
int p;

offsets = (long *)cbuf;
p = bh.nPlanes;
while (--p >= 0)
	{
	long_intel_swap(offsets);
	off = *offsets++;
	if (off)
		{
		decode_vkplane(cbuf+off, screen, 
			ANIM_BPR);
		}
	screen += PLANE_SIZE;
	}
}


next_chunk(ic, buf)
struct iff_chunk *ic;
UBYTE *buf;
{
if (jread(load_fd, buf, ic->iff_length) != ic->iff_length)
	{
	truncated(anim_name);
	return(0);
	}
if (strncmp(&ic->iff_type, "BMHD", 4) == 0)
	{
	copy_structure(buf, &bh, sizeof(bh));
	intel_swap(&bh.w);
	intel_swap(&bh.h);
	if (bh.w != 320 || bh.h != 200 || bh.nPlanes > ANIM_DEPTH)
		{
		char bb[80];
		char *bbs[3];

		sprintf(bb, anim_115 /* "File is %dx%dx%d." */, bh.w, bh.h, bh.nPlanes);
		bbs[0] = bb;
		bbs[1] = anim_116 /* "Can only do 320x200 non-HAM files." */;
		bbs[2] = NULL;
		continu_box(bbs);
		return(0);
		}
	got_bh = 1;
	}
else if (strncmp(&ic->iff_type, "CMAP", 4) == 0)
	{
	put_ea_cmap(buf,(int)ic->iff_length/3);
	}
else if (strncmp(&ic->iff_type, "BODY", 4) == 0)
	{
	if (!got_bh)
		return(0);
	put_body(buf, cur_screen);
	}
else if (strncmp(&ic->iff_type, "ANHD", 4) == 0)
	{
	copy_structure(buf, &ah, sizeof(ah));
	intel_swap(&ah.w);
	intel_swap(&ah.h);
	intel_swap(&ah.x);
	intel_swap(&ah.y);
	long_intel_swap(&ah.abstime);
	long_intel_swap(&ah.reltime);
	long_intel_swap(&ah.bits);
	}
else if (strncmp(&ic->iff_type, "DLTA", 4) == 0)
	{
	if (!got_bh)
		return(0);
	if (ah.operation != 5)
		{
		continu_line(anim_121 /* "Unknown compression" */);
		return(0);
		}
	put_dlta(buf, cur_screen);
	}
return(1);
}

static 
read_frame()
{
char *buf;
int colorcount;
int ok;
struct iff_chunk ic;
struct form_chunk fc;
long size;


if (!verify_frame(&fc))
	return(0);
size = fc.fc_length-4;
while (size > 0)
	{
	if (jread(load_fd, &ic, sizeof(ic)) != sizeof(ic) )
		{
		truncated(anim_name);
		return(0);
		}
	long_intel_swap(&ic.iff_length);
	if (ic.iff_length < 0 || ic.iff_length > size)
		{
		return(0);
		}
	ic.iff_length += 1;
	ic.iff_length &= 0xfffffffe;	/* force to even length */

	if ((buf = lbegmem(ic.iff_length)) == NULL)
		return(0);
	ok = next_chunk(&ic, buf);
	freemem(buf);
	if (!ok)
		return(0);
	size -= ic.iff_length + sizeof(ic);
	}
conv_screen(cur_screen);
return(1);
}

/* read in next frame */
next_anim()
{
anim_frame++;
cur_screen = screens[anim_frame&1];
if (!got_bh && anim_frame != 0)
	return(0);		/* didn't find screen dimensions in 1st frame! */
if (anim_frame == 1)
	copy_structure(screens[0], screens[1], (unsigned)ANIM_SSZ);
if (!read_frame())
	return(0);
return(1);
}

speed_anim()
{
return(4);
}

count_anim()
{
if (anim_err)
	return(0);
return(frame_count);
}


/* read in first frame */
start_anim()
{
if (anim_err)
	return(0);
jseek( load_fd, sizeof(animf_h), SEEK_START);	
anim_frame = -1;
return(next_anim());
}
