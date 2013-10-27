
#include "jimk.h"
#include "gif.h"
#include "gif.str"

extern Video_form alt_vf;
extern int sys5;

struct gif_header gif;
struct gif_image gim;

int bad_code_count;
int gif_line;
Video_form *gif_form;
Bfile gif_bf;
char gif_stretch;
static char iphase;
static WORD iy;



shift_cmap(cmap, colors)
UBYTE *cmap;
int colors;
{
while (--colors >= 0)
	*cmap++ >>=2;
}

load_gif(name, screen)
char *name;
Video_form *screen;
{
int i,j;
int c;
int colors;
char buf[80];

gif_form = screen;
gif_line = 0;
gif_stretch = 0;
iphase = 0;
iy = 0;
if (bopen(name, &gif_bf) == 0)
	return(0);
if (bread(&gif_bf, &gif, sizeof(gif) ) < sizeof(gif))
	{
	goto TRUNCOUT;
	}
colors = (1<<((gif.colpix&PIXMASK)+1));
if (gif.colpix&COLTAB)
	{
	if (bread(&gif_bf, gif_form->cmap, colors*3) < colors*3)
		goto TRUNCOUT;
	shift_cmap(gif_form->cmap,colors*3);
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
if (gim.w != XMAX || gim.h != YMAX)
	{
	/* kludgy a bit to take care of color problem */
	goto BADOUT;  /* ldg */
	}
if (gim.w > screen->w || gim.h > screen->h)
	{
	gif_stretch = 1;
	make_bhash();
	if ((gif_form = alloc_big_screen(XMAX, gim.h)) == NULL)
		{
		goto BADOUT;
		}
	clear_form(gif_form);
	copy_cmap(screen->cmap, gif_form->cmap);
	}
if (gim.flags&COLTAB)
	{
	colors = (1<<((gim.flags&PIXMASK)+1));
	if (bread(&gif_bf, gif_form->cmap, colors*3) < colors*3)
		goto TRUNCOUT;
	shift_cmap(gif_form->cmap,colors*3);
	}

wait_sync();
expand_cmap(gif_form->cmap, colors);
find_colors();
jset_colors(0, 256, gif_form->cmap);
copy_cmap(gif_form->cmap, vf.cmap);  /* was alt_vf.cmap */
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
if (gif_stretch)
	{
	shrink_screen(gif_form, screen);
	free_screen(gif_form);
	free_bhash();
	}
return(1);

TRUNCOUT:
truncated(name);
BADOUT:
if (gif_stretch)
	{
	free_screen(gif_form);
	free_bhash();
	}
bclose(&gif_bf);

/* ldg */
jset_colors(0, 256, sys_cmap);
restore_bg();
find_colors(); 
sprintf(buf, gif_100 /* "Error loading file %s" */,name);
continu_line(buf);
/* ldg */
return(0);
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
y -= (gim.h - gif_form->h)/2;
if (y < 0)
	return(0);
if (y >= gif_form->h)
	return(0);
if (gif_stretch)
	stretch_ave_line(pixels, 
		long_to_pt(pt_to_long(gif_form->p) + (long)y*gif_form->bpr), linelen,
		gif_form->w, 1, 1, gif_form->cmap,1);
else
	{
	if (linelen > gif_form->bpr)
		{
		pixels += (linelen-gif_form->bpr)/2;
		linelen = gif_form->bpr;
		}
	copy_bytes(pixels, 
		long_to_pt(pt_to_long(gif_form->p) + (long)y*gif_form->bpr), linelen);
	}
return(0);
}

gif_get_byte()
{
return(bgetbyte(&gif_bf));
}


