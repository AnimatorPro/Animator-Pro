
/* scale.c - routines to do pixel averaging/interpolation image scaling. */

#include "errcodes.h"
#include "convert.h"
#include "scale.h"
#include "ffile.h"
#include "rgbcmap.h"
#include "reqlib.h"
#include "softmenu.h"


#define WHOLESCALE 256

FILE *rgb_files[3];

char *rgb_names[3] =
/* files for each of the red/green/blue components */
	{
	"=:red",
	"=:green",
	"=:blue",
	};

Errcode rgb_temp_err(Errcode err)
{
return(softerr(err, "rgb_temp"));
}

void kill_rgb_files()
{
int i;
for (i=0;i<3;i++)
	pj_delete(rgb_names[i]);
}

Errcode open_rgb_files(char *mode, int comp_count)
{
Errcode err;
int i;

for (i=0; i<comp_count; i++)
	{
	if ((err = ffopen(rgb_names[i], &rgb_files[i], mode)) < Success)
		{
		close_rgb_files();
		break;
		}
	}
return(rgb_temp_err(err));
}

close_rgb_files()
{
int i;

for (i=0; i<3; i++)
	ffclose(&rgb_files[i]);
}


Errcode abort_scale()
/*****************************************************************************
 * Query abuser if they wanna abort the ardous scaling process
 ****************************************************************************/
{
return(soft_abort("scale_abort"));
}


void pix_ave_scale(UBYTE *s, int sct, UBYTE *d, int dct)
/*****************************************************************************
 * Do a pixel averaging/interpolation scale of s into d.
 ****************************************************************************/
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

static void vga_to_red(UBYTE *in, UBYTE *out, int w, UBYTE *cmap)
/*****************************************************************************
 * Take a pixel buffer as input and run it through one of the components
 * of cmap (organized as RGB triples.)
 ****************************************************************************/
{
while (--w >= 0)
	*out++ = cmap[3 * *in++];
}

static Errcode scale_xdim_cel(Rcel *cel, int new_w, int comp_count)
/*****************************************************************************
 * Take a color mapped cel and write it's RGB components into files,
 * scaling as you go.
 ****************************************************************************/
{
Errcode err = Success;
UBYTE *line_in = NULL;
UBYTE *rgb_in = NULL;
UBYTE *rgb_out = NULL;
UBYTE *cmap = (UBYTE *)(cel->cmap->ctab);
int w = cel->width;
int h = cel->height;
int i,j;
int saymod = 0;

if ((err = open_rgb_files("wb", comp_count)) < Success)
	goto OUT;
if ((rgb_in = pj_malloc(w)) == NULL
	|| (rgb_out = pj_malloc(new_w)) == NULL
	|| (line_in = pj_malloc(w)) == NULL)
	{
	err = Err_no_memory;
	goto OUT;
	}
rdisk_set_max_to_avail(); /* set to max ram available */
for (j=0; j<h; j++)
	{
	if (--saymod <= 0)
		{
		if ((err = abort_scale()) < Success)
			goto OUT;
		soft_status_line("!%d%d","ctop_xscline",j,h);
		saymod = 25;
		}
	pj_get_hseg(cel,line_in,0,j,w);
	for (i=0; i<comp_count; i++)
		{
		vga_to_red(line_in, rgb_in, w, cmap+i);
		pix_ave_scale(rgb_in, w, rgb_out, new_w);
		if ((err = ffwrite(rgb_files[i],rgb_out,new_w)) < Success)
			goto OUT;
		}
	}
OUT:
pj_freez(&rgb_in);
pj_freez(&rgb_out);
pj_freez(&line_in);
close_rgb_files();
return(rgb_temp_err(err));
}

static void get_column(UBYTE *s, UBYTE *d, int w, int h)
/*****************************************************************************
 * Get a column d from a 2-D array of pixels s.
 ****************************************************************************/
{
while (--h >= 0)
	{
	*d++ = *s;
	s += w;
	}
}

static void put_column(UBYTE *s, UBYTE *d, int w, int h)
/*****************************************************************************
 * Put s into a column of a 2-D array of pixels in d.
 ****************************************************************************/
{
while (--h >= 0)
	{
	*d = *s++;
	d += w;
	}
}

static Errcode scaley(
	   UBYTE *inbytes,		/* unscaled source 2-d array */
	   UBYTE *outbytes, 	/* scaled destination 2-d array */
	   UBYTE *inline,		/* line buffer size oh */
	   UBYTE *outline,		/* line buffer size nh */
	   int w,				/* width of source and destination */
	   int oh,				/* height of source */
	   int nh,				/* height of destination */
	   char *comp_name) 	/* name of color component we're scaling */
/*****************************************************************************
 * interpolate scale a byte-plane in memory in y dimension
 ****************************************************************************/
{
int i;
char buf[50];
int saymod = 0;
Errcode err = Success;

for (i=0; i<w; i++)
	{
	get_column(inbytes++, inline, w, oh);
	pix_ave_scale(inline, oh, outline, nh);
	put_column(outline, outbytes++, w, nh);
	if (--saymod <= 0)
		{
		if ((err = abort_scale()) < Success)
			goto OUT;
		soft_status_line("!%s%d%d", "ctop_yscline", comp_name,i,w);
		saymod = 50;
		}
	}
OUT:
	return(err);
}

Errcode yscale_file(char *name, int w, int oh, int nh)
/*****************************************************************************
 * Assuming name is a file that contains an array of w*oh pixels,
 * scale file so it contains an array of w*nh pixels.
 ****************************************************************************/
{
Errcode err = Success;
UBYTE *inbytes = NULL;
UBYTE *outbytes;
Rcel *outcel = NULL;
long ibsize = w*oh; 	/* size of input file */
UBYTE *inline = NULL, *outline = NULL;	/* Line buffers */

if (oh == nh)		/* if input and output size same done already */
	return(Success);
ibsize = w * oh;	/* calculate size of input file */
if ((inbytes = pj_malloc(ibsize)) == NULL
	|| (inline = pj_malloc(oh)) == NULL
	|| (outline = pj_malloc(nh)) == NULL)
	{
	err = Err_no_memory;
	goto OUT;
	}
if ((err = valloc_ramcel(&outcel, w, nh)) < Success)
	goto OUT;
outbytes = outcel->hw.bm.bp[0];
if ((err = read_gulp(name, inbytes, ibsize)) < Success)
	goto OUT;
if ((err = scaley(inbytes, outbytes, inline, outline, w, oh, nh, name))
	< Success)
	goto OUT;
if ((err = write_gulp(name, outbytes, w*nh)) < Success)
	goto OUT;
grey_cmap(outcel->cmap);
conv_see_cel(outcel);
OUT:
pj_gentle_free(outline);
pj_gentle_free(inline);
pj_gentle_free(inbytes);
freez_cel(&outcel);
return(err);
}

void cc_fit_line(Ccache *cc, Cmap *cmap, UBYTE **rgb_bufs, UBYTE *dest, int count)
/*****************************************************************************
 * Synthesize dest from rgb components using ccache.
 ****************************************************************************/
{
UBYTE *r = *rgb_bufs++;
UBYTE *g = *rgb_bufs++;
UBYTE *b = *rgb_bufs;
Rgb3 rgb;

while (--count >= 0)
	{
	rgb.r = *r++;
	rgb.g = *g++;
	rgb.b = *b++;
	*dest++ = cc_closest(cc, &rgb, cmap, cs.do_dither);
	}
}

Errcode cmap_from_rgb_files(Cmap *cmap, int w, int h, int comp_count)
/*****************************************************************************
 * Make up a color map based on contents of RGB files.
 ****************************************************************************/
{
Errcode err;
int j;
UBYTE *histogram = NULL;
UBYTE *rgbs[3]	 = {NULL, NULL, NULL};

if (comp_count == 1)		/* deal with the all grey case quickly */
	{
	grey_cmap(cmap);
	return(Success);
	}
if ((err = alloc_histogram(&histogram)) < Success)
	goto OUT;
if ((err = open_rgb_files("rb", comp_count)) < Success)
	goto OUT;
j = 3;
while (--j >= 0)
	{
	if ((rgbs[j] = pj_malloc(w)) == NULL)
		{
		err = Err_no_memory;
		goto OUT;
		}
	}
soft_status_line("ctop_hist");
while (--h >= 0)
	{
	j = 3;
	while (--j >= 0)		/* read in a line of each component */
		{
		if ((err = ffread(rgb_files[j], rgbs[j], w)) < Success)
			goto OUT;
		}
	hist_set_bits(histogram, rgbs, w);
	}
hist_to_cmap(&histogram, cmap);
OUT:
	close_rgb_files();
	freez_histogram(&histogram);
	pj_gentle_free(rgbs[0]);
	pj_gentle_free(rgbs[1]);
	pj_gentle_free(rgbs[2]);
	return(err);
}

Errcode rgb_files_to_cel(Rcel *cel, int comp_count, Boolean new_cmap,
						 Boolean flip)
/*****************************************************************************
 * Read color component files into the cel color fitting to the cel's
 * cmap.
 ****************************************************************************/
{
Errcode err;
int w = cel->width;
int h = cel->height;
Cmap *cmap = cel->cmap;
Ccache *cc = NULL;
UBYTE *rgb_bufs[3];
UBYTE *lbuf = NULL;
int i,j;
int saymod = 0;
int y,dy;

rgb_bufs[0] = rgb_bufs[1] = rgb_bufs[2] = NULL; /* for error recovery */
if (new_cmap)
	{
	if ((err = cmap_from_rgb_files(cmap, w, h, comp_count)) < Success)
		goto OUT;
	}
if ((err = open_rgb_files("rb", comp_count)) < Success)
	goto OUT;
										/* Allocate line buffers */
if ((lbuf = pj_malloc(w)) == NULL)
	{
	err = Err_no_memory;
	goto OUT;
	}
for (i=0; i<comp_count; ++i)
	{
	if ((rgb_bufs[i] = pj_malloc(w)) == NULL)
		{
		err = Err_no_memory;
		goto OUT;
		}
	}
if (comp_count == 1)
	rgb_bufs[1] = rgb_bufs[2] = rgb_bufs[0];
if ((err = cc_make(&cc, comp_count==1, cs.colors_256)) < Success)
	goto OUT;
if (flip)
	{
	y = h-1;
	dy = -1;
	}
else
	{
	y = 0;
	dy = 1;
	}
for (i=0; i<h; ++i)
	{
	if (--saymod <= 0)
		{
		if ((err = abort_scale()) < Success)
			goto OUT;
		soft_status_line("!%d%d","ctop_cfitline",i,h);
		saymod = 10;
		}
	for (j=0; j<comp_count; ++j)
		{
		if ((err = ffread(rgb_files[j],rgb_bufs[j],w)) < Success)
			goto OUT;
		}
	cc_fit_line(cc,cmap,rgb_bufs,lbuf,w);
	pj_put_hseg(cel, lbuf, 0, y, w);
	y += dy;
	}
OUT:
cc_free(cc);
for (i=0; i<comp_count; ++i)
	pj_gentle_free(rgb_bufs[i]);
pj_gentle_free(lbuf);
close_rgb_files();
return(err);
}

static Errcode pcel_from_rgb_files(Rcel **pcel, int w, int h, Cmap *cmap,
						   int comp_count)
/*****************************************************************************
 * Make up an Rcel wxh from the rgb files fitting them to the color map.
 ****************************************************************************/
{
Errcode err = Success;
Rcel *cel;

if ((err = valloc_anycel(&cel, w, h)) < Success)
	return(err);
pj_cmap_copy(cmap, cel->cmap);
if ((err = rgb_files_to_cel(cel, comp_count,cs.recalc_colors,FALSE)) < Success)
	freez_cel(&cel);
*pcel = cel;
return(err);
}

static Boolean is_grey_cel(Rcel *cel)
/*****************************************************************************
 * Return TRUE if all pixels in Cel are a shade of grey.
 ****************************************************************************/
{
UBYTE cused[COLORS];	/* Table of colors used */
int i = COLORS;
Rgb3 *ct = cel->cmap->ctab;
UBYTE *cu = cused;
UBYTE r;

make_cused(cel, cused, COLORS);
while (--i >= 0)
	{
	if (*cu++)
		{
		r = ct->r;
		if (r != ct->g || r != ct->b)
			return(FALSE);
		}
	++ct;
	}
return(TRUE);
}

static Errcode cel_to_scaled_rgb(Rcel *cel, int neww, int newh,
								 int *pcomp_count)
/*****************************************************************************
 * Take a cel and transform it into scaled rgb files.
 ****************************************************************************/
{
Errcode err;
int i;
int comp_count = (is_grey_cel(cel) ? 1 : 3);

if ((err = scale_xdim_cel(cel, neww, comp_count)) < Success)
	goto OUT;
for (i=0; i<comp_count; ++i)
	{
	soft_status_line("!%d", "ctop_yscale", i);
	if ((err = yscale_file(rgb_names[i],neww,
						   cel->height,newh)) < Success)
		goto OUT;
	}
OUT:
	*pcomp_count = comp_count;
	return(err);
}

static Errcode scale_in_place(Rcel **pcel, int neww, int newh)
/*****************************************************************************
 * This one scales a single frame cel, replacing the old cel with
 * the new scaled one.
 ****************************************************************************/
{
Errcode err;
Rcel *cel = *pcel;
Cmap *cmap = NULL;
int comp_count;

				/* clone cmap for future reference */
if((err = pj_cmap_alloc(&cmap,COLORS)) < Success)
	goto ERROR;
pj_cmap_copy(cel->cmap, cmap);

if ((err = cel_to_scaled_rgb(cel, neww, newh, &comp_count)) < Success)
	goto ERROR;
freez_cel(pcel);						/* get rid of old cel */
err = pcel_from_rgb_files(pcel,neww, newh, cmap, comp_count);
ERROR:
	pj_cmap_free(cmap);
	kill_rgb_files();
	return(err);
}

static Errcode scale_copy_cel(Rcel *source, Rcel *dest)
/*****************************************************************************
 * Scale source cel into dest.
 ****************************************************************************/
{
Errcode err = Success;
int comp_count;

if ((err = cel_to_scaled_rgb(source, dest->width, dest->height, &comp_count))
	< Success)
	goto OUT;
pj_cmap_copy(source->cmap, dest->cmap);
if ((err = rgb_files_to_cel(dest,comp_count,cs.recalc_colors,FALSE)) < Success)
	goto OUT;

OUT:
	kill_rgb_files();
	return(err);
}

typedef struct
	{
	Rcel *cel;
	} Scale_data;

static Errcode scale_seek(int ix, void *data)
/*****************************************************************************
 * Called from a PDR save routine.	This moves the main convert frame
 * to the input index, and then makes a scaled copy of it in the data
 * frame.  Also keeps user informed of progress a bit by updating the
 * wait box with the frame count and displaying scaled cel.
 ****************************************************************************/
{
#define sd ((Scale_data *)data)
Errcode err;

if ((err = soft_abort("conv_abort")) < Success)
	goto OUT;
soft_put_wait_box("!%d%d", "ctop_scalef", ix+1, cs.ifi.ai.num_frames);
if ((err = ifi_cel_seek(ix,NULL)) < Success)
	goto OUT;
if ((err = scale_copy_cel(cs.ifi.cel, sd->cel)) < Success)
	goto OUT;
conv_see_cel(sd->cel);
OUT:
return(err);
#undef sd
}

Errcode scale_flic(char *sname, int nw, int nh, char *pdr_name)
{
Errcode err = Success;
Rcel *cur_frame = NULL;
Rcel *prev_frame = NULL;
Anim_info ai;
Pdr *pdr;
Image_file *ifile = NULL;
Scale_data sd;

if ((err = get_new_pdr(&pdr, pdr_name)) < Success)
	goto OUT;
ai = cs.ifi.ai;
ai.width = nw;
ai.height = nh;
if ((err = pdr_create_ifile(pdr, sname, &ifile, &ai)) < Success)
	goto OUT;
if ((err = valloc_ramcel(&cur_frame, nw, nh)) < Success)
	goto OUT;
if ((err = valloc_ramcel(&prev_frame, nw, nh)) < Success)
	goto OUT;
sd.cel = cur_frame;
if ((err = scale_seek(0,&sd)) < Success)
	goto OUT;
if ((err = pdr_save_frames(ifile, cur_frame,ai.num_frames,scale_seek,
						   &sd,prev_frame)) < Success)
	{
	pdr_close_ifile(&ifile);
	pj_delete(sname);		/* on error delete bogus file */
	goto OUT;
	}
OUT:
pdr_close_ifile(&ifile);
conv_free_pdr(&pdr);
freez_cel(&cur_frame);
freez_cel(&prev_frame);
return(err);
}

static Errcode render_scale()
{
Errcode err;
char *name;
char *pdr_name;
char *suffi;
char titbuf[80];
char nbuf[32];

if (cs.ifi.ai.num_frames == 1)		/* single frame scale */
	{
	err = scale_in_place(&cs.ifi.cel, cs.scalew, cs.scaleh);
	conv_center_cel(cs.ifi.cel);
	}
else
	{
	pdr_name = fli_pdr_name;
	suffi = ".FLC";
	/* If 320x200 see if they want it to be lo res animator
	 * compatible */
	if (cs.scalew == 320 && cs.scaleh == 200)
		if (soft_yes_no_box("conv_scale_flilo"))
			{
			pdr_name = flilores_pdr_name;
			suffi = ".FLI";
			}
	if ((name = conv_save_name(stack_string("conv_sclname", titbuf),
					   suffi, stack_string("conv_sclword",nbuf))) != NULL)
		{
		if ((err = scale_flic(name, cs.scalew, cs.scaleh, pdr_name)) >=
			Success)
			{
			/* Make scaled flic the current flic */
			err = get_a_flic(fli_pdr_name, name, NULL);
			}
		}
	else
		err = Err_abort;
	}
conv_see_cel(cs.ifi.cel);
cleanup_toptext();
return(softerr(err,NULL));
}



void qscale_opts()
{
USHORT mdis[5];

for (;;)
	{
	/* set up asterisks */
	clear_mem(mdis, sizeof(mdis));
	if (cs.do_dither)
		mdis[0] |= QCF_ASTERISK;
	if (cs.recalc_colors)
		mdis[1] |= QCF_ASTERISK;
	if (cs.colors_256)
		mdis[3] |= QCF_ASTERISK;
	else
		mdis[2] |= QCF_ASTERISK;
	switch (soft_qchoice(mdis, "conv_scale_opts"))
		{
		case 0:
			cs.do_dither = !cs.do_dither;
			break;
		case 1:
			cs.recalc_colors = !cs.recalc_colors;
			break;
		case 2:
			cs.colors_256 = FALSE;
			break;
		case 3:
			cs.colors_256 = TRUE;
			break;
		default:
			return;
		}
	}
}

void qscale_menu()
{
int choice;
long r1, r2;
int sw = vb.pencel->width;
int sh = vb.pencel->height;

if (cs.ifi.cel == NULL) /* This should be handled by disable logic soon */
	return;
for (;;)
	{
	choice = soft_qchoice(NULL,
						  "!%d%d%d%d%d%d%d%d", "conv_scale",
						  cs.ifi.cel->width, cs.ifi.cel->height,
						  cs.scalew, cs.scaleh,
						  sw, sh,
						  cs.ifi.ai.width, cs.ifi.ai.height);
	switch (choice)
		{
		case 0: 	/* Set width */
			clip_soft_qreq_number(&cs.scalew, 4, 10000, NULL, NULL,
				"conv_scale_width");
			break;
		case 1: 	/* Set height */
			clip_soft_qreq_number(&cs.scaleh, 4, 10000, NULL, NULL,
				"conv_scale_height");
			break;
		case 2: 	/* Set Default dims */
			cs.scalew = sw;
			cs.scaleh = sh;
			break;
		case 3: 	/* Correct aspect ratio */
			r1 = (long)sw*cs.ifi.cel->height;
			r2 = (long)sh*cs.ifi.cel->width;
			if (r1 > r2)	/* need to shrink y */
				{
				cs.scalew = cs.ifi.cel->width;
				cs.scaleh = (long)(cs.ifi.cel->height * r2 / r1);
				}
			else			/* need to shrink x */
				{
				cs.scalew = (long)(cs.ifi.cel->width * r1 / r2);
				cs.scaleh = cs.ifi.cel->height;
				}
			break;
		case 4: 	/* Revert... */
			cs.scalew = cs.ifi.ai.width;
			cs.scaleh = cs.ifi.ai.height;
			break;
		case 5: 	/* Dither */
			qscale_opts();
			break;
		case 6: 	/* Render */
			if (render_scale() < Success)
													/* exit 'cause cel might not
													 * exist */
				goto OUT;
			break;
		default:
			goto OUT;
		}
	}
OUT:
	return;
}
