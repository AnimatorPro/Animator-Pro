/*****************************************************************************
 * CONVRGB.C  - Routines to process rgb input pictures.
 ****************************************************************************/
#include "convert.h"
#include "rgbcmap.h"
#include "scale.h"
#include "ccache.h"
#include "errcodes.h"

typedef struct {
	Image_file *ifile;
	int 	   width;
	int 	   height;
	Boolean    is_flipped;
	Rgb3	   *linebuf;
	UBYTE	   *rgb_bufs[3];
	} Rgbctl;

static void rgb3_to_buffers(Rgb3 *rgbs, UBYTE **rgb_bufs, int size)
/*****************************************************************************
 *
 ****************************************************************************/
{
	UBYTE *r = *rgb_bufs++;
	UBYTE *g = *rgb_bufs++;
	UBYTE *b = *rgb_bufs;
	UBYTE *in = (UBYTE *)rgbs;

	while (--size >= 0)
		{
		*r++ = *in++;
		*g++ = *in++;
		*b++ = *in++;
		}

}

static void rgb_to_grey(Rgb3 *rgbs, UBYTE *greys, int size)
/*****************************************************************************
 * convert rgb values into greyscale values by averaging.
 ****************************************************************************/
{
	UBYTE *in = (UBYTE *)rgbs;

	while (--size >= 0)
		{
		*greys++ = (*in++ + *in++ + *in++)/3;
		++rgbs;
		}
}

static Errcode rgb_read_grey(Rgbctl *ctl, Rcel *cel)
/*****************************************************************************
 * Read a rgb file into a cel in greys.
 ****************************************************************************/
{
	int 	y;
	int 	dy;
	Errcode err    = Success;
	int 	height = ctl->height;
	int 	width  = ctl->width;
	UBYTE  *lbuf   = ctl->rgb_bufs[0]; /* use red buffer for greys */

	grey_cmap(cel->cmap);

	err = pdr_rgb_seekstart(ctl->ifile);
	if (err < Success) {
		goto OUT;
	} else if (err == Success) {	/* picture is rightsideup in file */
		y  = 0;
		dy = 1;
	} else {						/* picture is upsidedown in file  */
		y  = ctl->height-1;
		dy = -1;
	}

	while (--height >= 0)
		{
		if (Success > (err = pdr_rgb_readline(ctl->ifile, ctl->linebuf)))
			{
			goto OUT;
			}
		rgb_to_grey(ctl->linebuf, lbuf, width);
		pj_put_hseg(cel, lbuf, 0, y, width);
		y += dy;
		}
OUT:
	return err;
}

static Errcode rgb_read_histogram(Rgbctl *ctl, UBYTE *histogram)
/*****************************************************************************
 * Read rgb file to make up a color histogram.
 ****************************************************************************/
{
	Errcode err    = Success;
	int 	height = ctl->height;
	int 	width  = ctl->width;

	soft_status_line("ctop_hist");

	err = pdr_rgb_seekstart(ctl->ifile);
	if (err < Success) {
		goto OUT;
	}

	while (--height >= 0)
		{
		if (Success > (err = pdr_rgb_readline(ctl->ifile, ctl->linebuf)))
			goto OUT;
		rgb3_to_buffers(ctl->linebuf, ctl->rgb_bufs, width);
		hist_set_bits(histogram,  ctl->rgb_bufs, width);
		}
OUT:
	return err;
}

static Errcode rgb_read_cfit(Rgbctl *ctl, Rcel *screen, Ccache *cc)
/*****************************************************************************
 * Read rgb file into screen while color fitting to the screen palette.
 ****************************************************************************/
{
	int 	y;
	int 	dy;
	int 	i;
	int 	sofar	= 0;
	Errcode err 	= Success;
	int 	height	= ctl->height;
	int 	width	= ctl->width;
	Cmap	*cmap	= screen->cmap;

	err = pdr_rgb_seekstart(ctl->ifile);
	if (err < Success) {
		goto OUT;
	} else if (err == Success) {	/* picture is rightsideup in file */
		y  = 0;
		dy = 1;
	} else {						/* picture is upsidedown in file  */
		y  = ctl->height-1;
		dy = -1;
	}

	for (i=0; i<height; ++i)
		{
		if (--sofar <= 0)
			{
			if ((err = soft_abort("rgb_abort")) < Success)
				goto OUT;
			soft_status_line("!%d%d", "ctop_rfit", i, height);
			sofar = 10;
			}
		if (Success > (err = pdr_rgb_readline(ctl->ifile, ctl->linebuf)))
			goto OUT;
		rgb3_to_buffers(ctl->linebuf, ctl->rgb_bufs, width);
		cc_fit_line(cc, cmap, ctl->rgb_bufs, (UBYTE *)ctl->linebuf, width);
		pj_put_hseg(screen, ctl->linebuf, 0, y, width);
		y += dy;
		}
OUT:
	return err;
}

static Errcode rgb_read_color(Rgbctl *ctl, Rcel *screen)
/*****************************************************************************
 * Read rgb file where screen is the same x/y resolution as the rgb file.
 ****************************************************************************/
{
	Errcode err;
	UBYTE  *histogram = NULL;
	Ccache *cc = NULL;

	if ((err = alloc_histogram(&histogram)) < Success)
		goto OUT;
	if ((err = rgb_read_histogram(ctl, histogram)) < Success)
		goto OUT;
	if ((err = hist_to_cmap(&histogram, screen->cmap)) < Success)
		goto OUT;
	if ((err = cc_make(&cc, FALSE, cs.colors_256)) < Success)
		goto OUT;
	err = rgb_read_cfit(ctl, screen, cc);

OUT:
	cc_free(cc);
	freez_histogram(&histogram);
	return err;
}

static Errcode rgb_scale_x(Rgbctl *ctl, Rcel *screen)
/*****************************************************************************
 * Read rgb file, break it into RGB components, scale the components
 * in the X dimension, and write them out to our RGB files.
 ****************************************************************************/
{
	Errcode err = Success;
	int 	height = ctl->height;
	int 	width  = ctl->width;
	int 	dwidth = screen->width;
	UBYTE	*scale_buf = NULL;
	Cmap	*cmap = screen->cmap;
	int 	j;
	int 	i;
	int 	sofar = 0;

	soft_status_line("ctop_xscale");
	if (NULL == (scale_buf = pj_malloc(dwidth)))
		{
		err = Err_no_memory;
		goto OUT;
		}

	if (Success > (err = open_rgb_files("wb",3)))
		goto OUT;

	if (Success > (err = pdr_rgb_seekstart(ctl->ifile)))
		goto OUT;

	for (i=0; i<height; ++i)
		{
		if (--sofar <= 0)
			{
			if ((err = soft_abort("rgb_abort")) < Success)
				goto OUT;
			soft_status_line("!%d%d", "ctop_scaleline", i, height);
			sofar = 25;
			}
		if ((err = pdr_rgb_readline(ctl->ifile, ctl->linebuf)) < Success)
			goto OUT;
		rgb3_to_buffers(ctl->linebuf, ctl->rgb_bufs, width);
		j = 3;
		while (--j >= 0)
			{
			pix_ave_scale(ctl->rgb_bufs[j], width, scale_buf, dwidth);
			if ((err = ffwrite(rgb_files[j], scale_buf, dwidth)) < Success)
				goto OUT;
			}
		}
OUT:
	pj_gentle_free(scale_buf);
	close_rgb_files();
	return err;
}


static Errcode rgb_read_scaled(Rgbctl *ctl, Rcel *screen)
/*****************************************************************************
 * Read rgb file where screen is a different x/y resolution from the rgb file.
 ****************************************************************************/
{
	Errcode err;
	int i;

	if ((err = rgb_scale_x(ctl, screen)) < Success)
		goto OUT;

	for (i=0; i<3; ++i)
		{
		if ((err = yscale_file(rgb_names[i], screen->width, ctl->height,
							   screen->height)) < Success)
			goto OUT;
		}
	/* In here need to calculate cmap for cel... */
	if ((err = rgb_files_to_cel(screen, 3, TRUE, ctl->is_flipped)) < Success)
		goto OUT;

OUT:
	kill_rgb_files();
	return err;
}



Errcode convrgb_read_image(Image_file *ifile, Rcel *screen, Anim_info *ai, int rgb_choice)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	Rgbctl	*ctl;

	if (NULL == (ctl = pj_zalloc(sizeof(Rgbctl))))
		return Err_no_memory;

	ctl->ifile	= ifile;
	ctl->width	= ai->width;
	ctl->height = ai->height;

	if ((err = pdr_rgb_seekstart(ifile)) < Success)
		goto OUT;
	else
		ctl->is_flipped = (err > 0) ? TRUE : FALSE;

	if (NULL == (ctl->rgb_bufs[0] = pj_malloc(ai->width)) ||
		NULL == (ctl->rgb_bufs[1] = pj_malloc(ai->width)) ||
		NULL == (ctl->rgb_bufs[2] = pj_malloc(ai->width)) ||
		NULL == (ctl->linebuf	  = pj_malloc(3*ai->width)))
		{
		err = Err_no_memory;
		goto OUT;
		}

	switch (rgb_choice)
		{
		case RGB_SCALED:
			err = rgb_read_scaled(ctl, screen);
			break;
		case RGB_COLOR:
			err = rgb_read_color(ctl, screen);
			break;
		case RGB_GREY:
			err = rgb_read_grey(ctl, screen);
			break;
		}

OUT:

	pj_gentle_free(ctl->rgb_bufs[0]);
	pj_gentle_free(ctl->rgb_bufs[1]);
	pj_gentle_free(ctl->rgb_bufs[2]);
	pj_gentle_free(ctl->linebuf);
	pj_free(ctl);

	cleanup_toptext();
	return err;
}


Errcode convrgb_qoptions(void)
/*****************************************************************************
 * Put up rgb load options requestor.
 ****************************************************************************/
{
	int 	rgb_choice;
	USHORT	mdis[5];

	for (;;)
		{
		clear_mem(mdis, sizeof(mdis));
		if (cs.do_dither)
			mdis[RGB_DITHER] |= QCF_ASTERISK;
		if ((rgb_choice = soft_qchoice(mdis, "conv_rgb")) < Success)
			goto OUT;
		if (rgb_choice == RGB_DITHER)  /* dither */
			cs.do_dither = !cs.do_dither;
		else
			break;
		}

OUT:
	return rgb_choice;
}
