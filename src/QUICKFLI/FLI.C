
/* fli.c - Copyright 1989 Jim Kent; Dancing Flame, San Francisco.
   A perpetual non-exclusive license to use this source code in non-
   commercial applications is given to all owners of the Autodesk Animator.
   If you wish to use this code in an application for resale please
   contact Autodesk Inc., Sausilito, CA  USA  phone (415) 332-2244. */


#include "jimk.h"
#include "fli.h"


/* Go through all the chunks in a frame switching on chunk type to
   appropriate decompression routine. */
uncompfli(f, frame, colors)
Video_form *f;		/* screen to store uncompressed result */
struct fli_frame *frame;	/* points to RAM image of a flic frame */
int colors;			/* Update hw color palette? */
{
int j;
char *c;
struct fli_chunk *chunk;

c = (char *)(frame+1);
if (colors)
	wait_vblank();
for (j=0;j<frame->chunks;j++)
	{
	chunk = (struct fli_chunk *)c;
	switch (chunk->type)
		{
		case FLI_COLOR:
			if (colors)
				{
				cset_colors(chunk+1);
				}
			fcuncomp(chunk+1,f->cmap);
			break;
		case FLI_LC:
			unlccomp(chunk+1, f->p);
			break;
		case FLI_BLACK:
			clear_form(f);
			break;
		case FLI_BRUN:
			unbrun(chunk+1, f->p, f->h);
			break;
		case FLI_COPY:
			copy_words(chunk+1,f->p,32000);
			break;
		}
	c = norm_pointer(c + chunk->size);
	}
}



not_fli_frame()
{
bailout("Bad magic! Not a FLI frame");
}

mangled(name)
char *name;
{
char buf[200];
sprintf(buf, "File %s appears to be damaged");
bailout(buf);
}


/* Got buffer read next frame.  Reads a single frame into a buffer that
   should be at least CBUF_SIZE.  After read does decompression. */
gb_read_next_frame(fname,fd,fscreen,fliff,colors)
char *fname;				/* name of flic (for error handling) */
int fd;						/* flic file handle */
Video_form *fscreen;		/* screen to decompress onto */
struct fli_frame *fliff;	/* fliff buffer at least CBUF_SIZE */
int colors;					/* Update hw color palette? */
{
long size_left;
int ret;

ret = 0;
if (jread(fd,fliff,sizeof(*fliff)) < sizeof(*fliff) )
	{
	truncated(fname);
	goto BADOUT;
	}
if (fliff->type != FLIF_MAGIC)
	{
	not_fli_frame(fname);
	goto BADOUT;
	}
if (fliff->size >= CBUF_SIZE)
	{
	mangled(fname);
	goto BADOUT;
	}
size_left = fliff->size - sizeof(*fliff);
if (jread(fd,fliff+1,(unsigned)size_left) < size_left)
	{
	truncated(fname);
	goto BADOUT;
	}
uncompfli(fscreen, fliff, colors);
ret = 1;
BADOUT:
return(ret);
}


