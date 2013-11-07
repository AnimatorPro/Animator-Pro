
/* scale.c - Do a nice pixel-averaging scale-up/scale-down */

#include "jimk.h"
#include "crop.h"
#include "scale.str"

#define WHOLESCALE 256

struct bfile rgb_files[3];
char *rgb_names[3] = 
	{
	"red",
	"green",
	"blue",
	};

kill_rgb_files()
{
int i;
for (i=0;i<3;i++)
	jdelete(rgb_names[i]);
}

open_rgb_files()
{
int i;
for (i=0; i<3; i++)
	{
	if (!bopen(rgb_names[i], rgb_files+i))
		{
		cant_find(rgb_names[i]);
		close_rgb_files();
		return(0);
		}
	}
return(1);
}

create_rgb_files()
{
int i;
for (i=0; i<3; i++)
	{
	if (!bcreate(rgb_names[i], rgb_files+i))
		{
		cant_create(rgb_names[i]);
		close_rgb_files();
		return(0);
		}
	}
return(1);
}

close_rgb_files()
{
int i;

for (i=0; i<3; i++)
	bclose(rgb_files+i);
}

iscale(s, sct, d, dct)
UBYTE *s, *d;
int sct, dct;
{
if (sct > dct)	/* going to do some averaging */
	{
	int i;
	int j, jend, lj;
	long lasts, ldiv;
	long acc, div;
	long t1,t2;

	ldiv = WHOLESCALE;
	lasts = s[0];
	lj = 0;
	for (i=0; i<dct; i++)
		{
		acc = lasts*ldiv;
		div = ldiv;
		t1 = (i+1)*(long)sct;
		jend = t1/dct;
		for (j = lj+1; j<jend; j++)
			{
			acc += s[j]*WHOLESCALE;
			div += WHOLESCALE;
			}
		t2 = t1 - jend*(long)dct;
		lj = jend;
		lasts = s[lj];
		if (t2 == 0)
			{
			ldiv = WHOLESCALE;
			}
		else
			{
			ldiv = WHOLESCALE*t2/dct;
			div += ldiv;
			acc += lasts*ldiv;
			ldiv = WHOLESCALE-ldiv;
			}
		*d++ = acc/div;
		}
	}
else if (dct == sct)	/* they's the same */
	{
	while (--dct >= 0)
		*d++ = *s++;
	}
else if (sct == 1)
	{
	while (--dct >= 0)
		*d++ = *s;
	}
else/* going to do some interpolation */
	{
	int i;
	long t1;
	long p1;
	long err;
	int dct2;

	dct -= 1;
	sct -= 1;
	dct2 = dct/2;
	t1 = 0;
	for (i=0; i<=dct; i++)
		{
		p1 = t1/dct;
		err =  t1 - p1*dct;
		if (err == 0)
			*d++ = s[p1];
		else
			*d++ = (s[p1]*(dct-err)+s[p1+1]*err+dct2)/dct;
		t1 += sct;
		}
	}
}

static line_to_3files(l, size, rgb, y)
UBYTE *l;
int size;
int rgb, y;
{
int i;

if (rgb == 0 && y%10 == 0)
	{
	char buf[30];
	sprintf(buf, "%3d", y);
	stext(buf, 0, 10, sblack, swhite);
	}
if (bwrite(rgb_files + rgb, l, size) != size)
	{
	truncated(rgb_names[rgb]);
	return(0);
	}
return(1);
}

/* basically look up a component of in into out through red component of
	color map */
vga_to_red(in, out, w, cmap)
UBYTE *in;
UBYTE *out;
int w;
UBYTE *cmap;
{
while (--w >= 0)
	*out++ = cmap[3 * *in++];
}

get_column(s, d, w, h)
UBYTE *s, *d;
int w, h;
{
while (--h >= 0)
	{
	*d++ = *s;
	s = norm_pointer(s+w);
	}
}

put_column(s, d, w, h)
UBYTE *s, *d;
int w, h;
{
while (--h >= 0)
	{
	*d = *s++;
	d = norm_pointer(d+w);
	}
}

/* interpolate scale a byte-plane in memory in y dimension */
scaley(inbytes, outbytes, inline, outline, w, oh, nh)
UBYTE *inbytes, *outbytes;	/* image and a place to put scaled copy */
UBYTE *inline, *outline;	/* buffers size oh and nh... */
int w, oh, nh;
{
int i;
char buf[50];

for (i=0; i<w; i++)
	{
	get_column(inbytes++, inline, w, oh);
	iscale(inline, oh, outline, nh);
	put_column(outline, outbytes++, w, nh);
	if (i%10 == 0)
		{
		sprintf(buf, "%3d", i);
		stext(buf, 0, 10, sblack,  swhite);
		}
	}
}

grey_cmap(c)
UBYTE *c;
{
int i;
int j;

for (i=0; i<64; i++)
	{
	j = 1;
	while (--j >= 0)
		{
		*c++ = i;
		*c++ = i;
		*c++ = i;
		}
	}
}

yscale_file(name, w, oh, nh)
char *name;
int w, oh, nh;
{
UBYTE *inbytes;
UBYTE *outbytes;
Vcel *outcel;
struct bfile bf;
int ok = 0;
long ibsize;
UBYTE *inline, *outline;

if (oh == nh)
	return(1);
ibsize = w * (long)oh;
if ((inbytes = lbegmem(ibsize)) == NULL)
	return(0);
if ((outcel = alloc_cel(w,nh,0,0)) == NULL)
	{
	freemem(inbytes);
	return(0);
	}
outbytes = outcel->p;
if ((inline = begmem(oh)) != NULL)
	{
	if ((outline = begmem(nh)) != NULL)
		{
		if (read_gulp(name, inbytes, ibsize))
			{
			scaley(inbytes, outbytes, inline, outline, w, oh, nh);
			ok = write_gulp(name, outbytes, w * (long)nh);
			tile_cel(outcel);
			}
		freemem(outline);
		}
	freemem(inline);
	}
freemem(inbytes);
free_cel(outcel);
return(ok);
}

scale_xdim_cel(cel, new_w, lineout)
Vcel *cel;
int new_w;
Vector lineout;
{
int h;
UBYTE *line_in;
UBYTE *rgb_in;
UBYTE *rgb_out;
int i,j;
int ok = 0;

if ((rgb_in = begmem(cel->w)) == NULL)
	return(0);
if ((rgb_out = begmem(new_w)) == NULL)
	{
	freemem(rgb_in);
	return(0);
	}
line_in = cel->p;
h = cel->h;
for (j=0; j<h; j++)
	{
	for (i=0; i<3; i++)
		{
		vga_to_red(line_in, rgb_in, cel->w, cel->cmap+i);
		iscale(rgb_in, cel->w, rgb_out, new_w);
		if (!(*lineout)(rgb_out, new_w, i, j))
			goto OUT;
		}
	ok = 1;
	line_in = norm_pointer(line_in + cel->bpr);
	}
OUT:
freemem(rgb_in);
freemem(rgb_out);
return(ok);
}

scale_rgb_files(h, new_w, new_h)
int h, new_w, new_h;
{
char buf[40];
int i;
int ok;

/* now scale the ram-disk files y-wise */
grey_cmap(vf.cmap);
find_colors();
see_cmap();
for (i=0; i<3; i++)
	{
	sprintf(buf, scale_105 /* "Y-scale %s component " */, rgb_names[i]);
	stext(buf, 0, 0, sblack,  swhite);
	sprintf(buf, scale_108 /* "%3d of %d" */, 0, new_w);
	stext(buf, 0, 10, sblack,  swhite);
	if (!yscale_file(rgb_names[i], new_w, h, new_h))
		return(0);
	}

/* now figure out the color map and load cel with that color map from
   the rgb files */
if (!find_rgb_cmap(new_w, new_h, vf.cmap))
	return(0);
if (!open_rgb_files())
	return(0);
if (!make_bhash())
	return(0);
ok = cel_with_cmap(new_w, new_h);
free_bhash();
close_rgb_files();
if (!ok)
	{
	free_cel(pic_cel);
	pic_cel = NULL;
	return(0);
	}
see_cmap();
copy_cmap(vf.cmap, pic_cel->cmap);
return(1);
}

static
s_pic_cel(new_w, new_h)
int new_w, new_h;
{
int h;
int i, ok;
char buf[120];

if (create_rgb_files())
	{
	/* first scale it horizontally into 3 files in the ram-disk */
	h = pic_cel->h;
	sprintf(buf, scale_107 /* "X - scaling line" */, 0, h);
	stext(buf, 0, 00, sblack,  swhite);
	sprintf(buf, scale_108 /* "%3d of %d" */, 0, h);
	stext(buf, 0, 10, sblack,  swhite);
	ok = scale_xdim_cel(pic_cel, new_w, line_to_3files);
	close_rgb_files();
	if (!ok)
		return(0);
	free_cel(pic_cel);
	pic_cel = NULL;
	if (!scale_rgb_files(h, new_w, new_h))
		return(0);
	tile_cel(pic_cel);
	}
return(1);
}

scale_pic_cel(w,h)
int w, h;
{
int ok;

ok = s_pic_cel(w,h);
kill_rgb_files();
return(ok);
}


static int scalew = XMAX, scaleh = YMAX;
extern int dither;

qscale()
{
char *bufs[8];
char b1[30], b2[30], b3[30], b4[30];
int choice;
long r1,r2;
int ouzx,ouzy;	/* bad kludge to keep menus from moving... */

ouzx = uzx;
ouzy = uzy;
for (;;)
	{
	uzx = ouzx;
	uzy = ouzy;
	/* make up strings for menu */
	sprintf(b1, scale_109 /* "Set Width        %3d" */, scalew);
	bufs[0] = b1;
	sprintf(b2, scale_110 /* "Set Height       %3d" */, scaleh);
	bufs[1] = b2;
	bufs[2] =   scale_111 /* "Default      320x200" */;
	bufs[3] =   scale_112 /* "Correct Aspect Ratio" */;
	sprintf(b3, 
		scale_113 /* "Revert       %3dx%3d" */, pic_cel->w, pic_cel->h);
	bufs[4] =  b3;
	sprintf(b4, scale_114 /* "%s Dither" */, dither ? "*" : " ");
	bufs[5] = b4;
	bufs[6] =   scale_117 /* "Render" */;
	bufs[7] =   scale_118 /* "Exit Menu" */;
	if ((choice = qchoice(scale_119 /* "Scale..." */, 
		bufs, Array_els(bufs))) == 0)
		break;
	switch (choice)
		{
		case 1:
			qreq_number(scale_120 /* "Set destination width" */, 
				&scalew, 1, pic_cel->w);
			break;
		case 2:
			qreq_number(scale_121 /* "Set destination height" */, 
				&scaleh, 1, pic_cel->h);
			break;
		case 3:
			scalew = XMAX;
			scaleh = YMAX;
			break;
		case 4:
			r1 = (long)XMAX*pic_cel->h;
			r2 = (long)YMAX*pic_cel->w;
			if (r1 > r2)	/* need to shrink y */
				{
				scalew = pic_cel->w;
				scaleh = (long)(pic_cel->h * r2 / r1);
				}
			else			/* need to shrink x */
				{
				scalew = (long)(pic_cel->w * r1 / r2);
				scaleh = pic_cel->h;
				}
			break;
		case 5:
			scalew = pic_cel->w;
			scaleh = pic_cel->h;
			break;
		case 6:
			dither = !dither;
			break;
		case 7:
			do_scale();
			break;
		}
	if (scalew <= 0)
		scalew = 1;
	if (scaleh <= 0)
		scaleh = 1;
	}
}

do_scale()
{
long l;
extern unsigned mem_free;

/* make sure will have enough memory before go spend 1/2 hour doing it... */
l = scalew;
l *= scaleh;
l += 1000;
if (pic_cel)
	l -= (long)pic_cel->w * pic_cel->h;
if (l >= mem_free*16L)
	{
	outta_memory();
	return;
	}

switch (intype)
	{
	case MAC:
		scale_mac(scalew,scaleh);
		break;
	case PCX:
		scale_pcx(scalew,scaleh);
		break;
	case TARGA:
		scale_vision(scalew,scaleh);
		kill_rgb_files();
		break;
	default:
		scale_pic_cel(scalew, scaleh);
		break;
	}
}

