
/* vision.c - Handles AT&T Targa format pictures.  (Which is same as
   Everex Vision 16, hence the name.) */

#include "jimk.h"
#include "crop.h"
#include "img.h"
#include "vision.str"

extern UBYTE *get_hist();
extern UBYTE bitmasks[];
extern struct bfile rgb_files[3];
extern char *rgb_names[3]; 



static struct imgfile vh;
static char *vision_name;
static int vision_err = -1;
static struct bfile vision_bf;
static long data_offset;
static char is_compressed;
static char is_flipped;
static long linebytes;
static long vnext;
static UBYTE *vstart;
static UBYTE *buf;
static UBYTE *over;
static int over_count;
static UBYTE *rgb_bufs[3];
static int bap;

#define VWID vh.imgdata.width

char *not_vision_lines[] = {
	vision_100 /* "This file doesn't seem to be" */,
	vision_101 /* "Targa compatible.  It isn't" */,
	vision_102 /* "16, 24, or 32 bit a pixel." */,
	NULL,
	};

open_verify_vision()
{
int i;

if ((bopen(vision_name, &vision_bf)) == 0)
	{
	cant_find(vision_name);
	return(0);
	}
if (bread(&vision_bf, &vh, sizeof(vh)) <
	sizeof(vh))
	{
	trunc();
	return(0);
	}
linebytes = VWID;
switch (vh.imgdata.pixsiz)
	{
	case 16:
		bap = 2;
		break;
	case 24:
		bap = 3;
		break;
	case 32:
		bap = 4;
		break;
	default:
		continu_box(not_vision_lines);
		return(0);
	}
linebytes *= bap;
switch (vh.imgtype)
    {
	case 2:             /* uncompressed */
		is_compressed = 0;
		break;
	case 10:
		is_compressed = 1;
		break;
	default:
		continu_line(vision_103 /* "Unknown compression type" */);
		return(0);
	}
if ((vh.imgdata.imgdesc & INLEAVE) != 0)
	{
	continu_line(vision_104 /* "Sorry, can't deal with Interleave" */);
	return(0);
	}
if ((buf = lbegmem(linebytes + 130*bap)) == NULL)
	return(0);
over = buf + linebytes;	/* extra at end to make decompression easier */
for (i=0; i<3; i++)
	{
	if ((rgb_bufs[i] = begmem(VWID)) == NULL)
		return(0);
	}
is_flipped = !((vh.imgdata.imgdesc&SCRORG) == SCRORG);
if ((pic_cel = alloc_cel(VWID, vh.imgdata.height,
	vh.imgdata.xorg, vh.imgdata.yorg)) == NULL)
	return(0);
vstart = pic_cel->p;
vnext = pic_cel->bpr;
if (is_flipped)
	{
	vstart = long_to_pt(
		pt_to_long(vstart) + vnext * (vh.imgdata.height-1));
	vnext = -vnext;
	}
data_offset = vh.idlength + sizeof(vh);
if (vh.maptype != 0)
	data_offset += ((vh.cms.mapbits+7)/8)*vh.cms.maplen;
return(1);
}


trunc()
{
truncated(vision_name);
}

vision_nextline()
{
int i, lpos;
UWORD *w;
WORD ww;
UBYTE *p;
UBYTE *r,*g,*b;
UBYTE pbuf[4];	/* max possible bytes a pixel */

if (is_compressed)
	{
	/* first deal with any overflow from last line */
	copy_bytes(over, buf, over_count);
	lpos = linebytes - over_count;
	p = buf + over_count;
	while (lpos > 0)
		{
		if ((ww = bgetbyte(&vision_bf)) < 0)
			{
			trunc();
			return(0);
			}
		if (ww&0x80)	/* it's a run dude */
			{
			ww = (ww&0x7f)+1;	/* length of run - 1*/
			if (bread(&vision_bf, pbuf, bap) != bap) /* get data to repeat */
				{
				trunc();
				return(1);
				}
			while (--ww >= 0)
				{
				copy_bytes(pbuf, p, bap);
				p += bap;
				lpos -= bap;
				}
			}
		else           /* This bit is just raw data */
			{
			i = (ww+1)*bap;	/* length in bytes */
			if (bread(&vision_bf, p, i) != i)
				{
				trunc();
				return(0);
				}
			p += i;
			lpos -= i;
			}
		}
	over_count = -lpos;
	}
else
	{
	if (bread(&vision_bf, buf, (int)linebytes) != linebytes)
		{
		trunc();
		return(0);
		}
	}
p = buf;
r = rgb_bufs[0];
g = rgb_bufs[1];
b = rgb_bufs[2];
i = VWID;
switch (vh.imgdata.pixsiz)
	{
	case 24:
		while (--i >= 0)
			{
			*b++ = (*p++>>2);
			*g++ = (*p++>>2);
			*r++ = (*p++>>2);
			}
		break;
	case 32:
		while (--i >= 0)
			{
			*b++ = (*p++>>2);
			*g++ = (*p++>>2);
			*r++ = (*p>>2);
			p += 2;
			}
		break;
	case 16:
		w = (UWORD *)p;
		while (--i >= 0)
			{
			ww = *w++;
			*r++ = ((ww&0x7c00)>>9);
			*g++ = ((ww&0x3e0)>>4);
			*b++ = ((ww&0x1f)<<1);
			}
		break;
	}
return(1);
}

grey_cur_line(p)
UBYTE *p;
{
UBYTE *r, *g, *b;
int i;

r = rgb_bufs[0];
g = rgb_bufs[1];
b = rgb_bufs[2];
i = VWID;
while (--i >= 0)
	{
	*p++ = (*r++ + *g++ + *b++)/3;
	}
}

load_bw_vision()
{
int y;
UBYTE *p;

grey_cmap(vf.cmap);
see_cmap();
find_colors();
if (bseek(&vision_bf, data_offset, 0) < 0L)
	{
	trunc();
	return(0);
	}
over_count = 0;
p = vstart;
for (y=0; y<vh.imgdata.height; y++)
	{
	if (y%10 == 0)
		{
		char buf[40];
		sprintf(buf, vision_105 /* " %3d of %d" */, y, vh.imgdata.height);
		stext(buf, 0, 0, sblack, swhite);
		}
	if (!vision_nextline())
		return(0);
	grey_cur_line(p);
	p = long_to_pt(pt_to_long(p) + vnext);
	}
return(1);
}


reset_vbf()
{
if (bseek(&vision_bf, data_offset, 0) < 0L)
	{
	trunc();
	return(0);
	}
return(1);
}


close_vision()
{
int i;
bclose(&vision_bf);
vision_err = -1;
free_cel(pic_cel);
pic_cel = NULL;
gentle_freemem(buf);
buf = NULL;
for (i=0; i<3; i++)
	{
	gentle_freemem(rgb_bufs[i]);
	rgb_bufs[i] = NULL;
	}
}

open_vision(name)
char *name;
{
int ok;

if (vision_err != -1)
	{
	continu_line(vision_106 /* "Already opened TARGA/VISION file" */);
	return(0);
	}
vision_name = name;
if (!open_verify_vision())
	{
	close_vision();
	return(0);
	}
if (!load_bw_vision())
	{
	close_vision();
	return(0);
	}
vision_err = 0;
return(1);
}


count_vision()
{
if (vision_err)
	return(0);
return(1);
}

next_vision()
{
if (vision_err)
	return(0);
tile_cel(pic_cel);
return(1);
}

start_vision()
{
return(next_vision());
}

speed_vision()
{
return(4);
}

cur_to_3files(w,b)
int w;
UBYTE *b;
{
int i;

for (i=0; i<3; i++)
	{
	iscale(rgb_bufs[i], VWID, b, w);
	if (bwrite(rgb_files+i, b, w) != w)
		{
		truncated(rgb_names[i]);
		return(0);
		}
	}
return(1);
}

scale_vision(w,h)
int w,h;
{
int y;
int ok;
UBYTE *wb;

if (vision_err)
	return(0);
free_cel(pic_cel);
pic_cel = NULL;
stext(vision_107 /* " Scaling along X" */, 0, 0, sblack, swhite);
if (!reset_vbf())
	return(0);
y = vh.imgdata.height;
over_count = 0;
ok = 1;
if (create_rgb_files())
	{
	for (y=0; y<vh.imgdata.height; y++)
		{
		if (y%10 == 0)
			{
			char buf[30];
			sprintf(buf, vision_108 /* "%3d of %d" */, y, vh.imgdata.height);
			stext(buf, 0, 10, sblack, swhite);
			}
		if (!vision_nextline())
			{
			ok = 0;
			break;
			}
		if ((wb = begmem(w)) == NULL)
			{
			ok = 0;
			break;
			}
		if (!cur_to_3files(w,wb))
			{
			ok = 0;
			break;
			}
		freemem(wb);
		}
	close_rgb_files();
	if (ok)
		{
		if (scale_rgb_files(vh.imgdata.height, w, h))
			{
			if (is_flipped)
				flip_cel(pic_cel);
			tile_cel(pic_cel);
			}
		else
			ok = 0;
		}
	}
return(ok);
}
