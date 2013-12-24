/*****************************************************************************
 *
 ****************************************************************************/

#include "tiff.h"

static Errcode init_rgb_stuff(Tiff_file *tf)
/*****************************************************************************
 *
 ****************************************************************************/
{
	long maxdata;

	/*------------------------------------------------------------------------
	 * get a strip I/O buffer and a decompression buffer.
	 * if the file format is multiplanar, also get separate r,g,b buffers.
	 *----------------------------------------------------------------------*/

	maxdata = calc_maxdata(tf);

	if (NULL == (tf->lzwbuf = malloc(tf->longest_strip)))
		return Err_no_memory;

	if (tf->planar_configuration == 1)
		{
		if (NULL == (tf->stripbuf = malloc(maxdata)))
			return Err_no_memory;
		}
	else
		{
		if (NULL == (tf->stripbuf = malloc(3*maxdata)))
			return Err_no_memory;
		if (NULL == (tf->rbuf = malloc(3*maxdata)))
			return Err_no_memory;
		tf->gbuf = tf->rbuf + maxdata;
		tf->bbuf = tf->gbuf + maxdata;
		}

	/*------------------------------------------------------------------------
	 * if the compression type is LZW, allocate & init the unlzw table...
	 *----------------------------------------------------------------------*/

	if (tf->compression == CMPRS_LZW)
		{
		if (NULL == (tf->unlzwtable = malloc(UNLZW_TABLE_SIZE)))
			return Err_no_memory;
		unlzw_init(tf->unlzwtable);
		}

	/*------------------------------------------------------------------------
	 * if the data is stored in multiplanar format, set up the strip_data
	 * pointers for each separate plane...
	 *----------------------------------------------------------------------*/

	if (tf->planar_configuration == 2)
		{
		int strips_per_plane = tf->strips_per_image / 3;

		tf->rstrip_data = tf->strip_data;
		tf->gstrip_data = tf->rstrip_data + strips_per_plane;
		tf->bstrip_data = tf->gstrip_data + strips_per_plane;
		}

	return Success;
}

static Errcode rgb_unpackbits(Tiff_file *tf, UBYTE *source, UBYTE *dest, int width, int rowcount)
/*****************************************************************************
 *
 ****************************************************************************/
{
	int 	row;

	for (row = 0; row < rowcount; ++row)
		{
		if (NULL == (source = unpackbits(source, dest, width)))
			return Err_format;
		dest += width;
		}

	return Success;
}

static Errcode rgb_decompress(Tiff_file *tf, UBYTE *dest, int width, int rows)
/*****************************************************************************
 *
 ****************************************************************************/
{

	switch (tf->compression)
		{
		case CMPRS_LZW:

			unlzw(tf->lzwbuf, dest, tf->unlzwtable, width*rows);
			return Success;

		case CMPRS_PACKBITS:

			return rgb_unpackbits(tf, tf->lzwbuf, dest, width, rows);

		case CMPRS_NONE:
		case CMPRS_WNONE:

			memcpy(dest, tf->lzwbuf, width*rows);
			return Success;

		default:

			return Err_driver_protocol;
		}
}

static Errcode refill_rgb_buffers(Tiff_file *tf)
/*****************************************************************************
 * read the next chunk of rgb data into stripbuf, decompressing if needed.
 ****************************************************************************/
{
	Errcode err;
	int 	dwidth;
	int 	rowcount;
	int 	col;
	int 	row;
	UBYTE	*databuf;
	UBYTE	*r;
	UBYTE	*g;
	UBYTE	*b;

	if ((tf->image_row_cur + tf->rows_per_strip) > tf->height)
		rowcount = tf->height - tf->image_row_cur;
	else
		rowcount = tf->rows_per_strip;

	if (tf->planar_configuration == 1)	   /* monoplane data */
		{
		/*--------------------------------------------------------------------
		 * monoplane rgb is easy...
		 *------------------------------------------------------------------*/
		dwidth = 3*tf->width;
		if (Success != (err = read_strip(tf, tf->lzwbuf, &tf->strip_data[tf->strip_index])))
			 return err;
		if (Success != (err = rgb_decompress(tf, tf->stripbuf, dwidth, rowcount)))
			return err;
		}
	else								   /* multiplane data */
		{
		dwidth = tf->width;
		/*--------------------------------------------------------------------
		 * read each of the planes into its own buffer and decompress...
		 *------------------------------------------------------------------*/
		if (Success != (err = read_strip(tf, tf->lzwbuf, &tf->rstrip_data[tf->strip_index])))
			return err;
		if (Success != (err = rgb_decompress(tf, tf->rbuf, dwidth, rowcount)))
			return err;
		if (Success != (err = read_strip(tf, tf->lzwbuf, &tf->gstrip_data[tf->strip_index])))
			return err;
		if (Success != (err = rgb_decompress(tf, tf->gbuf, dwidth, rowcount)))
			return err;
		if (Success != (err = read_strip(tf, tf->lzwbuf, &tf->bstrip_data[tf->strip_index])))
			return err;
		if (Success != (err = rgb_decompress(tf, tf->bbuf, dwidth, rowcount)))
			return err;

		/*--------------------------------------------------------------------
		 * splice the buffers together into rgb triplets...
		 *------------------------------------------------------------------*/
		r = tf->rbuf;
		g = tf->gbuf;
		b = tf->bbuf;
		databuf = tf->stripbuf;
		for (row = 0; row < rowcount; ++row)
			{
			for (col = 0; col < tf->width; ++col)
				{
				*databuf++ = *r++;
				*databuf++ = *g++;
				*databuf++ = *b++;
				}
			}
		}

	++tf->strip_index;
	tf->rows_in_buffer	 = rowcount;
	tf->offset_in_buffer = 0;

	return Success;
}

Errcode rgb_readline(Tiff_file *tf, Rgb3 *linebuf)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;

	if (tf->image_row_cur >= tf->height)
		return Err_truncated;					/* should never happen */

	if (tf->rows_in_buffer == 0)
		{
		if (Success != (err = refill_rgb_buffers(tf)))
			return err;
		}

	memcpy(linebuf, tf->stripbuf+tf->offset_in_buffer, 3*tf->width);

	tf->image_row_cur	 += 1;
	tf->rows_in_buffer	 -= 1;
	tf->offset_in_buffer += 3*tf->width;

	return Success;
}

Errcode rgb_seekstart(Tiff_file *tf)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;

	/*------------------------------------------------------------------------
	 * the first time we are called for a given file, we have to allocate
	 * buffers we'll need for processing the file.
	 *----------------------------------------------------------------------*/

	if (NULL == tf->stripbuf)
		{
		if (Success != (err = init_rgb_stuff(tf)))
			return err;
		}

	/*------------------------------------------------------------------------
	 * reset values so that the next read will get the first line of data.
	 *----------------------------------------------------------------------*/

	tf->image_row_cur  = 0; 		/* first row, */
	tf->strip_index    = 0; 		/* in first strip */
	tf->rows_in_buffer = 0; 		/* force buffer reload on next read */

	return 0;	/* retval 0 indicates image is not upside down in file */
}
