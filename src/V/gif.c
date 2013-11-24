
/* gif.c - High level gif routines.  Take care of the file packaging but
   not the compression/decompression.  */

#include "jimk.h"
#include "gif.h"
#include "gif.str"
#include "peekpok_.h"

static struct gif_header gif;
static struct gif_image gim;

static char gifsig[] = "GIF87a";

int bad_code_count;
static int gif_line;
Bfile gif_bf;
static char iphase;
static WORD iy;

gif_get_byte()
{
return(bgetbyte(&gif_bf));
}


gif_out_line(pixels, linelen)
UBYTE *pixels;
int linelen;
{
int y;

y = gif_line;
if (gim.flags&ITLV_BIT)
	{
	y = iy;
	switch (iphase)
		{
		case 0:
		case 1:
			iy+=8;
			break;
		case 2:
			iy += 4;
			break;
		case 3:
			iy += 2;
			break;
		}
	if (iy >= gim.h)
		{
		switch (iphase)
			{
			case 0:
				iy = 4;
				break;
			case 1:
				iy = 2;
				break;
			case 2:
				iy = 1;
				break;
			}
		iphase++;
		}
	}
gif_line++;
copy_bytes(pixels, 
	long_to_pt(pt_to_long(render_form->p) + (long)y*BPR), linelen);
return(0);
}


static
shift_cmap(cmap, colors)
UBYTE *cmap;
int colors;
{
while (--colors >= 0)
	*cmap++ >>=2;
}

static char *wrong_res_lines[] =
	{
	gif_101 /* "File isn't 320x200." */,
	gif_102 /* "Use Autodesk Animator converter." */,
	NULL,
	};

load_gif(name, screen)
char *name;
Vscreen *screen;
{
int i,j;
int c;
int colors;
char buf[80];

gif_line = 0;
iphase = 0;
iy = 0;
if (bopen(name, &gif_bf) == 0)
	{
	cant_find(name);
	return(0);
	}
if (bread(&gif_bf, (void *)&gif, sizeof(gif) ) < sizeof(gif))
	{
	goto TRUNCOUT;
	}
if (bcompare(gif.giftype, gifsig, 3) != 3)
	{
	continu_line(gif_103 /* "Not a good GIF file" */);
	goto BADOUT;
	}
if (bcompare(gif.giftype+3,gifsig+3, 3) != 3)
	{
	continu_line(gif_104 /* "Unknown GIF revision, sorry" */);
	goto BADOUT;
	}
colors = (1<<((gif.colpix&PIXMASK)+1));
if (gif.colpix&COLTAB)
	{
	if (bread(&gif_bf, render_form->cmap, colors*3) < colors*3)
		goto TRUNCOUT;
	shift_cmap(render_form->cmap,colors*3);
	jset_colors(0, 256, render_form->cmap);
	}
for (;;)	/* skip over extension blocks and other junk til get ',' */
	{
	if ((c = bgetbyte(&gif_bf)) == READ_ERROR)
		goto TRUNCOUT;
	if (c == ',')
		break;
	if (c == ';')	/* semi-colon is end of piccie */
		goto TRUNCOUT;
	if (c == '!')	/* extension block */
		{
		if ((c = bgetbyte(&gif_bf)) == READ_ERROR)	/* skip extension type */
			goto TRUNCOUT;
		for (;;)
			{
			if ((c = bgetbyte(&gif_bf)) == READ_ERROR)
				goto TRUNCOUT;
			if (c == 0)	/* zero 'count' means end of extension */
				break;
			while (--c >= 0)
				{
				if (bgetbyte(&gif_bf) == READ_ERROR)
					goto TRUNCOUT;
				}
			}
		}
	}
if (bread(&gif_bf, (void *)&gim, sizeof(gim) ) < sizeof(gim) )
	goto TRUNCOUT;
if (gim.w != XMAX || gim.h != YMAX)
	{
	continu_box(wrong_res_lines);
	goto BADOUT;
	}
if (gim.flags&COLTAB)
	{
	colors = (1<<((gim.flags&PIXMASK)+1));
	if (bread(&gif_bf, render_form->cmap, colors*3) < colors*3)
		goto TRUNCOUT;
	shift_cmap(render_form->cmap,colors*3);
	jset_colors(0, 256, render_form->cmap);
	}
switch (gif_decoder(gim.w))
	{
	case READ_ERROR:
	case BAD_CODE_SIZE:
		goto TRUNCOUT;
	case OUT_OF_MEMORY:
		outta_memory();
		goto BADOUT;
	default:
		break;
	}
bclose(&gif_bf);
return(1);

TRUNCOUT:
truncated(name);
BADOUT:
bclose(&gif_bf);
return(0);
}

extern unsigned char *gif_wpt;
extern long gif_wcount;



save_gif(name, screen)
char *name;
Vscreen *screen;
{
int i;

gif_wpt = screen->p;
gif_wcount = (long)screen->bpr*screen->h;
zero_structure(&gif, sizeof(gif));
if (!bcreate(name, &gif_bf))
	{
	cant_create(name);
	return(0);
	}
strcpy(gif.giftype, gifsig);
gif.w = gim.w = XMAX;
gif.h = gim.h = YMAX;
gim.x = gim.y = gim.flags = 0;
gif.colpix = COLPIXVGA13;
if (bwrite(&gif_bf, &gif, sizeof(gif) ) < sizeof(gif))
	{
	goto TRUNCOUT;
	}
/* write global color map */
for (i=0; i<COLORS*3; i++)
	{
	if ((bputbyte(&gif_bf, screen->cmap[i]<<2)) < 0)
		goto TRUNCOUT;
	}
if (bputbyte(&gif_bf, ',') < 0)	/* comma to start image */
	goto TRUNCOUT;

if (bwrite(&gif_bf, &gim, sizeof(gim) ) < sizeof(gim))
	goto TRUNCOUT;
bputbyte(&gif_bf,8);
bflush(&gif_bf);
i = gif_compress_data(8);
switch (i)
	{
	case -2:
		outta_memory();
		goto BADOUT;
	case -3:
		goto TRUNCOUT;
	default:
		break;
	}
bputbyte(&gif_bf, ';');	/* end of file for gif */
bclose(&gif_bf);
return(1);
TRUNCOUT:
truncated(name);
BADOUT:
bclose(&gif_bf);
jdelete(name);
return(0);
}

