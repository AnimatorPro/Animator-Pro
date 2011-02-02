

#include "jimk.h"
#include "fli.h"


uncompfli(f, frame, colors)
Video_form *f;
struct fli_frame *frame;
int colors;
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


gb_read_next_frame(fname,fd,fscreen,fliff,colors)
char *fname;
int fd;
Video_form *fscreen;
struct fli_frame *fliff;
int colors;
{
long size_left;
int ret;

ret = 0;
if (jread(fd,fliff,sizeof(*fliff)) < sizeof(*fliff) )
	{
	truncated(fname);
	}
if (fliff->type != FLIF_MAGIC)
	{
	not_fli_frame(fname);
	}
if (fliff->size >= CBUF_SIZE)
	{
	mangled(fname);
	}
size_left = fliff->size - sizeof(*fliff);
if (jread(fd,fliff+1,size_left) < size_left)
	{
	truncated(fname);
	}
uncompfli(fscreen, fliff, colors);
ret = 1;
BADOUT:
return(ret);
}


