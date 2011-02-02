
/* macpic.c - Go cope with a MacPaint Pic.  It's a hi-res goodie.   We'll
   probably keep it as a bit-plane until someone wants to scale it. */

#include "jimk.h"
#include "crop.h"
#include "macpic.str"


/* fixed width/height of a Macpaint pic */
#define MW 576
#define MH 720	
#define MBPR ((MW+7)/8)

static int mac_err = -1;
static struct bfile mac_bf;
static char *mac_name;

static
truncm()
{
truncated(mac_name);
}


static
mac_line(dest)
UBYTE *dest;
{
UBYTE buf[MBPR+128];	/* 128 so don't crash on bad data */

if (!unpic_line(&mac_bf, buf, MBPR, mac_name))
	return(0);
copy_bytes(buf, dest, MBPR);
return(1);
}

static
load_mac()
{
UBYTE *p;
int bpr;
int i;

if (bseek(&mac_bf, 512L, 0) < 0) /* go to just past header */
	{
	truncm();
	return(0);
	}
if ((pic_cel = alloc_bcel(MW,MH,0,0)) == NULL)
	return(0);
p = pic_cel->p;
bpr = pic_cel->bpr;
i = MH;
while (--i >= 0)
	{
	if (!mac_line(p))
		{
		return(0);
		}
	p = norm_pointer(p+bpr);
	}
return(1);
}

open_mac(name)
char *name;
{
if (mac_err != -1)
	{
	continu_line(macpic_100 /* "Mac file already open" */);
	return(0);
	}
mac_name = name;
if (!bopen(name, &mac_bf))
	{
	cant_find(name);
	return(0);
	}
if (!load_mac())
	{
	close_mac();
	return(0);
	}
mono_cmap();
see_cmap();
copy_cmap(vf.cmap, pic_cel->cmap);
mac_err = 0;
is_bitplane = 1;
return(1);
}

close_mac()
{
free_cel(pic_cel);
pic_cel = NULL;
mac_err = -1;
}


start_mac()
{
return(next_mac());
}

speed_mac()
{
return(4);
}

count_mac()
{
if (mac_err)
	return(0);
return(1);
}

next_mac()
{
if (mac_err)
	return(0);
tile_bit_cel(pic_cel);
return(1);
}

scale_mac(w,h)
int w,h;
{
UBYTE *msbuf;
UBYTE *mdbuf;
UBYTE bitbuf[MBPR];
int i;
int ok = 0;
struct bfile outf;
char buf[40];

if (bcreate("grey", &outf))
	{
	free_cel(pic_cel);
	pic_cel = NULL;
	if ((msbuf = begmem(MW)) != NULL)
		{
		if ((mdbuf = begmem(w)) !=  NULL)
			{
			bseek(&mac_bf, 512L, 0);
			for (i=0; i<MH; i++)
				{
				if (i%10 == 0)
					{
					sprintf(buf, macpic_102 /* "%3d of 720" */, i);
					stext(buf, 0, 0, sblack, swhite);
					}
				if (!mac_line(bitbuf))
					goto ELOOP;
				zero_bytes(msbuf, MW);
				bits_to_bytes(bitbuf, msbuf, MW, 0x3f);
				iscale(msbuf, MW, mdbuf, w);
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
	sprintf(buf, macpic_103 /* "  0 of %d" */, w);
	stext(buf, 0, 10, sblack, swhite);
	if (yscale_file("grey", w, MH, h))
		{
		if ((pic_cel = alloc_cel(w,h,0,0)) != NULL)
			{
			if (read_gulp("grey", pic_cel->p, (long)pic_cel->bpr * pic_cel->h))
				{
				igrey_cmap(vf.cmap);
				copy_cmap(vf.cmap, pic_cel->cmap);
				see_cmap();
				is_bitplane = 0;
				}
			}
		}
	}
jdelete("grey");
return(ok);
}

/* make a grey scale color map */
igrey_cmap(c)
UBYTE *c;
{
int i;

i = 64;

while (--i >=0)
	{
	*c++ = i;
	*c++ = i;
	*c++ = i;
	}
}

