/*****************************************************************************
 * MFRMSCRN.C - PJ TIFF routines to process monoplane data from the screen.
 ****************************************************************************/

#include "tiff.h"

static void fromscreen_monochrome_palette(Tiff_file *tf)
/*****************************************************************************
 * make a color-to-greyscale translation table.
 ****************************************************************************/
{
	int 	counter;
	UBYTE	*xltab = tf->color_table;
	Rgb3	*ctab  = tf->screen_rcel->cmap->ctab;

	for (counter = 0; counter < COLORS; ++counter, ++xltab, ++ctab)
		*xltab = (ctab->r + ctab->g + ctab->b) / 3;
}

static int fromscreen_monoplane_row(Tiff_file *tf,
									 char *destp,
									 char *linebuf,
									 char *rgbbuf)
/*****************************************************************************
 * get current line from the screen, xlate & pack it, store in strip buffer.
 *
 * a little strangeness:
 *	 packbits is a line-oriented compression scheme; runs do not span lines.
 *	 (uncompressed data can be considered a degenerate case of the same
 *	 concept).	lzw on the other hand is strip-oriented, and compression does
 *	 span lines.  thus, the lzw compressor must keep track of its current
 *	 location in the strip buffer (because it's a bit-offset location, and
 *	 we keep track of byte offsets here).  to kludge around all this, we
 *	 return a '1' as the 'packed length' of each lzw-compressed line we
 *	 process here.	the '1' keeps our caller happy (signals 'no error').
 *	 our caller will obtain the true compressed length via a call to
 *	 lzw_endstrip() after all rows in the strip have been processed, so the
 *	 bogus line length of '1' we return for lzw is basically ignored.
 ****************************************************************************/
{
	int 	width = tf->width;
	int 	packed_length;

	/*------------------------------------------------------------------------
	 * get the line of data from the screen...
	 *---------------------------------------------------------------------*/

	pj_get_hseg(tf->screen_rcel, linebuf, 0, tf->image_row_cur, width);

	/*------------------------------------------------------------------------
	 * do photometric processing on the line...
	 *	- for color mapped output we do nothing, the data is already mapped.
	 *	- for greyscale, we translate the color index values to greyscale
	 *	  index values via the translation table we built earlier.
	 *	- for rgb output, we translate the color index values to rgb triplets.
	 *	- any other value indicates we've gone insane.
	 *----------------------------------------------------------------------*/

	switch (tf->photometric)
		{
		case PHMET_PALETTE_COLOR:
			break;

		case PHMET_GREY_0ISBLACK:
			xlatebuffer(tf->color_table, linebuf, linebuf, width);
			break;

		case PHMET_RGB:
			xlate2rgb(tf->screen_rcel->cmap->ctab, linebuf, rgbbuf, width);
			width *= 3; 		/* we tripled the size of the data */
			linebuf = rgbbuf;	/* adjust pointer for compressor routine */
			break;

		default:
			return Err_driver_protocol;
		}

	/*------------------------------------------------------------------------
	 * do compression processing on the line...
	 *----------------------------------------------------------------------*/

	switch (tf->compression)
		{
		case CMPRS_NONE:
		case CMPRS_LZW: 	/* lzw compression done at higher level */
			memcpy(destp, linebuf, width);
			packed_length = width;
			break;

		case CMPRS_PACKBITS:
			packed_length = packbits(linebuf, destp, width);
			break;

		default:
			return Err_driver_protocol; /* we're lost */
		}

	return packed_length;
}

static int fromscreen_monoplane_strip(Tiff_file *tf,
										char *destp,
										char *linebuf,
										char *rgbbuf)
/*****************************************************************************
 * process each of the rows in a strip, return total number bytes in strip.
 *	returns zero if a sanity check fails (eg, packed data length < 0)
 ****************************************************************************/
{
	int 	rowcount;
	int 	rowlen;
	int 	striplen = 0;
	int 	height	 = tf->height;
	int 	rows_per_strip = tf->rows_per_strip;

	for (rowcount = 0;
		  (rowcount < rows_per_strip) && (tf->image_row_cur < height);
		  ++rowcount, ++tf->image_row_cur)
		{
		rowlen = fromscreen_monoplane_row(tf, destp, linebuf, rgbbuf);
		striplen += rowlen;
		destp += rowlen;
		if (rowlen <= 0 || striplen <= 0 || striplen > tf->longest_strip)
			{
			return Err_driver_protocol; /* oops, better bail out...should never happen */
			}
		}

	return striplen;
}

Errcode fromscreen_monoplane_image(Tiff_file *tf, int photometric, int compression)
/*****************************************************************************
 * drive the process of writing a monoplane image to a file.
 *
 *	on entry the following fields of the Tiff_file passed in must be valid:
 *	  tf->file
 *	  tf->width
 *	  tf->height
 *	everything else is initialized herein, based on width & height.
 *
 *	before entry to this routine, the file must have been opened, and the
 *	file header written, leaving the current file offset pointing to the byte
 *	immediately following the file header (ie, ready for image data). this
 *	is currently accomplished by the create_file() routine.
 *
 *	this routine drives the process of writing all the image data to the
 *	file. once all the image data is written, the tags & such are dumped, and
 *	the file header is rewritten to plug in a valid offset-to-first-ifd value.
 ****************************************************************************/
{
	Errcode err;
	int 	strip_counter;
	int 	striplen;
	int 	rps;
	char	*stripbuf = NULL;
	char	*linebuf  = NULL;
	char	*rgbbuf   = NULL;
	int 	width = tf->width;

	/*------------------------------------------------------------------------
	 * fill in tiff data values we'll need later when dumping the tags...
	 *----------------------------------------------------------------------*/

	tf->compression = compression;
	tf->photometric = photometric;
	tf->planar_configuration = 1;
	tf->min_sample_value	 = 0;
	tf->max_sample_value	 = 255;

	if (photometric == PHMET_RGB)
		{
		width *= 3; 	/* adjust width used in calculating buffer sizes */
		tf->samples_per_pixel	 = 3;
		tf->bits_per_sample[0]	 = 8;
		tf->bits_per_sample[1]	 = 8;
		tf->bits_per_sample[2]	 = 8;
		}
	else
		{
		tf->samples_per_pixel	 = 1;
		tf->bits_per_sample[0]	 = 8;
		}

	/*------------------------------------------------------------------------
	 * calc rows-per-strip, strips-per-image, and strip buffer size.
	 * we try to keep strips smallish, but not too small.  one of our output
	 * compression schemes (packbits) has a worst-case behavior of one
	 * extra byte of output per 128 bytes of input, so we factor that
	 * (plus an extra line, just to be safe) into the strip buffer size.
	 * (lzw has a worse worst-case, but lzw compression is done into a
	 * separate buffer, which is allocated later as 2*longest_strip).
	 *----------------------------------------------------------------------*/

	tf->rows_per_strip	 = (rps = OUTPUT_IDEAL_STRIPSZ / width);
	tf->strips_per_image = (tf->height + rps - 1) / rps;
	tf->longest_strip	 = ((rps + 1) * width) + (rps * ((width + 127) / 128));
	tf->image_row_cur	 = 0;

	/*-----------------------------------------------------------------------
	 * aquire local-use buffers...
	 *	these buffers will get free'd before this function exits.
	 *----------------------------------------------------------------------*/

	err = Err_no_memory;	/* if anything fails, it will be this... */

	if (NULL == (stripbuf = malloc(tf->longest_strip)))
		goto ERROR_EXIT;

	if (NULL == (linebuf = malloc(width)))
		goto ERROR_EXIT;

	if (photometric == PHMET_RGB)
		if (NULL == (rgbbuf = malloc(width)))
			goto ERROR_EXIT;

	/*------------------------------------------------------------------------
	 * aquire global-use buffers...
	 * these buffers will get free'd by the close_file() function.
	 *----------------------------------------------------------------------*/

	if (NULL == (tf->strip_data = malloc(tf->strips_per_image * sizeof(Strip_data))))
		goto ERROR_EXIT;

	if (compression == CMPRS_LZW)
		{
		if (NULL == (tf->lzwbuf = malloc(2*tf->longest_strip)))
			goto ERROR_EXIT;
		lzw_init(2*tf->longest_strip);
		}

	/*-----------------------------------------------------------------------
	 * go make the color->monochrome translation table if output is greyscale
	 *----------------------------------------------------------------------*/

	if (photometric == PHMET_GREY_0ISBLACK)
		{
		if (NULL == (tf->color_table = malloc(COLORS)))
			goto ERROR_EXIT;
		fromscreen_monochrome_palette(tf);
		}

	/*------------------------------------------------------------------------
	 * loop once for each strip until the entire file is written.
	 *	Note that the loop counts strips, but is terminated based on the
	 *	row counter.  The row counter is incremented by the lower level
	 *	routines.  This is compensates for the fact that our fudge factor in
	 *	calc'ing the buffer size for worst-case compression could make
	 *	strips_per_image inaccurate.
	 *----------------------------------------------------------------------*/

	for (strip_counter = 0; tf->image_row_cur < tf->height; ++strip_counter)
		{
		if (0 >= (err = striplen = fromscreen_monoplane_strip(tf, stripbuf, linebuf, rgbbuf)))
			goto ERROR_EXIT;

		if (compression == CMPRS_LZW)
			{
			if (Success > (err = striplen = lzw_compress(stripbuf, tf->lzwbuf, striplen)))
				goto ERROR_EXIT;
			if (Success != (err = write_strip(tf, tf->lzwbuf, striplen, strip_counter)))
				goto ERROR_EXIT;
			}
		else
			{
			if (Success != (err = write_strip(tf, stripbuf, striplen, strip_counter)))
				goto ERROR_EXIT;
			}
		}

	/*
	 * go dump the tif tags (ifd) and the associated strip offsets & sizes,
	 * then rewrite the file header to update the ifd offset field in it.
	 */

	if (Success != (err = write_tiftags(tf, stripbuf)))
		goto ERROR_EXIT;
	if (Success != (err = write_filehdr(tf)))
		goto ERROR_EXIT;
	err = Success;

ERROR_EXIT:

	if (stripbuf != NULL)
		free(stripbuf);
	if (linebuf !=NULL)
		free(linebuf);
	if (rgbbuf != NULL)
		free(rgbbuf);
	if (compression == CMPRS_LZW)
		lzw_cleanup();

	return err;
}
