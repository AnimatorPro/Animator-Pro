
/* readrif.c - Stuff to handle Amiga/Zoetrope/Live! RIF files (which
   are sequences of pictures much like a FLI file. */

#include <fcntl.h>
#include <stdio.h>
#include "jimk.h"
#include "jiff.h"
#include "vcomp.h"
#include "readrif.str"

#define RIF_BPR 40
#define YMAX 200
#define WIDTH 320
#define HEIGHT 200
#define RIF_DEPTH 5
#define PLANE_SIZE (RIF_BPR*HEIGHT)
#define RIF_SSZ ((long)RIF_DEPTH*(long)PLANE_SIZE)
#define MAX_SCREENS (4000)

extern UBYTE sys_cmap[];


static load_fd;
static char *rif_name;
static PLANEPTR amiga_screen1;
static int rif_err = -1;

static struct riff_head riff_h;
static struct vcomp_iff vc_h;




static 
open_verify_rif(name)
char *name;
{
if ((load_fd = jopen(name, 0)) <= 0)
	{
	cant_find(name);
	return(0);
	}
if (jread(load_fd, &riff_h, sizeof(riff_h)) < sizeof(riff_h) )
	{
	truncated(name);
	goto BADEXIT;
	}
if (strncmp(riff_h.iff_type, "RIFF", 4))
	{
	continu_line(readrif_101 /* "Not a RIFF file." */);
	goto BADEXIT;
	}
intel_swap(&riff_h.width);
intel_swap(&riff_h.height);
intel_swap(&riff_h.depth);
intel_swap(&riff_h.frames_written);
intel_swap(&riff_h.frame_count);
intel_swap(&riff_h.jiffies_frame);
if (riff_h.width != WIDTH || riff_h.height != HEIGHT ||
	riff_h.depth > RIF_DEPTH)
	{
	continu_line(readrif_102 /* "File not 320x200." */);
	goto BADEXIT;
	}
if (riff_h.frames_written != 0)
	riff_h.frame_count = riff_h.frames_written;
return(1);
BADEXIT:
jclose(load_fd);
load_fd = 0;
return(0);
}

static
swap_comp(comp)
struct comp_size *comp;
{
intel_swap(&comp->comp);
intel_swap(&comp->size);
}

static
next_vcomp_iff(name, comp_buf)
char *name;
char *comp_buf;
{
unsigned int load_size;
int i;

if ( jread(load_fd, &vc_h, sizeof(vc_h)) < sizeof(vc_h) )
	{
	truncated(name);
	return(0);
	}
if (strncmp(vc_h.iff_type, "VRUN", 4))
	{
	continu_line(readrif_104 /* "Expecting a VRUN here..." */);
	return(0);
	}
long_intel_swap(&vc_h.iff_size);
intel_swap(&vc_h.xoff);
intel_swap(&vc_h.yoff);
intel_swap(&vc_h.width);
intel_swap(&vc_h.height);
intel_swap(&vc_h.depth);
intel_swap(&vc_h.ViewMode);
for (i=0; i<8; i++)
	swap_comp(&(vc_h.comps[i]));
intel_swap(&vc_h.hold_time);
intel_swap(&vc_h.reserved[0]);
for (i=0; i<32; i++)
	intel_swap(&(vc_h.cmap[i]));
load_size = vc_h.iff_size - sizeof(Vcomp_head);
if ( (jread(load_fd, comp_buf, load_size)) < load_size)
	{
	truncated(name);
	return(0);
	}
return(1);
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
bpr = RIF_BPR;
i = YMAX;
while (--i >= 0)
	{
	*pt++ = acc;
	acc += bpr;
	}
}



/* fake amiga... */
static
put_amiga_cmap(cmap,count)
UWORD *cmap;
int count;
{
int i;
UBYTE *ct;
UWORD cm;

if (count > 256)
	count = 256;
ct = vf.cmap;
for (i=0; i<count; i++)
	{
	cm = *cmap++;
	*ct++ = ((cm&0xf00)>>6);
	*ct++ = ((cm&0x0f0)>>2);
	*ct++ = ((cm&0x00f)<<2);
	}
wait_sync();
jset_colors(0, count, sys_cmap);
}


/* close file, free buffers */
close_rif()
{
if (load_fd != 0)
	{
	jclose(load_fd);
	load_fd = 0;
	}
gentle_freemem(amiga_screen1);
amiga_screen1 = 0;
rif_err = -1;
}

open_rif(name)
char *name;
{
if (rif_err != -1)
	{
	continu_line(readrif_105 /* "RIF file already open" */);
	return(0);
	}
rif_name = name;
if (open_verify_rif(name))
	{
	if ((amiga_screen1 = lbegmem(RIF_SSZ)) == NULL)
		{
		close_rif();
		return(0);
		}
	}
else
	return(0);
rif_err = 0;
make_ytable();
return(1);
}

/* read in next frame */
next_rif()
{
register WORD i;
WORD j;
WORD success = 0;
register PLANEPTR thispt;
register PLANEPTR readpt;
WORD *comp_buf = NULL;

if (rif_err)
	return(0);
if ((comp_buf = lbegmem(50000L)) == NULL)
	return(0);
if (next_vcomp_iff(rif_name, (char *)comp_buf))
	{
	put_amiga_cmap((UWORD *)vc_h.cmap,32);
	thispt = (PLANEPTR)amiga_screen1;
	readpt = (PLANEPTR)comp_buf;
	for (j=0; j<riff_h.depth; j++)
		{
		switch (vc_h.comps[j].comp)
			{
			case VCOMP_NONE:
				copy_lots(readpt, thispt, (long)vc_h.comps[j].size);
				break;
			case VCOMP_VRUN:
				decode_vplane(readpt, thispt, 
					RIF_BPR);
				break;
			case VCOMP_SKIP:
				decode_vkplane(readpt, thispt,
					RIF_BPR);
				break;
			}
		thispt += PLANE_SIZE;
		readpt += vc_h.comps[j].size;
		}
	conv_screen(amiga_screen1);
	success = 1;
	}
freemem(comp_buf);
return(success);
}

speed_rif()
{
if (rif_err)
	return(0);
return(riff_h.jiffies_frame);
}

count_rif()
{
if (rif_err)
	return(0);
return(riff_h.frame_count);
}

/* read in first frame */
start_rif()
{
if (rif_err)
	return(0);
jseek( load_fd, sizeof(riff_h) + (long)riff_h.frame_count*sizeof(long),
	SEEK_START);	
return(next_rif());
}
