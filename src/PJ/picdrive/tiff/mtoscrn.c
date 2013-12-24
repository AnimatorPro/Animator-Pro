/*****************************************************************************
 * MTOSCRN.C - PJ TIFF routines to unpack monoplane samples to the screen.
 ****************************************************************************/

#include "tiff.h"

#define MIN_BILEVEL_CONTRAST 45

static int luminanceof(Rgb3 *c)
/*****************************************************************************
 * return luminance of an rgb value.
 ***************************************************************************/
{
	int max, min;

	max = c->r;
	if (c->g>max) max = c->g;
	if (c->b>max) max = c->b;
	min = c->r;
	if (c->g<min) min = c->g;
	if (c->b<min) min = c->b;
	return (max+min)>>1;
}

static void toscreen_monochrome_palette(Tiff_file *tf)
/*****************************************************************************
 * build and set the PJ color palette before loading a monochrome image.
 *
 *	this routine is weird.	if the image is bilevel, we check the current
 *	fg/bg colors.  if either color is zero, and the other color has a low
 *	luminance, we set entries 0 & 1 in the color palette to black & white
 *	based on the photometric interpretation flag.  if both colors (0 & 1)
 *	are nonzero, or there is already a good contrast between them, we leave
 *	them alone, on the assumption the user has pre-set a special palette.
 *	(The main thrust here is to ensure the picture comes up visible on the
 *	screen without undoing something the user has set up special.)
 *
 *	if the image is greyscale, we just set a linear spread of black to white
 *	thru the first max_sample_value entries in the palette.  when we finally
 *	figure out how to interpret grey response curves values, they need to be
 *	factored in to the greyscale loop somehow.
 ****************************************************************************/
{
	int 	luminance0;
	int 	luminance1;
	int 	greylevel;
	int 	greydelta;
	int 	greystart;
	int 	counter;
	Rcel	*screen = tf->screen_rcel;
	Rgb3	black = {0,0,0};
	Rgb3	lgrey = {200,200,200};
	Rgb3	*ctab = screen->cmap->ctab;

	if (tf->bits_per_sample[0] == 1)
		{
		luminance0 = luminanceof(&ctab[0]);
		luminance1 = luminanceof(&ctab[1]);
		if ((luminance0 == 0 && luminance1 < MIN_BILEVEL_CONTRAST) ||
			(luminance1 == 0 && luminance0 < MIN_BILEVEL_CONTRAST) )
			{
			if (tf->photometric == PHMET_GREY_0ISWHITE)
				{
				ctab[0] = lgrey;
				ctab[1] = black;
				}
			else
				{
				ctab[0] = black;
				ctab[1] = lgrey;
				}
			}
		}
	else
		{
		greystart = 0;
		greydelta = 255 / (tf->max_sample_value);
		if (tf->photometric == 0)
			{
			greydelta = -greydelta;
			greystart = 255;
			}
		for (counter = 0, greylevel = greystart;
			 counter <= tf->max_sample_value;
			 ++counter, greylevel += greydelta)
			 {
			 ctab[counter].r = ctab[counter].g = ctab[counter].b = greylevel;
			 }
		}

	pj_cmap_load(screen, screen->cmap);
}

static void toscreen_monoplane_row(Tiff_file *tf, char *sourcep, char *wrkbuf)
/*****************************************************************************
 * output a bits-packed-into-bytes line to the screen.
 *
 *	this routine separates samples from the bit stream into bytes, and then
 *	writes the full line of bytes to the screen.  it can deal with any
 *	bits_per_sample value (up to 8), but requires that planar_configuration
 *	be 1 (contiguous bits), and that samples_per_pixel be 1 (monoplane data).
 *
 *	when bits_per_sample is 1, we use the fast pj_mask1blit routine to write
 *	the bitmap to the screen in a single operation.  when it's 8, we already
 *	have full bytes, so we just dump them to the screen.  for other values
 *	we unpack the samples into a byte buffer, then write the full buffer.
 ****************************************************************************/
{
	int 	width	= tf->width;
	int 	row 	= tf->image_row_cur;
	Rcel	*screen = tf->screen_rcel;

	switch (tf->bits_per_sample[0])
		{
		case 1:
			pj_mask1blit(sourcep, 32767, 0, 0, screen, 0, row, width, 1, 1);
			break;
		case 8:
			pj_put_hseg(screen, sourcep, 0, row, width);
			break;
		default:
			unpack_samples(sourcep, wrkbuf, width, tf->bits_per_sample[0]);
			pj_put_hseg(screen, wrkbuf, 0, row, width);
			break;
		}

	return;
}

static Errcode toscreen_monoplane_strip(Tiff_file *tf,
										 char *stripbuf,
										 char *decompressbuf,
										 char *samples2bytesbuf)
/*****************************************************************************
 * drive processing and writing to the screen each of the rows in a strip.
 *
 *	each line is decompressed (if needed), then passed to the ouput_row()
 *	routine for unpacking of samples into bytes and transfer to the screen.
 *
 *	note that if the compression type is LZW, we have already decompressed
 *	it before reaching this point; here we just treat it as uncompressed.
 ****************************************************************************/
{
	int 	bpr;
	int 	rowcount;
	int 	height = tf->height;
	int 	width = tf->width;
	int 	rows_per_strip = tf->rows_per_strip;
	int 	compression = tf->compression;

	bpr = ((width * tf->bits_per_sample[0]) + 7) >> 3; /* bytes per row */
	if (tf->compression == CMPRS_WNONE && (bpr & 0x01))
		++bpr;	/* force word alignment for this compression type */

	for (rowcount = 0;
		  (rowcount < rows_per_strip) && (tf->image_row_cur < height);
		  ++rowcount, ++tf->image_row_cur)
		{
		switch (compression)
			{
			case CMPRS_NONE:
			case CMPRS_WNONE:
			case CMPRS_LZW: /* unlzw done at a higher level */
				toscreen_monoplane_row(tf, stripbuf, samples2bytesbuf);
				stripbuf += bpr;
				break;
			case CMPRS_PACKBITS:
				if (NULL == (stripbuf = unpackbits(stripbuf, decompressbuf, bpr)))
					return Err_format;
				toscreen_monoplane_row(tf, decompressbuf, samples2bytesbuf);
				break;
			case CMPRS_1DHUFFMAN:
				if (NULL == (stripbuf = decmprs2(stripbuf, decompressbuf, width)))
					return Err_format;
				toscreen_monoplane_row(tf, decompressbuf, samples2bytesbuf);
				break;
			}
		}

	return Success;
}

Errcode toscreen_monoplane_image(Tiff_file *tf)
/*****************************************************************************
 * process and output each of the strips in an image.
 *
 *	this routine drives the process of writing an image to the screen.
 *	it allocates 2 row buffers for use by the lower level routines. (one
 *	decompression buffer, and one for unpacking samples into byte values.)
 *	note that these are allocated one byte bigger than the row width, in case
 *	we need a pad byte for data stored in 'uncompressed word-aligned' (32771)
 *	format.  after getting the buffers, this routine loops through all the
 *	strips in the image, calling read_strip() then output_strip() for each.
 *
 ****************************************************************************/
{
	char		*stripbuf = NULL;
	char		*wrkbuf1  = NULL;
	char		*wrkbuf2  = NULL;
	UBYTE		asciitab[256];
	Strip_data	*curstrip = tf->strip_data;
	Errcode 	err;
	int 		counter;
	int 		lzwbuflen;

	/*------------------------------------------------------------------------
	 * allocate i/o, decompression, and line buffers...
	 *----------------------------------------------------------------------*/

	if (NULL == (stripbuf = malloc(tf->longest_strip)))
		{
		err = Err_no_memory;
		goto ERROR_EXIT;
		}

	if (NULL == (wrkbuf1 = malloc(2*tf->width+2)))
		{
		err = Err_no_memory;
		goto ERROR_EXIT;
		}
	wrkbuf2 = wrkbuf1 + tf->width+1;


	/*------------------------------------------------------------------------
	 * set the pj color palette based on the type of tiff we're reading...
	 *----------------------------------------------------------------------*/

	if (tf->photometric == PHMET_PALETTE_COLOR)
		{
		memcpy(tf->screen_rcel->cmap->ctab, tf->color_table,3*COLORS);
		pj_cmap_load(tf->screen_rcel, tf->screen_rcel->cmap);
		}
	else
		{
		toscreen_monochrome_palette(tf);
		}

	/*------------------------------------------------------------------------
	 * if the compression type if lzw, allocate the extra buffers and data
	 * structures we need to cope with it.
	 *----------------------------------------------------------------------*/

	if (tf->compression == CMPRS_LZW)
		{
		lzwbuflen = calc_maxdata(tf);
		if (NULL == (tf->lzwbuf = malloc(lzwbuflen)))
			{
			err = Err_no_memory;
			goto ERROR_EXIT;
			}
		if (NULL == (tf->unlzwtable = malloc(UNLZW_TABLE_SIZE)))
			{
			err = Err_no_memory;
			goto ERROR_EXIT;
			}
		unlzw_init(tf->unlzwtable);
		}

	/*------------------------------------------------------------------------
	 * loop to process strips in the file...
	 * if the compression type is lzw, we have to decompress the entire strip
	 * then pass it along to the lower level screen output routines, which
	 * will treat the data as uncompressed (which it is, at that point).
	 * for the other compression schemes, the lower level screen output
	 * routines will invoke the proper decompressor.  this is because lzw
	 * compression is oriented to strip boundries, and the other compression
	 * methods are oriented to line boundries.
	 *----------------------------------------------------------------------*/

	tf->image_row_cur = 0;
	if (tf->compression == CMPRS_LZW)
		{
		for (counter = 0; counter < tf->strips_per_image; ++counter, ++curstrip)
			{
			if (Success != (err = read_strip(tf, stripbuf, curstrip)))
				goto ERROR_EXIT;
			if (Success != (err = unlzw(stripbuf, tf->lzwbuf, tf->unlzwtable, lzwbuflen)))
				goto ERROR_EXIT;
			if (Success != (err = toscreen_monoplane_strip(tf, tf->lzwbuf, wrkbuf1, wrkbuf2)))
				goto ERROR_EXIT;
			}
		}
	else
		{
		for (counter = 0; counter < tf->strips_per_image; ++counter, ++curstrip)
			{
			if (Success != (err = read_strip(tf, stripbuf, curstrip)))
				goto ERROR_EXIT;
			if (Success != (err = toscreen_monoplane_strip(tf, stripbuf, wrkbuf1, wrkbuf2)))
				goto ERROR_EXIT;
			}
		}

	err = Success;

ERROR_EXIT:

	if (stripbuf != NULL)
		free(stripbuf);
	if (wrkbuf1 != NULL)
		free(wrkbuf1);

	return err;
}
