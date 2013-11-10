
/* stpic.c - Stuff to handle Atari ST still screen files */

#include "jimk.h"
#include "crop.h"
#include "seq.h"
#include "stpic.str"

static char *stsuf[] =
	{
	".NEO", ".PI1", ".PI2", ".PI3", ".PC1", ".PC2", ".PC3",
	};
static int xres[] =
	{
	320, 320, 640, 640, 320, 640, 640,
	};
static int yres[] =
	{
	200, 200, 200, 400, 200, 200, 400,
	};
static int cres[] =
	{
	4, 4, 2, 1, 4, 2, 1,
	};
static char comps[] =
	{
	0, 0, 0, 0, 1, 1, 1,
	};
static int bprs[] =
	{
	160, 160, 160, 80, 160, 160, 80,
	};
static UWORD stmagic[] =
	{
	0, 0, 1, 2, 0x8000, 0x8001, 0x8002,
	};
static long bitoffset[] =
	{
	128, 34, 34, 34, 34, 34, 34,
	};
static long coloroffset[] =
	{
	4, 2, 2, 2, 2, 2, 2,
	};
static int sttype;
static int st_err = -1;
static char *st_name;
static struct bfile st_bf;
static int st_w, st_h, st_d, st_bpr;
static int st_wpl;
static char is_comp;

static char *sttype_lines[] = 
	{
	stpic_107 /* "Load Neochrome (.NEO)" */,
	stpic_108 /* "Load Degas Lo Res (.PI1)" */,
	stpic_109 /* "Load Degas Med Res (.PI2)" */,
	stpic_110 /* "Load Degas High Res (.PI3)" */,
	stpic_111 /* "Load Degas Elite Lo Res (.PC1)" */,
	stpic_112 /* "Load Degas Elite Med Res (.PC2)" */,
	stpic_113 /* "Load Degas Elite High Res (.PC3)" */,
	stpic_114 /* "CANCEL" */,
	};

static
sttrunc()
{
truncated(st_name);
}

static
load_st()
{
int magic;
int st_cmap[16];
char buf[50];
int i;
Vector stline;
UBYTE *p;
int bpr;

if (!bopen(st_name, &st_bf))
	return(0);
free_cel(pic_cel);
st_w = xres[sttype];
st_h = yres[sttype];
st_d = cres[sttype];
st_bpr = bprs[sttype];
is_comp = comps[sttype];
st_wpl = st_w/16;
if ((pic_cel = alloc_cel(st_w, st_h, 0, 0)) == NULL)
	return(0);
if (bread(&st_bf, &magic, 2) != 2)
	{
	sttrunc();
	return(0);
	}
intel_swap(&magic);
if (magic != stmagic[sttype])
	{
	sprintf(buf, stpic_115 /* "Not a good %s file" */, stsuf[sttype]);
	continu_line(buf);
	return(0);
	}
if (bseek(&st_bf, coloroffset[sttype], 0) < 0)
	{
	sttrunc();
	return(0);
	}
if (bread(&st_bf, st_cmap, (long)sizeof(st_cmap)) != (long)sizeof(st_cmap))
	{
	sttrunc();
	return(0);
	}
intel_swaps(st_cmap, 16);
put_st_cmap(st_cmap);
if (st_d == 1)
	mono_cmap();
copy_cmap(vf.cmap, pic_cel->cmap);
see_cmap();

if (bseek(&st_bf, bitoffset[sttype], 0) < 0)
	{
	sttrunc();
	return(0);
	}

p = pic_cel->p;
bpr = pic_cel->bpr;
i = st_h;
while (--i >= 0)
	{
	if (is_comp)
		{
		if (!st_comp_line(p))
			return(0);
		}
	else
		{
		if (!st_line(p))
			return(0);
		}
	p = norm_pointer(p+bpr);
	}
return(1);
}

static
st_comp_line(p)
UBYTE *p;
{
UBYTE lcbuf[80+128];
UBYTE omask, imask;
int i,j,k;
UBYTE *out, *in;
int bpl;
UBYTE c;

i = st_d;
omask = 1;
bpl = st_wpl<<1;
zero_structure(p, st_w);
while (--i >= 0)
	{
	if (!unpic_line(&st_bf, lcbuf, bpl, st_name))
		return(0);
	j = bpl;
	in = lcbuf;
	out = p;
	while (--j >= 0)
		{
		imask = 0x80;
		k = 8;
		c = *in++;
		while (--k >= 0)
			{
			if (c&imask)
				*out |= omask;
			out++;
			imask >>= 1;
			}
		}
	omask <<= 1;
	}
return(1);
}

static
st_line(p)
UBYTE *p;
{
UWORD lbuf[80];
UWORD *nword, *n;
UBYTE c;
int i, j, k;
UWORD mask;
UBYTE omask;

if (bread(&st_bf, lbuf, st_bpr) != st_bpr)
	{
	sttrunc();
	return(0);
	}
intel_swaps(lbuf, st_bpr/2);
nword = lbuf;
i = st_wpl;
while (--i >= 0)
	{
	mask = 0x8000;
	j = 16;
	while (--j >= 0)
		{
		c = 0;
		k = st_d;
		omask = 1;
		n = nword;
		while (--k >= 0)
			{
			if (mask & *n++)
				c |= omask;
			omask <<= 1;
			}
		*p++ = c;
		mask >>= 1;
		}
	nword += st_d;
	}
return(1);
}


qload_st()
{
int choice;
char *title;

if ((choice = qchoice(stpic_116 /* "Load an Atari ST still picture?" */,
	sttype_lines, Array_els(sttype_lines))) != 0)
	{
	choice = choice - 1;
	if ((title = get_filename(sttype_lines[choice], stsuf[choice], 0))
		!= NULL)
		{
		sttype = choice;
		load_first(title, ST);
		}
	}
}

open_st(name)
char *name;
{
if (st_err != -1)
	{
	continu_line(stpic_117 /* "Already opened ST file!" */);
	return(0);
	}
st_name = name;
if (!load_st())
	{
	close_st();
	return(0);
	}
st_err = 0;
return(1);
}

close_st()
{
free_cel(pic_cel);
pic_cel = NULL;
bclose(&st_bf);
st_err = -1;
}

start_st()
{
return(next_st());
}

next_st()
{
if (st_err)
	return(0);
tile_cel(pic_cel);
return(1);
}

speed_st()
{
return(4);
}

count_st()
{
if (st_err)
	return(0);
return(1);
}
