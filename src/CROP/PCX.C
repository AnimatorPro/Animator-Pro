
/* pcx.c - Stuff to handle PC-PaintBrush (.PCX) files */

#include "jimk.h"
#include "crop.h"
#include "pcx.str"

struct pcx_header
	{
	UBYTE magic, version, encode, bitpx;
	WORD x1,y1,x2,y2;
	WORD cardw, cardh;
	UBYTE palette[48];
	UBYTE vmode, nplanes;
	WORD bpl;	/* bytes per line of piccie */
	UBYTE pad[60];
	};
static struct pcx_header pcxh;
static int pcx_err = -1;
static char *pcx_name;
static struct bfile pcx_bf;
static int pcw, pch;
static UBYTE *over;
static UBYTE *buf;
static UBYTE with_cmap;
static int over_count;

static UBYTE default_pcx_cmap[] = 
		  {
	      0x00, 0x00, 0x00,       
	      0x00, 0x00, 0xaa,
	      0x00, 0xaa, 0x00,
	      0x00, 0xaa, 0xaa,
	      0xaa, 0x00, 0x00,
	      0xaa, 0x00, 0xaa,
	      0xaa, 0xaa, 0x00,
	      0xaa, 0xaa, 0xaa,
	      0x55, 0x55, 0x55,
	      0x55, 0x55, 0xff,
	      0x55, 0xff, 0x55,
	      0x55, 0xff, 0xff,
	      0xff, 0x55, 0x55,
	      0xff, 0x55, 0xff,
	      0xff, 0xff, 0x55,
	      0xff, 0xff, 0xff        
		  };
static UBYTE bwcmap[] = {0,0,0,0x3f,0x3f,0x3f,};


static
pctrunc()
{
truncated(pcx_name);
}

static char *wpcx_lines[] =
	{
	pcx_100 /* "This isn't a type of .PCX file" */,
	pcx_101 /* "Converter understands.  Possibly the" */,
	pcx_102 /* "file is damaged or a new revision." */,
	NULL,
	};

static
weird_pcx()
{
continu_box(wpcx_lines);
}

static read_palette_256(cmap)
UBYTE *cmap;
{
int i;
int c;

if (bgetbyte(&pcx_bf) != 12)
	{
	weird_pcx();
	}
i = 256*3;
while (--i >= 0)
	{
	if ((c = bgetbyte(&pcx_bf)) < 0)
		{
		pctrunc();
		return(0);
		}
	*cmap++ = (c>>2);
	}
return(1);
}

static
pcx_line()
{
int i, lpos;
UBYTE *p;
register int count;
UBYTE data;

/* first deal with any overflow from last line */
copy_bytes(over, buf, over_count);
lpos = pcxh.bpl - over_count;
p = buf + over_count;
while (lpos > 0)
	{
	if ((count = bgetbyte(&pcx_bf)) < 0)
		{
		pctrunc();
		return(0);
		}
	if ((count & 0xc0) == 0xc0)
		{
		count &= 0x3f;
		data = bgetbyte(&pcx_bf);
		lpos -= count;
		while (--count >= 0)
			*p++ = data;
		}
	else
		{
		*p++ = count;
		lpos -= 1;
		}
	}
over_count = -lpos;
return(1);
}

static
bits2_to_bytes(in, out, w, omask)
UBYTE *in, *out;
int w;
UBYTE omask;
{
int k;
UBYTE imask;
UBYTE omask2;
UBYTE c;

omask2 = omask<<1;
while (w > 0)
	{
	imask = 0x80;
	k = 4;
	if (k > w)
		k = w;
	c = *in++;
	while (--k >= 0)
		{
		if (c&imask)
			*out |= omask;
		imask >>= 1;
		if (c&imask)
			*out |= omask2;
		out += 1;
		imask >>= 1;
		}
	w -= 4;
	}
}

static
pcx_to_bap(p,omask)
UBYTE *p;
int omask;
{
if (pcxh.bitpx == 1)
	{
	bits_to_bytes(buf, p, pcw, omask);
	}
else if (pcxh.bitpx == 2)
	{
	bits2_to_bytes(buf,p,pcw, omask);
	}
else if (pcxh.bitpx == 8)
	{
	copy_bytes(buf,p,pcw);
	}
}

static
init_pcx_line()
{
over_count = 0;
if ((buf=begmem(pcxh.bpl  +  64)) == NULL)
	return(0);
over = buf + pcxh.bpl;	
return(1);
}

static
unpack_pcx()
{
int y;
UBYTE *p;
int dbpr;
int ok;
int d;
int omask;

if (!init_pcx_line())
	return(0);
y = pch;
p = pic_cel->p;
dbpr = pic_cel->bpr;
ok = 1;
while (--y >= 0)
	{
	zero_bytes(p, dbpr);
	d = pcxh.nplanes;
	omask = 1;
	while (--d >= 0)
		{
		if (!pcx_line())
			{
			ok = 0;
			goto OUT;
			}
		if (is_bitplane)
			{
			copy_bytes(buf, p, dbpr);
			}
		else
			{
			pcx_to_bap(p, omask);
			}
		omask <<= 1;
		}
	p = norm_pointer(p+dbpr);
	}
OUT:
freemem(buf);
return(ok);
}

static
trans_pal(s,d,count)
UBYTE *s, *d;
int count;
{
count *= 3;
while (--count >= 0)
	*d++ = (*s++)>>2;
}

open_pcx(name)
char *name;
{
Vcel *(*alloc_func)();

if (pcx_err != -1)
	{
	continu_line(pcx_103 /* "Already opened PCX" */);
	return(0);
	}
pcx_name = name;
if (!bopen(name, &pcx_bf))
	{
	cant_find(name);
	return(0);
	}
if (bread(&pcx_bf, &pcxh, sizeof(pcxh)) != sizeof(pcxh))
	{
	pctrunc();
	goto BADOUT;
	}
if (pcxh.encode != 1)
	{
	weird_pcx();
	goto BADOUT;
	}
switch (pcxh.version)
	{
	case 0:
		pcxh.nplanes = 1;
		with_cmap = 0;
		break;
	case 2:
		with_cmap = 1;
		break;
	case 3:
		with_cmap = 0;
		break;
	case 5:
		with_cmap = 1;
		break;
	default:
		weird_pcx();
		goto BADOUT;
	}
if (pcxh.nplanes*pcxh.bitpx == 1)
	{
	is_bitplane = 1;
	}
if (with_cmap)
	trans_pal(pcxh.palette, vf.cmap, 16);
else
	{
	if (is_bitplane)	/* one bit plane no cmap, use b&w */
		copy_bytes(bwcmap,vf.cmap,6);
	else								/* use standard vga thingie */
		trans_pal(default_pcx_cmap, vf.cmap, 16);
	}
switch (pcxh.bitpx)
	{
	case 1:
	case 2:
	case 8:
		break;
	default:
		weird_pcx();
		goto BADOUT;
	}
pcw = pcxh.x2 - pcxh.x1 + 1;
pch = pcxh.y2 - pcxh.y1 + 1;
free_cel(pic_cel);
if (is_bitplane)
	{
	if ((pic_cel = alloc_bcel(pcw,pch,pcxh.x1,pcxh.y1)) == 0)
		goto BADOUT;
	}
else
	{
	if ((pic_cel = alloc_cel(pcw,pch,pcxh.x1,pcxh.y1)) == 0)
		goto BADOUT;
	}
if (!unpack_pcx())
	goto BADOUT;
if (pcxh.bitpx == 8 && with_cmap)
	{
	if (!read_palette_256(vf.cmap))
		goto BADOUT;
	}
copy_cmap(vf.cmap, pic_cel->cmap);
see_cmap();
pcx_err = 0;
return(1);
BADOUT:
close_pcx();
return(0);
}

close_pcx()
{
free_cel(pic_cel);
pic_cel = NULL;
bclose(&pcx_bf);
pcx_err = -1;
}

count_pcx()
{
if (pcx_err)
	return(0);
return(1);
}

speed_pcx()
{
return(4);
}

start_pcx()
{
return(next_pcx());
}

next_pcx()
{
if (pcx_err)
	return(0);
tile_s_cel(pic_cel);
return(1);
}

scale_pcx(w,h)
int w,h;
{
UBYTE *msbuf;
UBYTE *mdbuf;
int i;
int ok = 0;
struct bfile outf;
int bpr;
char lbuf[40];

if (!is_bitplane)
	{
	return(scale_pic_cel(w,h));
	}
if (init_pcx_line())
	{
	if (bcreate("grey", &outf))
		{
		free_cel(pic_cel);
		pic_cel = NULL;
		if ((msbuf = begmem(pcw)) != NULL)
			{
			if ((mdbuf = begmem(w)) !=  NULL)
				{
				bseek(&pcx_bf, (long)sizeof(pcxh), SEEK_START);
				for (i=0; i<pch; i++)
					{
					if (i%10 == 0)
						{
						sprintf(lbuf, pcx_105 /* "%3d of %d" */, i, pch);
						stext(lbuf, 0, 0, sblack, swhite);
						}
					if (!pcx_line())
						goto ELOOP;
					zero_bytes(msbuf, pcw);
					bits_to_bytes(buf, msbuf, pcw, 0x3f);
					iscale(msbuf, pcw, mdbuf, w);
					if (bwrite(&outf, mdbuf, w) != w)
						goto ELOOP;
					}
				ok = 1;
		ELOOP:
				freemem(mdbuf);
				}
			freemem(msbuf);
			}
		bclose(&outf);
		}
	if (ok)
		{
		sprintf(buf, pcx_106 /* "  0 of %d" */, w);
		stext(buf, 0, 10, sblack, swhite);
		if (yscale_file("grey", w, pch, h))
			{
			if ((pic_cel = alloc_cel(w,h,0,0)) != NULL)
				{
				if (read_gulp("grey", pic_cel->p, 
					(long)pic_cel->bpr * pic_cel->h))
					{
					grey_cmap(vf.cmap);
					copy_cmap(vf.cmap, pic_cel->cmap);
					see_cmap();
					is_bitplane = 0;
					}
				}
			}
		}
	jdelete("grey");
	}
return(ok);
}

