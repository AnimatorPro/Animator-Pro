
/* crop.c - Relatively high level file.  Sits right under main() pretty
   much.  Defines the main crop methods:
   	      open_xxx	- open an image and verify its right type of image
		  close_xxx	- close up image
		  start_xxx	- get ready to display the 1st frame
		  next_xxx  - display next frame
		  speed_xxx - how fast does it go?
		  count_xxx - how many frames?
   There's a bunch of big switches that direct queries to next_xxx, speed_xxx
   etc to the appropriate code in the file associated with a particular
   image type.   Then towards the end of this file are some routines common
   to more than one image type.  */

#include "jimk.h"
#include "fli.h"
#include "jiff.h"
#include "vcomp.h"
#include "crop.h"
#include "crop.str"

int intype;
char is_bitplane;

struct vcel *pic_cel;

is_still(type)
int type;
{
switch (type)
	{
	case NONE:
	case RIF:
	case SEQ:
	case FLI:
	case ANIM:
		return(0);
	case GIF:
	case TARGA:
	case MAC:
	case ST:
	case AMIGA:
	case PCX:
		return(1);
	}
}


open_in(name, type)
char *name;
int type;
{
int ok;

is_bitplane = 0;	/* only MAC and some PCX are bitplane... */
close_in();
switch (type)
	{
	case PCX:
		ok = open_pcx(name);
		break;
	case AMIGA:
		ok = open_amiga(name);
		break;
	case ST:
		ok = open_st(name);
		break;
	case MAC:
		ok = open_mac(name);
		break;
	case TARGA:
		ok = open_vision(name);
		break;
	case GIF:
		ok = open_gif(name);
		break;
	case ANIM:
		ok = open_anim(name);
		break;
	case RIF:
		ok = open_rif(name);
		break;
	case FLI:
		ok = open_fli(name);
		break;
	case SEQ:
		ok = open_seq(name);
		break;
	default:
		ok = 0;
		break;
	}
if (ok)
	intype = type;
return(ok);
}

start_in()
{
int ok;

switch (intype)
	{
	case PCX:
		ok = start_pcx();
		break;
	case AMIGA:
		ok = start_amiga();
		break;
	case ST:
		ok = start_st();
		break;
	case MAC:
		ok = start_mac();
		break;
	case TARGA:
		ok = start_vision();
		break;
	case GIF:
		ok = start_gif();
		break;
	case ANIM:
		ok = start_anim();
		break;
	case RIF:
		ok = start_rif();
		break;
	case FLI:
		ok = start_fli();
		break;
	case SEQ:
		ok = start_seq();
		break;
	default:
		ok = 0;
		break;
	}
return(ok);
}

next_in()
{
int ok;

switch (intype)
	{
	case PCX:
		ok = next_pcx();
		break;
	case AMIGA:
		ok = next_amiga();
		break;
	case ST:
		ok = next_st();
		break;
	case MAC:
		ok = next_mac();
		break;
	case TARGA:
		ok = next_vision();
		break;
	case GIF:
		ok = next_gif();
		break;
	case RIF:
		ok = next_rif();
		break;
	case ANIM:
		ok = next_anim();
		break;
	case FLI:
		ok = next_fli();
		break;
	case SEQ:
		ok = next_seq();
		break;
	default:
		ok = 0;
		break;
	}
return(ok);
}



speed_in()
{
int ok;

switch (intype)
	{
	case PCX:
		ok = speed_pcx();
		break;
	case AMIGA:
		ok = speed_amiga();
		break;
	case ST:
		ok = speed_st();
		break;
	case MAC:
		ok = speed_mac();
		break;
	case TARGA:
		ok = speed_vision();
		break;
	case GIF:
		ok = speed_gif();
		break;
	case ANIM:
		ok = speed_anim();
		break;
	case RIF:
		ok = speed_rif();
		break;
	case FLI:
		ok = speed_fli();
		break;
	case SEQ:
		ok = speed_seq();
		break;
	default:
		ok = 0;
		break;
	}
return(ok);
}

count_in()
{
int ok;

switch (intype)
	{
	case PCX:
		ok = count_pcx();
		break;
	case AMIGA:
		ok = count_amiga();
		break;
	case ST:
		ok = count_st();
		break;
	case MAC:
		ok = count_mac();
		break;
	case TARGA:
		ok = count_vision();
		break;
	case GIF:
		ok = count_gif();
		break;
	case ANIM:
		ok = count_anim();
		break;
	case RIF:
		ok = count_rif();
		break;
	case FLI:
		ok = count_fli();
		break;
	case SEQ:
		ok = count_seq();
		break;
	default:
		ok = 0;
		break;
	}
return(ok);
}

close_in()
{
clear_form(&vf);
switch (intype)
	{
	case PCX:
		close_pcx();
		break;
	case AMIGA:
		close_amiga();
		break;
	case ST:
		close_st();
		break;
	case MAC:
		close_mac();
		break;
	case TARGA:
		close_vision();
		break;
	case GIF:
		close_gif();
		break;
	case RIF:
		close_rif();
		break;
	case ANIM:
		close_anim();
		break;
	case FLI:
		close_fli();
		break;
	case SEQ:
		close_seq();
		break;
	}
intype = NONE;
}



load_first(name, type)
char *name;
int type;
{
if (!jexists(name))
	{
	cant_find(name);
	return;
	}
if (open_in(name, type))
	{
	if (!start_in())
		close_in();
	}
}

view_in()
{
int count, i;

count = count_in();
hide_mouse();

for (;;)
	{
	c_input();
	if (key_hit || RJSTDN)
		goto OUT;
	if (start_in())
		{
		for (i=1; i<count; i++)
			{
			c_input();
			if (key_hit || RJSTDN)
				goto OUT;
			if (!next_in())
				goto OUT;
			}
		}
	}
OUT:
show_mouse();
}

qload_vision()
{
char *title;

if ((title = get_filename(crop_100 /* "Load TARGA compatible file?" */, 
	".TGA", 0)) == NULL)
	return;
load_first(title, TARGA);
}

qload_pc()
{
char *title;

if ((title = get_filename(crop_102 /* "Load PC Paintbrush compatible file?" */,
	".PCX", 0))
	== NULL)
	return;
load_first(title, PCX);
}

qload_gif()
{
char *title;

if ((title = get_filename(crop_104 /* "Load GIF file?" */,
	".GIF", 0)) == NULL)
	return;
load_first(title, GIF);
}

qsave_gif()
{
char *title;

if ((title = get_filename(crop_106 /* "Save GIF file" */, 
	".GIF", 1)) == NULL)
	return;
save_gif(title, &vf);
}


qload_rif()
{
char *title;

if ((title = get_filename(crop_108 /* "Load Amiga .RIF (Zoetrope) file?" */,
	".RIF", 0)) == NULL)
	return;
load_first(title, RIF);
}

qload_anim()
{
char *title;

if ((title = get_filename(crop_110 /* "Load Amiga .ANIM file?" */,
	".ANI", 0)) == NULL)
	return;
load_first(title, ANIM);
}




qload_fli()
{
char *title;

if ((title = get_filename(crop_112 /* "Load Autodesk Animator FLIC?" */,
	".FLI", 0)) == NULL)
	return;
load_first(title, FLI);
}

qload_mac()
{
char *title;

if ((title = get_filename(crop_114 /* "Load MacPaint format picture?" */,
	".MAC", 0))
	== NULL)
	return;
load_first(title, MAC);
}

qload_amiga()
{
char *title;

if ((title = get_filename(
	crop_116 /* "Load Amiga/Dpaint format IFF ILBM Picture?" */, 
	".LBM", 0)) == NULL)
	return;
load_first(title, AMIGA);
}

qload_seq()
{
char *title;

if ((title = get_filename(crop_118 /* "Load Atari ST CYBER SEQ file?" */,
	".SEQ", 0)) == NULL)
	return;
load_first(title, SEQ);
}



s_fli(name, si, iv, count, speed)
char *name;
Vector si, iv;
int count, speed;
{
int i, ok;
void *cbuf;
Video_form *lasts;

ok = 0;
lasts = NULL;
if ((*si)())
	{
	if ((lasts = alloc_screen()) == NULL)
		goto OUT;
	copy_form(&vf, lasts);
	/* alloc and free cbuf lots so 'next_in' routine has as much memory
	   as possible */
	if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)	
		goto OUT;
	if (fli_first_frame(cbuf,name,NULL,NULL,vf.p,vf.cmap,speed))
		{
		for (i=1; i<count; i++)
			{
			freemem(cbuf);
			if (!(*iv)())
				/* let them keep what they got if input short */
				{
				break;
				}
			if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)
				goto OUT;
			if (!fli_next_frame(cbuf,lasts->p, 
				lasts->cmap,vf.p,vf.cmap))
				/* file gets deleted if output short */
				{
				freemem(cbuf);
				goto OUT;
				}
			copy_form(&vf, lasts);
			}
		freemem(cbuf);
		if (!(*si)())
			goto OUT;
		if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)
			goto OUT;
		ok = fli_last_frame(cbuf,lasts->p, lasts->cmap, 
			vf.p, vf.cmap);
		}
	freemem(cbuf);
	}
OUT:
free_screen(lasts);
return(ok);
}


qsave_fli()
{
char *title;
int frames, i;

if ((title = get_filename(crop_120 /* "Save FLIC for Autodesk Animator?" */, 
	".FLI", 1))
	== NULL)
	return;
if (!save_fli(title))
	jdelete(title);
}

save_fli(name)
char *name;
{
return(s_fli(name, start_in, next_in, count_in(), speed_in()));
}

#ifdef SLUFFED
static Point qsl1, qsl2;
#endif /* SLUFFED */
extern Vcel *pic_cel;

tile_s_cel(cel)
Vcel *cel;
{
if (is_bitplane)
	tile_bit_cel(cel);
else
	tile_cel(cel);
}

qmove()
{
for (;;)
	{
	wait_click();
	if (PJSTDN)
		{
		move_cel(pic_cel);
		if (!PJSTDN)
			break;
		}
	else
		break;
	}
}


mono_cmap()
{
vf.cmap[0] = vf.cmap[1] = vf.cmap[2] = 63;
vf.cmap[3] = vf.cmap[4] = vf.cmap[5] = 0;
}

bits_to_bytes(in, out, w, omask)
UBYTE *in, *out;
int w;
UBYTE omask;
{
int k;
UBYTE imask;
UBYTE c;

while (w > 0)
	{
	imask = 0x80;
	k = 8;
	if (k > w)
		k = w;
	c = *in++;
	while (--k >= 0)
		{
		if (c&imask)
			*out |= omask;
		out += 1;
		imask >>= 1;
		}
	w -= 8;
	}
}


/* unpic_line() - read in a single mac pic (or amiga ILBM) line 
   from a buffered file and decompress it into buf argument.  Decompressed
   length is len bytes.  Pass in file name for error reporting. */
unpic_line(bf, buf, len, name)
struct bfile *bf;
UBYTE *buf;
int len;
char *name;
{
UBYTE *p;
char b;
UBYTE d;
int count;

/* first uncompress it into the buffer mon */
p = buf;
while (len > 0)
	{
	b = bgetbyte(bf);
	if (b < 0)	/* it's a run */
		{
		d = bgetbyte(bf);
		count = -b;
		count += 1;
		len -= count;
		while (--count >= 0)
			*p++ = d;
		}
	else
		{
		count = b;
		count += 1;
		if (bread(bf, p, count) != count)
			{
			truncated(name);
			return(0);
			}
		p += count;
		len -= count;
		}
	}
return(1);
}

intel_swaps(x, length)
register char *x;
int length;	/* WORDS */
{
char swap;

while (--length >= 0)
	{
	swap = x[0];
	x[0] = x[1];
	x[1] = swap;
	x += 2;
	}
}

intel_swap(x)
register char *x;
{
char swap;

swap = x[0];
x[0] = x[1];
x[1] = swap;
}

long_intel_swap(x)
register int *x;
{
int swap;

swap = x[0];
x[0] = x[1];
x[1] = swap;
intel_swap((char *)x);
intel_swap((char *)(x+1));
}

/* fake amiga... */
put_st_cmap(cmap)
UWORD *cmap;
{
int i;
UBYTE *ct;
UWORD cm;

ct = vf.cmap;
for (i=0; i<16; i++)
	{
	cm = *cmap++;
	*ct++ = ((cm&0x700)>>5);
	*ct++ = ((cm&0x070)>>1);
	*ct++ = ((cm&0x007)<<3);
	}
wait_sync();
jset_colors(0, 16, sys_cmap);
}

