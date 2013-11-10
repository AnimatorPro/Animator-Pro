/* gif1.c - The high level GIF reading routines.  See writegif for the
   write side.  Also decode.c for lower level GIF reading code. */

#include "jimk.h"
#include "gif.h"
#include "crop.h"
#include "gif.str"

extern struct gif_header gif;
extern struct gif_image gim;
extern char gifsig[];
extern Bfile gif_bf;

static int gif_line;
static char iphase;
static WORD iy;
static UBYTE gif_cmap[256*3];
static int gif_colors;


count_gif()
{
return(1);
}


speed_gif()
{
return(4);
}


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
	long_to_pt(pt_to_long(pic_cel->p) + (long)y*pic_cel->bpr), linelen);
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


static
load_gif(name)
char *name;
{
int i,j;
int c;
char buf[80];

gif_line = 0;
iphase = 0;
iy = 0;
/* make error recovery easier */
free_cel(pic_cel);
pic_cel = NULL;
if (bopen(name, &gif_bf) == 0)
	return(0);
if (bread(&gif_bf, &gif, sizeof(gif) ) < sizeof(gif))
	{
	goto TRUNCOUT;
	}
if (bcompare(gif.giftype, gifsig, 3) != 3)
	{
	continu_line(gif_100 /* "Not a good GIF file" */);
	goto BADOUT;
	}
if (bcompare(gif.giftype+3,gifsig+3, 3) != 3)
	{
	continu_line(gif_101 /* "Unknown GIF revision, sorry" */);
	goto BADOUT;
	}
gif_colors = (1<<((gif.colpix&PIXMASK)+1));
if (gif.colpix&COLTAB)
	{
	if (bread(&gif_bf, gif_cmap, gif_colors*3) < gif_colors*3)
		goto TRUNCOUT;
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
if (bread(&gif_bf, &gim, sizeof(gim) ) < sizeof(gim) )
	goto TRUNCOUT;
if ((pic_cel = alloc_cel(gim.w, gim.h, gim.x, gim.y)) == NULL)
	goto BADOUT;
copy_cmap(vf.cmap, pic_cel->cmap);
copy_bytes(gif_cmap, pic_cel->cmap, 3*gif_colors);
if (gim.flags&COLTAB)
	{
	gif_colors = (1<<((gim.flags&PIXMASK)+1));
	if (bread(&gif_bf, pic_cel->cmap, gif_colors*3) < gif_colors*3)
		goto TRUNCOUT;
	}
shift_cmap(pic_cel->cmap,gif_colors*3);
switch (gif_decoder(gim.w))
	{
	case READ_ERROR:
	case BAD_CODE_SIZE:
		goto TRUNCOUT;
	case OUT_OF_MEMORY:
		outta_memory(name);
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


/* standard methods to any graphics file class */
static int gif_err = -1;
static char *gif_name;

open_gif(name)
char *name;
{
if (gif_err != -1)
	{
	continu_line(gif_102 /* "Already opened GIF" */);
	return(0);
	}
gif_name = name;
if (!load_gif(gif_name))
	{
	close_gif();
	return(0);
	}
copy_bytes(pic_cel->cmap, vf.cmap, gif_colors*3);
see_cmap();
tile_cel(pic_cel);
gif_err = 0;
return(1);
}

close_gif()
{
free_cel(pic_cel);
pic_cel = NULL;
gif_err = -1;
}

start_gif()
{
if (gif_err)
	return(0);
return(1);
}

next_gif()
{
if (gif_err)
	return(0);
return(1);
}

