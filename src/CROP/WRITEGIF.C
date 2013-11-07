
/* writegif.c - stuff to write out a GIF file.  See also comprs.c */

#include "jimk.h"
#include "gif.h"
#include "crop.h"

char gifsig[] = "GIF87a";
struct gif_header gif;
struct gif_image gim;
Bfile gif_bf;

extern unsigned char *gif_wpt;
extern long gif_wcount;

save_gif(name, screen)
char *name;
Video_form *screen;
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

