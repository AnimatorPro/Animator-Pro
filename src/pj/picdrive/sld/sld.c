
/* sld.c - source code for the Pic-driver to read AutoCAD Slide files. */

#define REXLIB_INTERNALS
#include "errcodes.h"
#include "stdtypes.h"
#include "stdio.h"
#include "picdrive.h"
#include "syslib.h"
#include "sld.h"


Errcode fill_poly_inside(Poly *pl, EFUNC hline, void *hldat);

typedef struct sld_head_9
/* Release 9 headers. */
	{
	char id_string[17];
	UBYTE type;
	UBYTE level;
	USHORT width;
	USHORT height;
	ULONG aspect;
	USHORT hard_fill;
	USHORT test_iswap;
	} Sld_head_9;
typedef struct sld_head_8
/* Pre-release 9 headers. */
	{
	char id_string[17];
	UBYTE type;
	UBYTE level;
	USHORT width;
	USHORT height;
	double aspect;
	USHORT hard_fill;
	UBYTE pad;
	} Sld_head_8;
typedef struct sld_head_0
/* Common part of old and new headers. */
	{
	char id_string[17];
	UBYTE type;
	UBYTE level;
	USHORT width;
	USHORT height;
	} Sld_head_0;
typedef union sld_union
/* Union to deal with differing headers at differing resolutions. */
	{
	Sld_head_0 common;
	Sld_head_8 old;
	Sld_head_9 new;
	} Sld_union;
typedef struct 
	{
	Sld_union u;
	long end_off;		/* file offset of just after header. */
	Boolean is_swapped;	/* Is data in Motorola format? */
	} Sld_head;

typedef struct ifile {
/** This structure starts with and Image_file, and has further data
 ** local to the PDR.  
 **/
	Image_file hdr;
	FILE *file;
	Sld_head sld;
} Ifile;


#define copy_bytes(source,dest,count) memcpy(dest,source,count)
#define clear_mem(dest,count) memset(dest,0,count)
#define clear_struct(s) clear_mem(s, sizeof(s))


static void intel_swaps(void *p, int length)
/*****************************************************************************
 * Convert an array of 16 bit quantitys between 68000 and intel format.
 ****************************************************************************/
{
register char *x = p;
char swap;

while (--length >= 0)
	{
	swap = x[0];
	x[0] = x[1];
	x[1] = swap;
	x += 2;
	}
}


static Boolean spec_best_fit(Anim_info *ainfo)
/*****************************************************************************
 * Tell host that we can only write 1 frame 8 bit-a-pixel images.  We can
 * accept any resolution though since we're a vector file.
 ****************************************************************************/
{
Boolean nofit;

nofit = (ainfo->depth == 8
		 && ainfo->num_frames == 1);
ainfo->depth = 8;
ainfo->num_frames = 1;
ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
return(nofit);	/* return whether fit was exact */
}

void get_signature(char *dest)
{
sprintf(dest, "AutoCAD Slide%c%c%c", '\r', '\n', 26 /* control Z */ );
}

static Errcode read_head(Sld_head *s, FILE *f)
{
Errcode err = Success;
char signature[17];


get_signature(signature);
if (fread(&s->u.new, sizeof(s->u.new), 1, f) != 1)
	{
	err = Err_truncated;
	goto ERROR;
	}
if (memcmp(s->u.common.id_string, signature, sizeof(signature)) != 0 
	|| s->u.common.type != 0x56)
	{
	err = Err_bad_magic;
	goto ERROR;
	}
if (s->u.common.level == 1)		/* process old version */
	{
	rewind(f);
	if (fread(&s->u.old, sizeof(s->u.old), 1, f) != 1)
		{
		err = Err_truncated;
		goto ERROR;
		}
	s->end_off = sizeof(s->u.old);
	}
else if (s->u.common.level == 2)		/* do current version */
	{
	s->end_off = sizeof(s->u.new);
	s->is_swapped = (s->u.new.test_iswap != 0x1234);
	}
else
	{
	err = Err_version;
	goto ERROR;
	}
if (s->is_swapped)
	{
	intel_swaps(&s->u.common.width, 2);
	}
/* printf("Image is %dx%d\n", s->u.common.width, s->u.common.height); */
ERROR:
return(err);
}

static Errcode open_file(Pdr *pd, char *path, Image_file **pif, 
						 Anim_info *ainfo )
/*****************************************************************************
 * Open up the file, and if possible verify header. 
 ****************************************************************************/
{
Ifile *gf;
FILE *f;
Errcode err = Success;

*pif = NULL;	/* for better error recovery */

if((gf = zalloc(sizeof(*gf))) == NULL)
	return(Err_no_memory);

if((f = gf->file = fopen(path, "rb")) == NULL)
	{
	err = pj_errno_errcode();
	goto ERROR;
	}
if ((err = read_head(&gf->sld, gf->file)) < Success)
	goto ERROR;
if (ainfo != NULL)		/* fill in ainfo structure if any */
	{
	clear_struct(ainfo);
	spec_best_fit(ainfo);
	ainfo->width = gf->sld.u.common.width + 1;
	ainfo->height = gf->sld.u.common.height + 1;
	}
*pif = (Image_file *)gf;
return(Success);

ERROR:
close_file(&gf);
return(err);
}

static close_file(Ifile **pgf)
/*****************************************************************************
 * Clean up resources used by picture driver in loading (saving) a file.
 ****************************************************************************/
{
Ifile *gf;

if(pgf == NULL || (gf = *pgf) == NULL)
	return;
if(gf->file)
	fclose(gf->file);
free(gf);
*pgf = NULL;
}



static void draw_line(Rcel *screen, short x1, short y1, short x2, short y2, 
	short color)
/* Draw a line. */
{
register SHORT   duty_cycle;
SHORT incy;
register SHORT delta_x, delta_y;
register SHORT dots;

	delta_y = y2-y1;
	delta_x = x2-x1;
	if (delta_y < 0) 
	{
		delta_y = -delta_y;
		incy = -1;
	}
	else
	{
		incy = 1;
	}
	if ((delta_x) < 0) 
	{
		delta_x = -delta_x;
		incy = -incy;
		x1 = x2;
		y1 = y2;
	}
	duty_cycle = (delta_x - delta_y)/2;

	if (delta_x >= delta_y)
	{
		dots = ++delta_x;
		while (--dots >= 0)
		{
			pj_put_dot(screen, color, x1,y1);
			duty_cycle -= delta_y;
			++x1;
			if (duty_cycle < 0)
			{
				duty_cycle += delta_x;	  /* update duty cycle */
				y1+=incy;
			}
		}
	}
	else /* dy > dx */
	{
		dots = ++delta_y;
		while (--dots >= 0)
		{
			pj_put_dot(screen, color, x1,y1);
			duty_cycle += delta_x;
			y1+=incy;
			if (duty_cycle > 0)
			{
				duty_cycle -= delta_y;	  /* update duty cycle */
				++x1;
			}
		}
	}
}

static void paint_line(Rcel *screen, short x1, short y1, short x2, short y2, 
	short yend, short color)
/* Draw a line flipping the y coordinate while you're at it. */
{
draw_line(screen, x1, yend-y1, x2, yend-y2, color);
}

#define MAXP 10

static short polyx[MAXP], polyy[MAXP];
int poly_ix;

void make_poly(Poly *p, LLpoint *pts, short *x, short *y, int count)
{
LLpoint *ll;
int i; 

p->pt_count = count;
p->clipped_list = pts;
ll = pts;
i = count;
while (--i >= 0)
	{
	ll->x = *x++;
	ll->y = *y++;
	ll->z = 0;
	ll->next = ll+1;
	++ll;
	}
ll = pts + count - 1;
ll->next = pts;
}


Errcode add_point(short x, short y)
{
if (poly_ix >= MAXP)
	return(Err_overflow);
polyx[poly_ix] = x;
polyy[poly_ix] = y;
++poly_ix;
return(Success);
}

typedef struct
	{
	Rcel *screen;
	short color;
	} Screen_color;

Errcode fill_hline(SHORT y, SHORT x1, SHORT x2, Screen_color *sc)
{
pj_set_hline(sc->screen, sc->color, x1, y, x2-x1+1);
return(Success);
}

Errcode paint_poly(Rcel *screen, short *x, short *y, int count, short color)
{
int i;
Poly poly;
LLpoint poly_pts[MAXP];
Errcode err;
Screen_color sc;


make_poly(&poly, poly_pts, x, y, count);
sc.screen = screen;
sc.color = color;
if ((err = fill_poly_inside(&poly, fill_hline, &sc)) < Success)
	return(err);
for (i=1; i<count; ++i)
	{
	draw_line(screen, x[i-1], y[i-1], x[i], y[i], color);
	}
draw_line(screen, x[0], y[0], x[count-1], y[count-1], color);
return(Success);
}

static Errcode paint_slide(Rcel *screen, Ifile *gf)
{
typedef union {
	short word;
	struct {UBYTE lo,hi;} byte;
	} Reg;
Reg tk;
FILE *f = gf->file;
Errcode err = Success;
short abuf[8];		/* When we want a lot of shorts. */
signed char bbuf[8];
int color = 255;
short lastx, lasty;
short newx, newy;
short yend = gf->sld.u.common.height;
Boolean filling = FALSE;

for (;;)
	{
	if (fread(&tk, sizeof(tk), 1, f) != 1)
		return(Err_truncated);
	if (gf->sld.is_swapped)
		intel_swaps(&tk, 1);
	if (tk.word >= 0)	/* usual case, a line follows */
		{
		if (fread(abuf, sizeof(short), 3, f) != 3)
			return(Err_truncated);
		if (gf->sld.is_swapped)
			intel_swaps(abuf,3);
		paint_line(screen, lastx = tk.word, lasty = abuf[0], 
				  abuf[1], abuf[2], yend, color);
		}
	else
		{
		switch (tk.byte.hi)
			{
			case 0xfb:	/* Offset vector */
				{
				if (fread(bbuf, sizeof(char), 3, f) != 3)
					return(Err_truncated);
				newx = (signed char)tk.byte.lo + lastx;
				newy = bbuf[0] + lasty;
				paint_line(screen, newx, newy,
						  lastx+bbuf[1],lasty+bbuf[2], yend, color);
				lastx = newx;
				lasty = newy;
				}
				break;
			case 0xfc:	/* End of file */
				return(Success);
			case 0xfd:	/* Solid fill */
				{
				if (fread(abuf, sizeof(short), 2, f) != 2)
					return(Err_truncated);
				if (gf->sld.is_swapped)
					intel_swaps(abuf,2);
				if (abuf[1] < 0)	/* negative Y coordinate, 
									 * toggle fill state */
					{
					if ((filling = !filling) != 0)	/* starting off */
						{
						poly_ix = 0;
						}
					else							/* finishing up */
						{
						if ((err = paint_poly(screen, polyx, polyy, 
											  poly_ix, color)) < Success)
							return(err);
						}
					}
				else
					{
					if (!filling)
						return(Err_format);
					if ((err = add_point(abuf[0], yend-abuf[1])) < Success)
						return(err);
					}
				}
				break;
			case 0xfe:	/* Common endpoint */
				{
				if (fread(bbuf, sizeof(char), 1, f) != 1)
					return(Err_truncated);
				newx = (signed char)tk.byte.lo + lastx;
				newy = bbuf[0] + lasty;
				paint_line(screen, lastx, lasty, newx, newy, yend, color);
				lastx = newx;
				lasty = newy;
				}
				break;
			case 0xff:  /* New color */
				{
				color = tk.byte.lo;
				}
				break;
			default:
				return(Err_unimpl);
			}
		}
	}
}

UBYTE auto_cmap[256*3] = {
0,0,0, 252,0,0, 252,252,0, 0,252,0,
0,252,252, 0,0,252, 252,0,252, 252,252,252,
252,252,252, 252,252,252, 252,0,0, 252,124,124,
160,0,0, 160,80,80, 124,0,0, 124,60,60,
72,0,0, 72,36,36, 36,0,0, 36,16,16,
252,60,0, 252,156,124, 160,40,0, 160,100,80,
124,28,0, 124,76,60, 72,16,0, 72,44,36,
36,8,0, 36,20,16, 252,124,0, 252,188,124,
160,80,0, 160,120,80, 124,60,0, 124,92,60,
72,36,0, 72,56,36, 36,16,0, 36,28,16,
252,188,0, 252,220,124, 160,120,0, 160,140,80,
124,92,0, 124,108,60, 72,56,0, 72,64,36,
36,28,0, 36,32,16, 252,252,0, 252,252,124,
160,160,0, 160,160,80, 124,124,0, 124,124,60,
72,72,0, 72,72,36, 36,36,0, 36,36,16,
188,252,0, 220,252,124, 120,160,0, 140,160,80,
92,124,0, 108,124,60, 56,72,0, 64,72,36,
28,36,0, 32,36,16, 124,252,0, 188,252,124,
80,160,0, 120,160,80, 60,124,0, 92,124,60,
36,72,0, 56,72,36, 16,36,0, 28,36,16,
60,252,0, 156,252,124, 40,160,0, 100,160,80,
28,124,0, 76,124,60, 16,72,0, 44,72,36,
8,36,0, 20,36,16, 0,252,0, 124,252,124,
0,160,0, 80,160,80, 0,124,0, 60,124,60,
0,72,0, 36,72,36, 0,36,0, 16,36,16,
0,252,60, 124,252,156, 0,160,40, 80,160,100,
0,124,28, 60,124,76, 0,72,16, 36,72,44,
0,36,8, 16,36,20, 0,252,124, 124,252,188,
0,160,80, 80,160,120, 0,124,60, 60,124,92,
0,72,36, 36,72,56, 0,36,16, 16,36,28,
0,252,188, 124,252,220, 0,160,120, 80,160,140,
0,124,92, 60,124,108, 0,72,56, 36,72,64,
0,36,28, 16,36,32, 0,252,252, 124,252,252,
0,160,160, 80,160,160, 0,124,124, 60,124,124,
0,72,72, 36,72,72, 0,36,36, 16,36,36,
0,188,252, 124,220,252, 0,120,160, 80,140,160,
0,92,124, 60,108,124, 0,56,72, 36,64,72,
0,28,36, 16,32,36, 0,124,252, 124,188,252,
0,80,160, 80,120,160, 0,60,124, 60,92,124,
0,36,72, 36,56,72, 0,16,36, 16,28,36,
0,60,252, 124,156,252, 0,40,160, 80,100,160,
0,28,124, 60,76,124, 0,16,72, 36,44,72,
0,8,36, 16,20,36, 0,0,252, 124,124,252,
0,0,160, 80,80,160, 0,0,124, 60,60,124,
0,0,72, 36,36,72, 0,0,36, 16,16,36,
60,0,252, 156,124,252, 40,0,160, 100,80,160,
28,0,124, 76,60,124, 16,0,72, 44,36,72,
8,0,36, 20,16,36, 124,0,252, 188,124,252,
80,0,160, 120,80,160, 60,0,124, 92,60,124,
36,0,72, 56,36,72, 16,0,36, 28,16,36,
188,0,252, 220,124,252, 120,0,160, 140,80,160,
92,0,124, 108,60,124, 56,0,72, 64,36,72,
28,0,36, 32,16,36, 252,0,252, 252,124,252,
160,0,160, 160,80,160, 124,0,124, 124,60,124,
72,0,72, 72,36,72, 36,0,36, 36,16,36,
252,0,188, 252,124,220, 160,0,120, 160,80,140,
124,0,92, 124,60,108, 72,0,56, 72,36,64,
36,0,28, 36,16,32, 252,0,124, 252,124,188,
160,0,80, 160,80,120, 124,0,60, 124,60,92,
72,0,36, 72,36,56, 36,0,16, 36,16,28,
252,0,60, 252,124,156, 160,0,40, 160,80,100,
124,0,28, 124,60,76, 72,0,16, 72,36,44,
36,0,8, 36,16,20, 80,80,80, 116,116,116,
148,148,148, 184,184,184, 216,216,216, 252,252,252,
};

static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Seek to the beginning of an open  file, and then read in the
 * first frame of image into screen.  (In our case read in the only
 * frame of image.)
 ****************************************************************************/
{
Ifile *gf = (Ifile *)ifile;		/* There's a bit of data past ifile header */
FILE *f = gf->file;
Errcode err = Success;

if ((err = fseek(f, gf->sld.end_off, SEEK_SET)) < Success)
	goto ERROR;
pj_clear_rast(screen);
	/* load in color map here... */
memcpy(screen->cmap->ctab, auto_cmap, sizeof(auto_cmap));
pj_cmap_load(screen,screen->cmap);
	/* Then do pixels onto screen) */
if ((err = paint_slide(screen, gf)) < Success)
	goto ERROR;
ERROR:
	return(err);
}

static Errcode read_next(Image_file *ifile,Rcel *screen)
/*****************************************************************************
 * Read in subsequent frames of image.  Since we only have one  this
 * routine is pretty trivial. 
 ****************************************************************************/
{
return(Success);
}

Hostlib _a_a_stdiolib = { NULL, AA_STDIOLIB, AA_STDIOLIB_VERSION };
Hostlib _a_a_gfxlib = { &_a_a_stdiolib, AA_GFXLIB, AA_GFXLIB_VERSION };
Hostlib _a_a_syslib = { &_a_a_gfxlib, AA_SYSLIB, AA_SYSLIB_VERSION };

static char title_info[] = "AutoCAD Slide.";

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, &_a_a_syslib },
	title_info, 		/* title_info */
						/* long_info */
	"AutoCAD slide file format reader.\n  Copyright 1991 Dancing Flame",
	".SLD",		 		/* default_suffi */
	0,1,		 		/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	NOFUNC,				/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file,			/* (*close_image_file)() */
	read_first,			/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	NOFUNC,				/* (*save_frames)() */
};


