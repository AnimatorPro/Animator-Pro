/*****************************************************************************
 * TGAGWRIT.C - Routines to write headers and data to a targa file.
 ****************************************************************************/

#include "memory.h"
#include "targa.h"

#define MAXRUN 128

Errcode write_targa_header(Targa_file *tf)
/*****************************************************************************
 * fill in the fields in the file header structure and write it
 ****************************************************************************/
{
	Tgaheader *th = &tf->tgahdr;
	Imgspec   *im = &tf->tgahdr.imgdata;
	Tcmap	  *cm = &tf->tgahdr.tcmdata;
	Cmap	  *pcmap = tf->screen_rcel->cmap;
	Rgb3	  *incolor;
	Bgr3	  outcolor;
	int 	  i;

	im->xorg	 = 0;
	im->yorg	 = 0;
	im->width	 = tf->width;
	im->height	 = tf->height;
	im->pixsiz	 = tf->pdepth;	/* pixel depth */
	im->imgdesc  = 0;			/* image is bottom-left */
	th->idlength = 0;			/* no id string */

	if (tf->is_rgb)
		{
		th->maptype  = MAP_NONE;		   /* no color map */
		if (tf->is_compressed)
			th->imgtype = RL_RGB_IMAGE;
		else
			th->imgtype = RGB_IMAGE;
		}
	else /* not rgb, it's mapped */
		{
		th->maptype = MAP_LUT;
		cm->mapidx	= 0;
		cm->mapcnt	= COLORS;
		cm->mapbits = 24;
		if (tf->is_compressed)
			th->imgtype = RL_MAPPED_IMAGE;
		else
			th->imgtype = MAPPED_IMAGE;
		}

	if (1 != xfwrite(th, sizeof(*th), 1, tf->file))
		return xerrno();

	if (!tf->is_rgb)	/* write color map if not rgb file */
		{
		incolor = pcmap->ctab;
		for (i = 0; i < COLORS; ++i, ++incolor)
			{
			outcolor.r = incolor->r;
			outcolor.g = incolor->g;
			outcolor.b = incolor->b;
			if (1 != xfwrite(&outcolor, sizeof(outcolor), 1, tf->file))
				return xerrno();
			}
		}

	return Success;
}

static Pixel get_next_pixel(Targa_file *tf)
/*****************************************************************************
 * get the next pixel from the screen.
 *	we grab a line at a time and dole out characters from a buffer to keep
 *	down the raster call overhead.	there is no checking for end-of-screen,
 *	it is the caller's responsibility not to call this routine more than
 *	width*height times.
 ****************************************************************************/
{
	if (++tf->curx >= tf->width)
		{
		tf->curx = 0;
		--tf->cury;
		pj_get_hseg(tf->screen_rcel, tf->hsegbuf, 0, tf->cury, tf->width);
		}
	return tf->hsegbuf[tf->curx];
}

static Errcode dump_run(Targa_file *tf, Pixel color, int runlength)
/*****************************************************************************
 * dump a run of pixels to the output file.
 *
 *	this is used only for compressed data; this routine doesn't get called
 *	for uncompressed output. the run may be of any arbitrary length, so this
 *	routine breaks them up into runs of 128 bytes or less, and outputs as
 *	many of these run packets as necessary to express the full run.
 *
 * for rgb output:
 *	the color index of the repeated pixel is translated to its rgb values
 *	for output.  note that targa files store samples in bgr order not rgb.
 * for mapped output:
 *	two bytes are written: the run length then the pixel byte.
 ****************************************************************************/
{
	struct {
		 UBYTE length;
		 UBYTE b;
		 UBYTE g;
		 UBYTE r;
		} rundata;
	Rgb3	*rgbcolor;
	int 	output_size;

	if (tf->is_rgb)
		{
		rgbcolor  = &tf->screen_rcel->cmap->ctab[color];
		rundata.r = rgbcolor->r;
		rundata.g = rgbcolor->g;
		rundata.b = rgbcolor->b;
		output_size = 4;		/* runlength byte + 3 bytes color info */
		}
	else
		{
		rundata.b = color;
		output_size = 2;		/* runlength byte + color index byte */
		}

	while (runlength > 0)
		{
		if (runlength > MAXRUN)
			{
			rundata.length = (MAXRUN-1) | 0x80;
			runlength -= MAXRUN;
			}
		else
			{
			rundata.length = (runlength-1) | 0x80;
			runlength = 0;
			}
		if (1 != xfwrite(&rundata, output_size, 1, tf->file))
			return xerrno();
		}

	return Success;
}

static Errcode dump_literal(Targa_file *tf, Pixel *pbuf, int runlength)
/*****************************************************************************
 * dump a sequence of literal (non-run) data into the output file.
 *
 * note that this routine is used to write literal runs of compressed data,
 * or to write uncompressed data.  the only difference is that compressed
 * data gets a literal count inserted into the output stream.
 *
 * for rgb output:
 *	the color index of the each pixel is translated to its rgb values
 *	for output.  note that targa files store samples in bgr order not rgb.
 * for mapped output:
 *	the pixels are output as a sequence of bytes, without translation.
 ****************************************************************************/
{
	UBYTE	rlen = runlength - 1;
	Rgb3	*ctab = tf->screen_rcel->cmap->ctab;
	Rgb3	*rgbcolor;
	struct {
		UBYTE b;
		UBYTE g;
		UBYTE r;
		} outdata;

	if (tf->is_compressed)
		{
		if (1 != xfwrite(&rlen, sizeof(rlen), 1, tf->file))
			return xerrno();
		}

	if (tf->is_rgb)
		{
		while (runlength--)
			{
			rgbcolor = &ctab[*pbuf++];
			outdata.r = rgbcolor->r;
			outdata.g = rgbcolor->g;
			outdata.b = rgbcolor->b;

			if (1 != xfwrite(&outdata, sizeof(outdata), 1, tf->file))
				return xerrno();
			}
		}
	else
		{
		if (1 != xfwrite(pbuf, runlength, 1, tf->file))
			return xerrno();
		}

	return Success;
}

static Errcode write_compressed_image(Targa_file *tf)
/*****************************************************************************
 * drive the process of writing a compressed targa file from the screen image.
 *
 *	this routine loops through the pixels on the screen doing run-length
 *	compression on the data, then invoking the dump_xxx() routines to
 *	translate the pixel color indicies to rgb values and write them.
 *
 *	the rle format is similar to macintosh packbits, except that runs can
 *	span image lines, and what is encoded is runs of pixels (rgb values) not
 *	runs of individual bytes.
 ****************************************************************************/
{
	Errcode err;
	Pixel	curpix;
	Pixel	lastpix;
	int 	bufcnt;
	int 	runcnt = 0;
	Pixel	*buf = NULL;
	long	pixcount = tf->width * tf->height;

	if (NULL == (buf = pj_malloc(tf->width)))
		{
		err = Err_no_memory;
		goto ERROR_EXIT;
		}

	tf->curx  = tf->width;/* prime curx to something larger than line width */
	tf->cury  = tf->height;	/* to force a buffer load on 1st get_next_pixel() */

	buf[0] = lastpix = get_next_pixel(tf);
	bufcnt = 1;
	while (--pixcount)
		{
		curpix = get_next_pixel(tf);
		if (curpix == lastpix)
			{
			if (bufcnt != 0)	/* if there is one pixel in the buffer, it is */
				{				/* lastpix, which we know is part of a run.   */
				if (bufcnt > 1) /* if there's more than one pixel, we have to */
					{			/* dump the literal sequence in the buffer.   */
					if (0 != (err = bufcnt = dump_literal(tf, buf, bufcnt-1)))
						goto ERROR_EXIT;
					}
				bufcnt = 0; 	/* literal buffer is now empty */
				}
			++runcnt;			/* count the run character */
			}
		else	/* curpix != lastpix, start or continue literal sequence */
			{
			if (runcnt != 0) /* if run, dump it before starting literal */
				{
				if (0 != (err = runcnt = dump_run(tf, lastpix, runcnt+1)))
					goto ERROR_EXIT;
				}
			if (bufcnt == MAXRUN)	/* if literal buffer is full, dump it */
				{
				if (0 != (err = bufcnt = dump_literal(tf, buf, bufcnt)))
					goto ERROR_EXIT;
				}
			buf[bufcnt++] = curpix;
			}
		lastpix = curpix;
		}

	if (bufcnt != 0)		/* all done. if ended on a literal, dump it. */
		{
		if (0 != (err = bufcnt = dump_literal(tf, buf, bufcnt)))
			goto ERROR_EXIT;
		}
	else if (runcnt != 0)	/* else if ended on a run, dump that. */
		{
		if (0 != (err = runcnt = dump_run(tf, lastpix, runcnt+1)))
			goto ERROR_EXIT;
		}

	err = Success;

ERROR_EXIT:

	if (buf != NULL)
		pj_free(buf);
	return err;

}

static Errcode write_uncompressed_image(Targa_file *tf)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	int 	y;
	int 	w = tf->width;
	int 	h = tf->height;

	for (y = h; --y >= 0; )
		{
		pj_get_hseg(tf->screen_rcel, tf->hsegbuf, 0, y, w);
		if (Success != (err = dump_literal(tf, tf->hsegbuf, w)))
			return err;
		}

	return Success;
}

Errcode write_targa_image(Targa_file *tf)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (tf->is_compressed)
		return write_compressed_image(tf);
	else
		return write_uncompressed_image(tf);
}

