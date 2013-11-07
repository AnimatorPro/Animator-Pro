
/* amigapic.c - Stuff to read in an Amiga/Dpaint IFF ILBM format
   (bitplane oriented) picture. */

#include "jimk.h"
#include "jiff.h"
#include "crop.h"
#include "amigapic.str"

static struct bfile amiga_bf;
static char *amiga_name;
static int amiga_err = -1;
static int abpr;
static int masked;
static struct BitMapHeader bh;

static
decode_lplane(buf)
UBYTE *buf;
{
if (bh.compression)
	{
	if (!unpic_line(&amiga_bf, buf, abpr, amiga_name))
		return(0);
	}
else
	{
	if (bread(&amiga_bf, buf, abpr) != abpr)
		{
		amtrunc();
		return(0);
		}
	}
return(1);
}

static
decode_line(p,buf)
UBYTE *p, *buf;
{
int i;
UBYTE *out, *in;
UBYTE omask;

i = bh.nPlanes;
omask = 1;
zero_bytes(p, bh.w);
while (--i >= 0)
	{
	if (!decode_lplane(buf))
		return(0);
	bits_to_bytes(buf, p, bh.w, omask);
	omask <<= 1;
	}
if (masked)	/* skip over nasty masking plane */
	{
	if (!decode_lplane(buf))
		return(0);
	}
return(1);
}

static
decode_body()
{
int i;
UBYTE *p, *buf;
int bpr;
int ok;

p = pic_cel->p;
bpr = pic_cel->bpr;
i = pic_cel->h;
if ((buf = begmem(abpr+128)) == NULL)
	return(0);
while (--i >= 0)
	{
	ok = decode_line(p, buf);
	if (!ok)
		break;
	p = norm_pointer(p+bpr);
	}
freemem(buf);
return(ok);
}

static
decode_cmap(count, dest)
int count;
UBYTE *dest;
{
int c;

count *= 3;	/* rgb dude... */
while (--count >= 0)
	{
	if ((c = bgetbyte(&amiga_bf)) < 0)
		{
		amtrunc();
		return(0);
		}
	*dest++ = c>>2;
	}
return(1);
}

static
uncode_ilbm(size, file_pos)
long size;
long file_pos;
{
struct iff_chunk chunk;
int colorcount;
int got_head;
char buf[50];


got_head = 0;
while (size > 0)
	{
	if (bseek(&amiga_bf, file_pos, 0) < 0)
		{
		amtrunc();
		return(0);
		}
	if (bread(&amiga_bf, &chunk, sizeof(chunk)) != sizeof(chunk))
		{
		amtrunc();
		return(0);
		}
	long_intel_swap(&chunk.iff_length);
	if (bcompare(chunk.iff_type.b4_name, "BMHD",4) == 4)
		{
		if (bread(&amiga_bf, &bh, sizeof(bh)) != sizeof(bh))
			{
			amtrunc();
			return(0);
			}
		intel_swap(&bh.w);
		intel_swap(&bh.h);
		intel_swap(&bh.x);
		intel_swap(&bh.y);
		abpr = (((bh.w+15)>>4)<<1);
		if (bh.masking&1)
			masked = 1;
		else
			masked = 0;
#ifdef DEBUG
		sprintf(buf, "%d x %d   at   %d x %d   %d planes", 
			bh.w, bh.h, bh.x, bh.y, bh.nPlanes);
		continu_line(buf);
#endif /* DEBUG */
		free_cel(pic_cel);	/* just in case... */
		if ((pic_cel = alloc_cel(bh.w, bh.h, bh.x, bh.y)) == NULL)
			return(0);
		if (bh.compression > 1)
			{
			mangled(amiga_name);
			return(0);
			}
		got_head = 1;
		}
	else if (bcompare(chunk.iff_type.b4_name, "CMAP",4) == 4)
		{
		if (!got_head)
			{
			mangled(amiga_name);
			return(0);
			}
		colorcount = 1<<bh.nPlanes;
		if (colorcount > 256)
			colorcount = 256;
		if (!decode_cmap(colorcount, vf.cmap))
			return(0);
		copy_cmap(vf.cmap, pic_cel->cmap);
		see_cmap();
		}
	else if (bcompare(chunk.iff_type.b4_name, "BODY",4) == 4)
		{
		if (!got_head)
			{
			mangled(amiga_name);
			return(0);
			}
		if (!decode_body())
			return(0);
		return(1);	/* got body */
		}
	if (chunk.iff_length & 1)
		{
		chunk.iff_length += 1;
		}
	chunk.iff_length += sizeof(chunk);
	size -= chunk.iff_length;
	file_pos += chunk.iff_length;
	}
continu_line(amigapic_103 /* "No body - a color file perhaps?" */);
return(0);
}

static
load_amiga()
{
struct form_chunk fc;

if (!bopen(amiga_name, &amiga_bf))
	{
	cant_find(amiga_name);
	return(0);
	}
if (bread(&amiga_bf, &fc, sizeof(fc)) != sizeof(fc) )
	{
	amtrunc();
	return(0);
	}
if (bcompare("FORM", fc.fc_type.b4_name, 4) != 4)
	{
	not_ilbm();
	return(0);
	}
if (bcompare("ILBM", fc.fc_subtype.b4_name, 4) != 4)
	{
	not_ilbm();
	return(0);
	}
long_intel_swap(&fc.fc_length);
if (!uncode_ilbm(fc.fc_length, (long)sizeof(fc)))
	return(0);
return(1);
}

static
not_ilbm()
{
continu_line(amigapic_106 /* "File isn't an IFF ILBM." */);
}

static
amtrunc()
{
truncated(amiga_name);
}


open_amiga(name)
char *name;
{
if (amiga_err != -1)
	{
	continu_line(amigapic_107 /* "Amiga file already open" */);
	return(0);
	}
amiga_name = name;
if (!load_amiga())
	{
	close_amiga();
	return(0);
	}
amiga_err = 0;
return(1);
}

start_amiga()
{
return(next_amiga());
}

next_amiga()
{
if (amiga_err)
	return(0);
tile_cel(pic_cel);
return(1);
}

count_amiga()
{
if (amiga_err)
	return(0);
return(1);
}

speed_amiga()
{
return(4);
}

close_amiga()
{
free_cel(pic_cel);
pic_cel = NULL;
bclose(&amiga_bf);
amiga_err = -1;
}

