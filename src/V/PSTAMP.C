
/* pstamp.c - routine to read the first frame of a FLIC into a buffer
   1/5 the size of a screen.  (4 out of 5 lines are discarded here).
   Then do an x dimension shrink by 5 and put result on screen somewhere.
   */

#include "jimk.h"
#include "fli.h"

#define SHRINK 5

static
un5compfli(f, frame, c)
Vscreen *f;
struct fli_frame *frame;
char *c;
{
int j;
struct fli_chunk *chunk;
PLANEPTR p;

p = f->p;
for (j=0;j<frame->chunks;j++)
	{
	chunk = (struct fli_chunk *)c;
	switch (chunk->type)
		{
		case FLI_COLOR:
			fcuncomp(chunk+1,f->cmap);
			break;
		case FLI_LC:
			break;
		case FLI_ICOLORS:
			copy_cmap(init_cmap, f->cmap);
			break;
		case FLI_BLACK:
			cblock(p,0,0,f->w,f->h,0);
			break;
		case FLI_BRUN:
			un5brun(chunk+1, p, f->h);
			break;
		case FLI_COPY:
			un5copy(chunk+1,p,BPR,f->h);
			break;
		default:
			break;
		}
	c = norm_pointer(c + chunk->size);
	}
}

static
un5fli(f,fd,name)
Vscreen *f;
int fd;
char *name;
{
struct fli_frame frame;	
char *cbuf;
long size;
int ret;


if (jread(fd, &frame, sizeof(frame)) < sizeof(frame))
	{
	truncated(name);
	return(0);
	}
if (frame.type != FLIF_MAGIC)
	{
	not_fli_frame();
	return(0);
	}
size = frame.size - sizeof(frame);
if ((cbuf = lbegmem(size)) == NULL)
	return(0);
if (jread(fd, cbuf, size) < size)
	{
	truncated(name);
	return(0);
	}
ret = un5compfli(f, &frame, cbuf);
freemem(cbuf);
return(ret);
}


static
make_tcmap(cmap, d)
register PLANEPTR cmap, d;
{
int i;

i = COLORS;
while (--i >= 0)
	{
	*d++ = 6*cmap[0]/64 * 36 + 6*cmap[1]/64 * 6 + 6*cmap[2]/64 + START_SIXCUBE;
	cmap += 3;
	}

}


postage_stamp(x,y,name)
int x,y;
char *name;
{
struct fli_head flih;
int fd;
Vscreen *form;
int ret;
PLANEPTR tcmap;

if ((fd = read_fli_head(name, &flih)) == 0)
	return(0);
if ((form = alloc_big_screen(XMAX,YMAX/SHRINK)) == NULL)
	{
	jclose(fd);
	return(0);
	}
if ((tcmap = begmem(COLORS)) == NULL)
	{
	free_screen(form);
	jclose(fd);
	return(0);
	}
if ((ret = un5fli(form,fd,name)) != 0)
	{
	make_tcmap(form->cmap, tcmap);
	xlat(tcmap,form->p,form->bpr*form->h);
	shrink5(64,40,0,0,form->p,form->bpr,x,y,vf.p,vf.bpr);
	}
freemem(tcmap);
free_screen(form);
jclose(fd);
return(ret);
}

