/*****************************************************************************
 * PSTAMP.C: A POE module to reduce a screen to a postage stamp image.
 *
 *	Major POE items/features demonstrated herein:
 *
 *		- Using a virtual raster for output.
 *		- Using simple floating point math in a POE module.
 *		- Using GFX functions to render onto an arbitrary screen.
 *
 *		This module provides a way to reduce the size of an image to a
 *		'postage stamp'; a much smaller representation of the original.
 *		The postage stamp image is rendered into a 6-cube color space.
 *		This implies a loss of color accuracy, but the smaller image size
 *		makes this side effect less noticable with most images.  It also
 *		allows any number of full-sized images, each with a different
 *		color palette, to be reduced onto the same output screen.
 *
 * NOTES:
 *
 *		All functions for this POE exist in this one source code module.
 *		The functions herein are defined as static if they are called
 *		only from within this module, and as global if they are exported
 *		to the Poco program.
 *
 * MAINTENANCE:
 *
 *	03/01/91	Ian Lepore
 *				Original version, called FLISUMRY.C.
 *
 *	10/15/91	Ian
 *				Reworked, renamed to PSTAMP.C, included with POE demo progs.
 *				It now allows arbitrary-sized output postage stamps; the old
 *				version required the pstamp width and height to be an
 *				integral divisor of the source image sizes. (IE, it could
 *				only reduce 2:1, 3:1, 4:1, etc.)
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * include the usual header files...
 *--------------------------------------------------------------------------*/

#include "stdtypes.h"
#include "errcodes.h"
#include "ptrmacro.h"
#include "pocorex.h"
#include "pocolib.h"
#include "syslib.h"
#include "gfx.h"
#include "cmap.h"

/*----------------------------------------------------------------------------
 * set up the host libraries we need...
 *--------------------------------------------------------------------------*/

#define HLIB_TYPE_1 AA_POCOLIB
#define HLIB_TYPE_2 AA_SYSLIB
#define HLIB_TYPE_3 AA_GFXLIB
#include <hliblist.h>

/*----------------------------------------------------------------------------
 * local data and constants...
 *--------------------------------------------------------------------------*/

#define BORDER_COLOR_IDX	255 		// we draw borders using color 255,
Rgb3	border_color_rgb =	{0,0,200};	// which we set to a nice medium blue.

#define MIN_PSWIDTH  10 		// these are pretty much arbitrary lower
#define MIN_PSHEIGHT 10 		// limits on the size of a postage stamp.

#define MAX_PSWIDTH  1024		// stack-alloc'd buffer size, MUST be <= 1536.

UBYTE	rtab[256];				// tables to hold the red, green, and blue
UBYTE	gtab[256];				// components of the input screen's color
UBYTE	btab[256];				// map; splitting them gives faster access.

int 	srcblkwi;				// integer width of a source averaging block.
int 	srcblkhi;				// integer height of a source averaging block.

int 	deltabpr;				// distance from end of block to start of next.

/*----------------------------------------------------------------------------
 * For Watcom C only, a performance tweak...
 *	The following pragma specifies passing of a pointer parm in a register
 *	for the very-often-called averaging function.  This is about the *only*
 *	way to fool the Watcom optimizer into a registerizing a pointer in the
 *	averaging function.  (Of course, it also helps to pass the parm in a
 *	reg, but that's not the main point.)
 *--------------------------------------------------------------------------*/

#ifdef __WATCOMC__
  #pragma aux average_pixel_block parm [edx];
#endif

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

static void unload_ctab(Rgb3 *ptab, int tabcount)
/*****************************************************************************
 * unload screen's rgbrgb... cmap to our separate red, green, blue arrays.
 ****************************************************************************/
{
	int i;
	for (i = 0; i < tabcount; ++i) {
		rtab[i] = ptab->r;
		gtab[i] = ptab->g;
		btab[i] = ptab->b;
		++ptab;
	}
}

static void draw_box(Rcel *drast, Pixel color, int x, int y, int w, int h)
/*****************************************************************************
 * draw a hollow box.
 ****************************************************************************/
{
	pj_set_hline(drast, color, x,	  y,	 w);
	pj_set_hline(drast, color, x,	  y+h-1, w);
	pj_set_vline(drast, color, x,	  y,	 h);
	pj_set_vline(drast, color, x+w-1, y,	 h);
}

static unsigned int average_pixel_block(Pixel *inbuf)
/*****************************************************************************
 * average the rgb values in an arbitrary source block to a single rgb value
 * which is mapped into a 6-cube color space.
 *
 * we keep track of 'black' pixels (those with color index 0, presumably the
 * background color) and factor them out of the averaging.	this allows
 * things like a single-pixel-wide line to show up in the postage stamp
 * image; straight averaging would make the line disappear.
 ****************************************************************************/
{
	int  w;
	int  h;
	unsigned int  pix;
	unsigned int  totpix;
	unsigned int  rsum		= 0;
	unsigned int  gsum		= 0;
	unsigned int  bsum		= 0;
	unsigned int  numblack	= 0;

	/*------------------------------------------------------------------------
	 * sum up the pixels in the averaging block...
	 *----------------------------------------------------------------------*/

	h = srcblkhi;
	while (--h >= 0) {
		w = srcblkwi;
		while (--w >= 0) {
			if (0 == (pix = *inbuf++)) {
				++numblack;
			} else {
				rsum += rtab[pix];
				gsum += gtab[pix];
				bsum += btab[pix];
			}
		}
		inbuf += deltabpr; // skip to start of averaging block on next line.
	}

	/*------------------------------------------------------------------------
	 * calc the average, and return the average rgb value mapped into
	 * the 6-cube color space.
	 *----------------------------------------------------------------------*/

	if (0 == (totpix = (srcblkwi * srcblkhi) - numblack)) {
		return 0;
	} else {
		rsum /= totpix;
		gsum /= totpix;
		bsum /= totpix;
		return (6*rsum/RGB_MAX*36)+(6*gsum/RGB_MAX*6)+(6*bsum/RGB_MAX);
	}
}

static void build_output_image(Rcel *vrast, Pixel *srastbuf,
							  int swidth, int sheight, int bpr,
							  double srcblkw, double srcblkh)
/*****************************************************************************
 * process the full-sized input image down to a postage stamp.
 ****************************************************************************/
{
	double srcx;					/* source x */
	double srcy;					/* source y */
	double srcw;					/* source width */
	double srch;					/* source height */
	int    dy;						/* output raster current line */
	int    dwidth;					/* output raster line width */
	Pixel  *psline; 				/* pointer to current source line */
	Pixel  *pdline; 				/* pointer to current location in linebuf */
	Pixel  linebuf[MAX_PSWIDTH];	/* output line buffer */

	/*------------------------------------------------------------------------
	 * initialize local vars...
	 *----------------------------------------------------------------------*/

	dy		 = 0;
	dwidth	 = vrast->width;

	srcw	 = swidth;
	srch	 = sheight;

	/*------------------------------------------------------------------------
	 * initialize global vars we share with averaging routine...
	 *----------------------------------------------------------------------*/

	srcblkwi = srcblkw + 0.5;
	srcblkhi = srcblkh + 0.5;
	deltabpr = bpr - srcblkwi - 1;

	/*------------------------------------------------------------------------
	 * loop through the input pixels, averaging each block of source pixels
	 * down to a single destination pixel.	accumulate a line at a time of
	 * destination pixels, and write them all at once when the line is full.
	 *
	 * note that the source x/y coordinates and the width/height of a source
	 * averaging block are maintained as floating point values.  this allows
	 * the accumulation of fractional pixels as the x/y coords are
	 * incremented, which automatically skips a pixel now and then as
	 * needed when the destination size is not an even multiple of the
	 * source size.
	 *----------------------------------------------------------------------*/

	for (srcy = 0.0; srcy < srch; srcy += srcblkh) {
		pdline = linebuf;
		psline = srastbuf + ((int)(srcy)) * bpr;
		for (srcx = 0.0; srcx < srcw; srcx += srcblkw) {
			*pdline++ = average_pixel_block(&psline[(int)srcx]);
		}
		pj_put_hseg(vrast, linebuf, 0, dy++, dwidth);
	}
}

Errcode make_pstamp(Popot sscreen, Popot dscreen,
					int dxstart, int dystart,
					int dwidth, int dheight,
					Boolean draw_border)
/*****************************************************************************
 * convert a screen to postage stamp image rendered onto another screen.
 ****************************************************************************/
{
	Rcel   *vrast;			 /* virtual destination raster */
	Rcel   workcel; 		 /* work raster for creating a virtual raster */
	int    swidth;			 /* integer width of source raster */
	int    sheight; 		 /* integer height of source raster */
	double srcblkw; 		 /* real width	of source averaging block */
	double srcblkh; 		 /* real height of source averaging block */
	int    vwidth;			 /* virtual dest width	*/
	int    vheight; 		 /* virtual dest height */
	int    bpr; 			 /* bytes per row in source raster */
	Pixel  *srastbuf;		 /* pointer to source raster in memory */
	Pixel  *allocbuf = NULL; /* pointer to allocated raster, if allocated */

	/*------------------------------------------------------------------------
	 * validate parms
	 *----------------------------------------------------------------------*/

	if (NULL == sscreen.pt || NULL == dscreen.pt)
		return builtin_err = Err_null_ref;

	if (dwidth < MIN_PSWIDTH || dheight < MIN_PSHEIGHT) {
		return builtin_err = poeQerror(4, 4*sizeof(int),
			Err_parameter_range,
			str2ppt("Cannot make a %d x %d postage stamp image.  "
					"The smallest allowable image size is %d x %d."
				   ),
			dwidth, dheight, MIN_PSWIDTH, MIN_PSHEIGHT);
	}

	if (dwidth > MAX_PSWIDTH) {
		return builtin_err = poeQerror(3, 3*sizeof(int),
			Err_too_big,
			str2ppt("Cannot make a %d x %d postage stamp image.  "
					"The largest allowable image width is %d."
				   ),
			dwidth, dheight, MAX_PSWIDTH);
	}

	/*------------------------------------------------------------------------
	 * unload the source raster's color map into our local tables; this
	 * allows faster access during averaging (yields better codegen).
	 *
	 * if the source raster is a bytemap, get the pointer to its memory
	 * buffer; if it's some other type of raster, allocate a buffer big
	 * enough to hold all the pixels, and load the pixels into it.
	 *----------------------------------------------------------------------*/

	{
		register Rcel *srast = sscreen.pt;

		swidth	= srast->width;
		sheight = srast->height;

		if (srast->cmap->num_colors > Array_els(rtab))
			return builtin_err = Err_too_big;
		else
			unload_ctab(srast->cmap->ctab, srast->cmap->num_colors);

		if (srast->type == RT_BYTEMAP) {
			srastbuf = srast->hw.bm.bp[0];
			bpr 	= srast->hw.bm.bpr;
		} else {
			if (NULL == (allocbuf = malloc(srast->width * srast->height))) {
				builtin_err = Err_no_memory;
				goto ERROR_EXIT;
			}
			srastbuf = allocbuf;
			bpr 	= srast->width;
			pj_get_rectpix(srast, srastbuf, 0, 0, srast->width, srast->height);
		}
	}

	/*------------------------------------------------------------------------
	 * set up the averaging block width and height.
	 * if the source screen width or height is smaller than the destination
	 * postage stamp size, set the averaging block to 1 pixel; our cheezy
	 * algorithm for averaging does a really bad job of expanding an image.
	 *----------------------------------------------------------------------*/

	if (swidth <= dwidth) {
		srcblkw  = 1.0;
		vwidth	 = swidth;
	} else {
		srcblkw = swidth / (double)dwidth;
		vwidth	= dwidth;
	}

	if (sheight <= dheight) {
		srcblkh  = 1.0;
		vheight  = sheight;
	} else {
		srcblkh = sheight / (double)dheight;
		vheight = dheight;
	}

	/*------------------------------------------------------------------------
	 * build a virtual raster that maps the output rectangle of the
	 * destination raster.	if the output rectangle sizes are greater than
	 * the source raster sizes, we center our virtual raster within the
	 * destination rectangle.
	 *----------------------------------------------------------------------*/

	{
		Rectangle workrect;

		workrect.x		= dxstart + ((dwidth-vwidth) >> 1);
		workrect.y		= dystart + ((dheight-vheight) >> 1);
		workrect.width	= vwidth;
		workrect.height = vheight;

		if (!pj_rcel_make_virtual(&workcel, (Rcel *)dscreen.pt, &workrect))
			goto ERROR_EXIT;
		vrast = &workcel;
	}

	/*------------------------------------------------------------------------
	 * go do the actual image reduction work
	 *----------------------------------------------------------------------*/

	build_output_image(vrast, srastbuf, swidth, sheight, bpr, srcblkw, srcblkh);

	/*------------------------------------------------------------------------
	 * draw the border box around the postage stamp we just built.	note
	 * that we draw the border around the full requested pstamp; this requires
	 * that we draw on the real output raster, not the virtual raster, since
	 * the virtual raster may be a smaller box centered within the output
	 * pstamp rectangle the caller requested.
	 *
	 * also note that we draw the border *after* the pstamp is generated,
	 * because the border actually wipes out the outer boundry pixels of
	 * the pstamp image.  doing it this way keeps the setup and loop control
	 * logic above a little cleaner.
	 *----------------------------------------------------------------------*/

	if (draw_border)
		draw_box((Rcel *)dscreen.pt, BORDER_COLOR_IDX, dxstart, dystart, dwidth, dheight);

ERROR_EXIT:

	if (allocbuf != NULL)
		free(allocbuf);

	return builtin_err;
}

int pstamp_difference(Popot screen1, int srcx, int srcy,
					  Popot screen2, int dx, int vy, int dw, int dh)
/*****************************************************************************
 * some day, this will return the weighted difference between a pair of
 * postage stamps.	a poco program that is, say, summarizing a flic into a
 * set of postage stamps could decide which frames to include in the
 * summary based on how different a postage stamp of the screen looks.
 * this would give a summary of 'significant action' in the flic.
 *
 * interesting idea, sounds real slow; maybe it'll get done someday.
 ****************************************************************************/
{
	return 1;
}

void init_pstamp_screen(Popot screen)
/*****************************************************************************
 * initialize the screen/raster upon which the postage stamps will be drawn.
 *
 * primarily, this consists of clearing the screen, and loading the screen's
 * palette with a standard 6-cube color map.
 ****************************************************************************/
{
	Rgb3 *ptab;
	Rcel *rast;
	int   r, g, b;

	if (NULL == (rast = screen.pt)) {
		builtin_err = Err_null_ref;
		return;
	}

	ptab = rast->cmap->ctab;
	ptab[BORDER_COLOR_IDX] = border_color_rgb;

	for (r=0; r<6; ++r) {
		for (g=0; g<6; ++g) {
			for (b=0; b<6; ++b) {
				ptab->r = RGB_MAX*r/6;
				ptab->g = RGB_MAX*g/6;
				ptab->b = RGB_MAX*b/6;
				++ptab;
			}
		}
	}

	pj_cmap_load(rast, rast->cmap);
	pj_set_rast(rast, 0);

	if (rast == GetPicScreen())
		poePicDirtied();
}

void cleanup_pstamp_screen(Popot screen)
/*****************************************************************************
 * if the screen the pstamps were being drawn onto is the main picscreen,
 * signal to PJ that it has been dirtied so that it will get recompressed.
 ****************************************************************************/
{
	if (screen.pt == GetPicScreen())
		poePicDirtied();
}

/*----------------------------------------------------------------------------
 * Setup rexlib/pocorex interface structures...
 *--------------------------------------------------------------------------*/

static Lib_proto calls[] = {
	{ init_pstamp_screen,	"void    InitPstampScreen(Screen *s);"},

	{ cleanup_pstamp_screen,"void    CleanupPstampScreen(Screen *s);"},

	{ make_pstamp,			"void    MakePstamp(Screen *source, Screen *dest,"
							" int dx, int dy, int dw, int dh, Boolean draw_border);"},

//	{ pstamp_difference,	"int     PstampDifference(Screen *s1, int srcx, int srcy,"
//							" Screen *s2, int dx, int vy, int dw, int dh);"},

};

Setup_Pocorex(NOFUNC, NOFUNC, "Postage Stamp Library", calls);

