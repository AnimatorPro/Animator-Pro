
/* writefli.c - stuff to generate frames compressed from two adjacent frames. */

#include <fcntl.h>
#include "jimk.h"
#include "fli.h"

extern unsigned write(), read();

extern char *dcomp(), *run_comp(), *skip_run_comp(), *bcomp(), *bsrcomp(),
	*sbsccomp(), *sbsrsccomp(); 
extern unsigned *fccomp(), *lccomp(), *brun();

static int wfli_fd;
static struct fli_head head;
static char *wfli_name;

static char *
full_cmap(cbuf, cmap)
char *cbuf, *cmap;
{
*cbuf++ = 1;
*cbuf++ = 0;
*cbuf++ = 0;
*cbuf++ = 0;
copy_bytes(cmap, cbuf, COLORS*3);
return(norm_pointer(cbuf+COLORS*3));
}

long
fli_comp_frame(comp_buf, last_screen, last_cmap, this_screen, this_cmap,type)
char *comp_buf;
char *last_screen, *last_cmap, *this_screen, *this_cmap;
WORD type;
{
char *c;
struct fli_frame *frame;
struct fli_chunk *chunk;

frame = (struct fli_frame *)comp_buf;
stuff_words(0, frame, sizeof(*frame)/sizeof(WORD) );
chunk = (struct fli_chunk *)(frame+1);
/* 1st make the color map chunk */
if (type == FLI_BRUN)
	c = full_cmap(chunk+1, this_cmap);
else
	c = (char *)fccomp(last_cmap, this_cmap, chunk+1, COLORS);
chunk->type = FLI_COLOR;
chunk->size = pt_to_long(c) - pt_to_long(chunk);
if (chunk->size & 1)	/* force even allignment */
	chunk->size++;
if (chunk->size == EMPTY_DCOMP)
	c = (char *)chunk;
else
	frame->chunks = 1;
chunk = (struct fli_chunk *)c;
switch (type)
	{
	case FLI_LC:
		c = (char *)lccomp(last_screen, this_screen, chunk+1, 320, 200);
		break;
	case FLI_BRUN:
		c = (char *)brun(this_screen, this_screen, chunk+1, 320,200);
		break;
	}
if (c == NULL)
	{
	chunk->size = 64000L+sizeof(chunk);
	chunk->type = FLI_COPY;
	c = norm_pointer((char *)(chunk+1)+64000);
	copy_words(this_screen, chunk+1, 32000);
	}
else
	{
	chunk->type = type;
	chunk->size = pt_to_long(c) - pt_to_long(chunk);
	}
if (chunk->size & 1)	/* force even allignment */
	chunk->size++;
if (chunk->size == EMPTY_DCOMP)
	c = (char *)chunk;
else
	frame->chunks++;
frame->type = FLIF_MAGIC;
frame->size = pt_to_long(c) - pt_to_long(comp_buf);
return(frame->size);
}

static long
fli_save_frame(name,fd,comp_buf,last_screen,
	last_cmap,this_screen,this_cmap,type)
char *name;
int fd;
char *comp_buf;
char *last_screen, *last_cmap, *this_screen, *this_cmap;
WORD type;
{
long size; 

size =  fli_comp_frame(
	comp_buf, last_screen, last_cmap, this_screen, this_cmap,type);
if (jwrite(fd, comp_buf, size) < size)
	{
	fli_finish();
	truncated(name);
	return(0);
	}
head.size += size;
head.frame_count += 1;
return(size);
}



fli_first_frame(cbuf, name,frame2,colors2,frame1,colors1,speed)
char *cbuf;
char *name;
char *frame2;
char *colors2;
char *frame1;
char *colors1;
WORD speed;
{
wfli_name = name;
if ((wfli_fd = jcreate(name)) == 0)
	{
	cant_create(name);
	return(0);
	}
head.frame_count = 0;
head.size = sizeof(head);
head.speed = speed;
head.type = FLIH_MAGIC;
head.width = XMAX;
head.height = YMAX;
head.bits_a_pixel = 8;
if (jwrite(wfli_fd, &head, sizeof(head)) < sizeof(head))
	{
	truncated(name);
	return(0);
	}
return(fli_save_frame(wfli_name,wfli_fd,cbuf,NULL, 
	colors2, frame1, colors1, FLI_BRUN) != 0);
}


fli_next_frame(cbuf,last_screen, last_cmap, this_screen, this_cmap)
char *cbuf, *last_screen, *last_cmap, *this_screen, *this_cmap;
{
return(
 fli_save_frame(wfli_name,wfli_fd,cbuf,last_screen, 
 	last_cmap, this_screen, this_cmap, FLI_LC) != 0);
}

static
fli_finish()
{
int success = 1;

if (wfli_fd != 0)
	{
	if (jseek(wfli_fd, 0L, 0) == -1)
		success = 0;
	else
		{
		if (jwrite(wfli_fd, &head, sizeof(head)) < sizeof(head))
			{
			truncated(wfli_name);
			success = 0;
			}
		}
	jclose(wfli_fd);
	wfli_fd = 0;
	}
return(success);
}

fli_last_frame(cbuf,last_screen, last_cmap, first_screen, first_cmap)
char *cbuf;
int *last_screen, *last_cmap, *first_screen, *first_cmap;
{
if (!fli_save_frame(wfli_name,wfli_fd,cbuf,last_screen, last_cmap,
	first_screen, first_cmap, FLI_LC) )
	{
	truncated(wfli_name);
	return(0);
	}
head.flags = (FLI_FINISHED | FLI_LOOPED);
head.frame_count-=1;
if (!fli_finish())
	return(0);
return(1);
}

