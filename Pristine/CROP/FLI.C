
/* Fli.c - This has the low level routines to uncompress and play-back a FLI 
	flic.  Also the glue between FLI files and crop.c */


#include "jimk.h"
#include "fli.h"
#include "fli.str"

static int fliin;
static char *fliin_name;
static int fli_err = -1;
static struct fli_head inflih;


static
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

static
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
		case FLI_ICOLORS:
			copy_cmap(init_cmap, f->cmap);
			if (colors)
				see_cmap();
			fli_has_colors = 1;
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



static
not_fli_frame()
{
continu_line(fli_101 /* "Bad magic! Not a FLI frame." */);
}


static
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



static
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


static
read_next_frame(fname,fd,fscreen,colors)
char *fname;
int fd;
Video_form *fscreen;
int colors;		/* update hw color map??? */
{
struct fli_frame *fliff;
int ret;

if ((fliff = lbegmem(CBUF_SIZE)) == NULL)
	return(0);
ret = gb_read_next_frame(fname, fd, fscreen, fliff, colors);
freemem(fliff);
return(ret);
}

open_fli(name)
char *name;
{
close_fli();
fliin_name = name;
if ((fliin = read_fli_head(fliin_name, &inflih)) == 0)
	{
	return(0);
	}
fli_err = 0;
return(1);
}

close_fli()
{
if (fliin != 0)
	{
	jclose(fliin);
	fliin = 0;
	}
fli_err = -1;
}

start_fli()
{
if (fli_err)
	return(0);
jseek(fliin, (long)sizeof(struct fli_head), SEEK_START);
return(next_fli());
}

next_fli()
{
if (fli_err)
	return(0);
return(read_next_frame(fliin_name,fliin,&vf,1));
}

count_fli()
{
if (fli_err)
	return(0);
return(inflih.frame_count);
}

speed_fli()
{
if (fli_err)
	return(0);
return(inflih.speed);
}

