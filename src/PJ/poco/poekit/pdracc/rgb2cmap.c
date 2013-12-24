/*****************************************************************************
 *
 ****************************************************************************/

#include <errcodes.h>
#include <rcel.h>
#include <cmap.h>
#include <pocolib.h>
#include <gfx.h>
#include <syslib.h>
#include <picdrive.h>
#include "packcmap.h"
#include "ccache.h"

#define DITHERFLAG 0x00010000

#define MC_FIRST 251
#define MC_COUNT 5
#define RGB_MAX  256

enum {
	RGB_GREY,
	RGB_C6CUBE,
	RGB_C64LEVEL,
	RGB_C256LEVEL,
	};

Boolean 	colors_256;
static Rgb3 rgb_black = {0,0,0};

static void make_6cube_palette(Rcel *screen)
/*****************************************************************************
 * make a palette for the 6-cube color approximation algorithm.
 *	this uses only 216 colors, so for user-friendliness, we load the user's
 *	preferred menu colors into the last 5 slots of the palette.
 ****************************************************************************/
{
	Rgb3 *mu_cm;
	Rgb3 *ps_cm;
	int   r, g, b;
	int   i;

	ps_cm = screen->cmap->ctab;

	for (r=0; r<6; r++) {
		for (g=0; g<6; g++) {
			for (b=0; b<6; b++) {
				ps_cm->r = RGB_MAX*r/6;
				ps_cm->g = RGB_MAX*g/6;
				ps_cm->b = RGB_MAX*b/6;
				++ps_cm;
			}
		}
	}

	GetMenuColors(NULL, NULL, &mu_cm);

	ps_cm = &screen->cmap->ctab[MC_FIRST];
	for (i = 0; i < MC_COUNT; ++i)
		*ps_cm++ = *mu_cm++;

	pj_cmap_load(screen, screen->cmap);
}

static Errcode make_color_palette(Image_file *ifile, Anim_info *ainfo,
								 Rcel *screen, Rgb3 *linebuf)
/*****************************************************************************
 * make a palette of the 254 colors closest to those found in the file.
 *	 color slots 0 and 255 always contain rgb {0,0,0} to help speed up
 *	 our color fitting and caching algorithms.	black in slot 255 is an
 *	 assist, but black in slot zero is an absolute requirement of the
 *	 current caching routines!	(oops, not anymore.  the new routines in
 *	 ccache.asm no longer require black in slot zero.)
 ****************************************************************************/
{
	Errcode err 	  = Success;
	int 	height	  = ainfo->height;
	int 	width	  = ainfo->width;
	int 	y		  = 0;
	Pdr 	*pdr	  = ifile->pd;
	Rgb3	*big_ctab = NULL;
	Rgb3	*ps_cm;
	Rgb3	*mu_cm;
	int 	i;
	int 	progress;
	int 	ccount;
	int 	dcount;
	static
	  Popot hist_progress = {"Making color histogram, working on line %d"};

	/*------------------------------------------------------------------------
	 * set the user's preferred menu colors into Animator's menu slots, so
	 * that our printf() messages show up while we're running.  if the file
	 * contains < 256 colors, these will remain in the pallete, else they
	 * get overwritten once we've finished the color packing.
	 *----------------------------------------------------------------------*/

	GetMenuColors(NULL, NULL, &mu_cm);

	ps_cm = &screen->cmap->ctab[MC_FIRST];
	for (i = 0; i < MC_COUNT; ++i)
		*ps_cm++ = *mu_cm++;

	pj_cmap_load(screen, screen->cmap);

	/*------------------------------------------------------------------------
	 * read the rgb data and build a histogram of colors encountered.
	 *----------------------------------------------------------------------*/

	err = pdr->rgb_seekstart(ifile);
	if (err < Success) {
		goto OUT;
	}

	cc_histinit();
	progress = 0;
	while (--height >= 0) {
		if (--progress <= 0) {
			if (CheckAbort(NULL)) {
				err = builtin_err = Err_abort;
				goto OUT;
			}
			progress = 20;
			poeprintf(1,4,hist_progress,y);
		}
		++y;						// used only for progress reporting

		if (Success > (err = pdr->rgb_readline(ifile, linebuf)))
			goto OUT;
		cc_histline(linebuf, width);
	}

	/*------------------------------------------------------------------------
	 * figure out how many colors we found, convert the histogram to a big
	 * ctab (array of Rgb3) contains all those colors, then do color packing.
	 *----------------------------------------------------------------------*/

	poeprintf(0,0,ptr2ppt("Converting histogram to color map...",0));

	ccount = cc_hist_color_count();

	if (NULL == (big_ctab = malloc(ccount*sizeof(Rgb3)))) {
		err = Err_no_memory;
		goto OUT;
	}

	cc_hist_to_ctab(big_ctab);

	if (ccount > screen->cmap->num_colors-1)
		dcount = screen->cmap->num_colors-1;
	else
		dcount = ccount;	// this preserves menu colors when possible

	err = fpack_ctable(big_ctab,ccount,screen->cmap->ctab,dcount);

	screen->cmap->ctab[255] = rgb_black; /* this helps caching work better */

	if (CheckAbort(NULL)) {
		err = builtin_err = Err_abort;
		goto OUT;
	}

	/*------------------------------------------------------------------------
	 * color palette all built, load it into the hardware and return.
	 *----------------------------------------------------------------------*/

	poeunprintf();

	pj_cmap_load(screen, screen->cmap);

OUT:

	if (big_ctab != NULL)
		free(big_ctab);

	return err;
}

static make_grey_palette(Rcel *screen)
/*****************************************************************************
 * make a palette with 256 levels of grey, load the palette into the screen.
 ****************************************************************************/
{
	int 	counter;
	Rgb3	*ptab = screen->cmap->ctab;

	for (counter = 0; counter < COLORS; ++counter) {
		ptab->r = ptab->g = ptab->b = counter;
		++ptab;
	}

	pj_cmap_load(screen, screen->cmap);
}

Errcode load_rgbconvert(ULONG loadoption, Image_file *ifile,
						Anim_info *ainfo, Rcel *screen)
/*****************************************************************************
 * load an rgb picture to the screen converting by the user-choosen method.
 ****************************************************************************/
{
	Errcode err;
	Pdr 	*pdr = ifile->pd;
	int 	width = ainfo->width;
	int 	linecounter;
	int 	pixcounter;
	int 	y, dy;
	int 	tabcount;
	int 	progress;
	Boolean do_dither;
	Rgb3	*ctab;
	Rgb3	*linebuf = NULL;
	UBYTE	*pixbuf  = NULL;
	UBYTE	*pbuf;
	Rgb3	*lbuf;

	do_dither	= (FALSE != (loadoption & DITHERFLAG));
	loadoption &= ~DITHERFLAG;
	colors_256	= (loadoption == RGB_C256LEVEL) ? TRUE : FALSE;

	/*------------------------------------------------------------------------
	 * get buffers...
	 *----------------------------------------------------------------------*/

	err = Err_no_memory;	// if we die now, this is why.

	if (NULL == (linebuf = malloc(width*sizeof(*linebuf)))) {
		goto ERROR_EXIT;
	}

	if (NULL == (pixbuf = malloc(width))) {
		goto ERROR_EXIT;
	}

	/*------------------------------------------------------------------------
	 * build the appropriate color palette...
	 *----------------------------------------------------------------------*/

	ctab = screen->cmap->ctab;
	tabcount = screen->cmap->num_colors;

	switch (loadoption) {
	  case RGB_GREY:
		make_grey_palette(screen);
		break;
	  case RGB_C6CUBE:
		make_6cube_palette(screen);
		break;
	  case RGB_C64LEVEL:
	  case RGB_C256LEVEL:
		if (Success > (err = cc_init(screen->cmap, colors_256, do_dither)))
			goto ERROR_EXIT;
		if (Success > (err = make_color_palette(ifile, ainfo, screen, linebuf)))
			goto ERROR_EXIT;
		cc_cfitinit();
		break;
	  default:
		err = Err_unimpl;
		goto ERROR_EXIT;
	}

	/*------------------------------------------------------------------------
	 * seek to rgb data in file, setup rightsideup/upsidedown processing...
	 *----------------------------------------------------------------------*/

	if (Success > (err = pdr->rgb_seekstart(ifile))) {
		goto ERROR_EXIT;
	} else if (0 == err) {
		y  = 0;
		dy = 1;
	} else {
		y  = ainfo->height - 1;
		dy = -1;
	}

	/*------------------------------------------------------------------------
	 * load and convert the image...
	 *----------------------------------------------------------------------*/

	progress = 0;
	for (linecounter = 0; linecounter < ainfo->height; ++linecounter) {

		if (Success > (err = pdr->rgb_readline(ifile, linebuf)))
			goto ERROR_EXIT;

		switch (loadoption) {

		  case RGB_GREY:	/* greyscale */

			pbuf = pixbuf;
			lbuf = linebuf;
			for (pixcounter = 0; pixcounter < width; ++pixcounter) {
				*pbuf = (lbuf->r + lbuf->g + lbuf->b) / 3;
				++pbuf;
				++lbuf;
			}
			break;

		  case RGB_C6CUBE:	/* approximate color */

			pbuf = pixbuf;
			lbuf = linebuf;
			for (pixcounter = 0; pixcounter < width; ++pixcounter) {
				*pbuf = (6*lbuf->r/RGB_MAX * 36) +
						(6*lbuf->g/RGB_MAX * 6) +
						(6*lbuf->b/RGB_MAX);
				++pbuf;
				++lbuf;
			}
			break;

		  default:			/* 64 or 256 level color */

			cc_cfitline(pixbuf, linebuf, width);
			break;

		} /* END switch (loadoption) */

		pj_put_hseg(screen, pixbuf, 0, y, width);
		y += dy;

		if (--progress < 0) {
			progress = 5;
			if (CheckAbort(NULL)) {
				err = builtin_err = Err_abort;
				goto ERROR_EXIT;
			}
		}

	} /* END for (linecount) */

ERROR_EXIT:

	if (loadoption == RGB_C64LEVEL || loadoption == RGB_C256LEVEL) {
		cc_cleanup();
	}

	if (linebuf != NULL)
		free(linebuf);
	if (pixbuf != NULL)
		free(pixbuf);

	return err;
}
