

#include "jimk.h"
#include "fli.h"
#include "fli.str"

notafli(name)
char *name;
{
char *buf[3];

buf[0] = name;
buf[1] = fli_100 /* "Isn't a FLI file, sorry" */;
buf[2] = NULL;
continu_box(buf);
}

char fli_has_colors;

uncompfli(f, frame, colors)
Video_form *f;
struct fli_frame *frame;
int colors;
{
int j;
char *c;
struct fli_chunk *chunk;

fli_has_colors = 0;
c = (char *)(frame+1);
if (colors)
	wait_vblank();
for (j=0;j<frame->chunks;j++)
	{
	chunk = (struct fli_chunk *)c;
	switch (chunk->type)
		{
		case FLI_WRUN:
			unrun(chunk+1, f->p);
			break;
		case FLI_SBSRSC: 
			unsbsrsccomp(chunk+1, f->p);
			break;
		case FLI_COLOR:
			if (colors)
				{
				cset_colors(chunk+1);
				}
			fcuncomp(chunk+1,f->cmap);
			fli_has_colors = 1;
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
		default:
			printf(fli_101 /* "unknown chunk type %d\n" */, chunk->type);
			break;
		}
	c = norm_pointer(c + chunk->size);
	}
}



not_fli_frame()
{
continu_line(fli_102 /* "Bad magic! Not a FLI frame." */);
}


read_fli_head(title, flih)
char *title;
struct fli_head *flih;
{
int fd;

if ((fd = jopen(title, 0)) == 0)
	{
	cant_find(title);
	return(0);
	}
/* read in fli header and check it's magic number */
if (jread(fd,flih,sizeof(*flih)) < sizeof(*flih) )
	{
	truncated(title);
	jclose(fd);
	return(0);
	}
if (flih->type != FLIH_MAGIC)
	{
	notafli(title);
	jclose(fd);
	return(0);
	}
return(fd);
}

/* got buffer read next frame */
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
if (jread(fd,fliff+1,size_left) < size_left)
	{
	truncated(fname);
	goto BADOUT;
	}
uncompfli(fscreen, fliff, colors);
ret = 1;
BADOUT:
return(ret);
}


read_next_frame(fname,fd,fscreen,colors)
char *fname;
int fd;
Video_form *fscreen;
int colors;		/* update hw color map??? */
{
struct fli_frame *fliff;
int ret;

if ((fliff = begmem(CBUF_SIZE)) == NULL)
	return(0);
ret = gb_read_next_frame(fname, fd, fscreen, fliff, colors);
freemem(fliff);
return(ret);
}



