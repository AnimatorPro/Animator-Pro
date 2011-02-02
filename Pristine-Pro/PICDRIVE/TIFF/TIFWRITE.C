/*****************************************************************************
 * TIFWRITE.C - TIFF driver routines to write tif tags, file header, strips.
 ****************************************************************************/

#include "tiff.h"

static Errcode write_colormap(Tiff_file *tf, long *mapoffset)
/*****************************************************************************
 * write the PJ color palette as a TIFF color map, return oset of map in file.
 *
 * Strangeness dept:
 *	The tiff 5.0 spec says that values in the color map range from 0-65535 to
 *	indicate relative intensity of the color.  That seems cut-and-dried to me:
 *	since our values range from 0-255, we need to shift our values up by 8
 *	bits to scale them to tiff's range.  However, it appears that most tiff
 *	readers don't downscale the values when they read the color map and load
 *	the VGA palettes.  In fact, most readers wig out totally if any of the
 *	values are > 255.  So <major sigh> we just write the values out in the
 *	0-255 range, and hope for tiff readers that incorporate the sort of
 *	smarts our read side has in coping with either 255 level or 64k level
 *	color values.
 *
 *	(An aside:	I attempted to fix this by replicating the color byte into
 *	both words of the output value; color 0x23 would be written as 0x2323,
 *	and so on.	Seemed like a fine idea:  a reader would either downscale,
 *	getting 0x23, or would ignore the upper byte, getting 0x23.  Didn't work.)
 *
 * 09/24/92 -
 *	Well, the TIFF 6.0 spec is out now, and it's much clearer on this issue:
 *	the values have to be scaled into a 0-64k range.  ("Black is represented
 *	by 0,0,0; white by 65535,65535,65535.")  For us, scaling is easy, we just
 *	shift our 0-255 range value up 8 bits when we store it.  On the read side,
 *	we still handle both types of color palette data, since it's so easy to
 *	do, and there're definitely programs out there writing unscaled palettes.
 ****************************************************************************/
{
	USHORT	curcolor;
	USHORT	colormap[3][256];
	UBYTE	(*ctab)[3] = (void *)tf->screen_rcel->cmap->ctab;
	int 	plane;
	int 	index;

	for (plane = 0; plane < 3; ++plane)
		for (index = 0; index < 256; ++index)
			{
			curcolor = ctab[index][plane];
			colormap[plane][index] = curcolor << 8;
			}

	if (0 >= (*mapoffset = ftell(tf->file)))
		return pj_errno_errcode();

	if (1 != fwrite(colormap, sizeof(colormap), 1, tf->file))
		return Err_truncated;

	return Success;
}

static Errcode write_stripdata_array(Tiff_file *tf, char *wrkbuf,
									 long *offsets,
									 long *counts)
/*****************************************************************************
 * unload strip_data array to buffer, write the buffer, return file offsets.
 ****************************************************************************/
{
	int 		strip_counter;
	int 		bytes_per_array;
	Strip_data	*curstrip;
	long		curoffset;
	long		*buffer1;
	long		*buffer2;

	/*
	 * first unload the data from the strip_data structures into a pair
	 * of arrays, one for strip offsets, and one for strip bytecounts.
	 */

	bytes_per_array = tf->strips_per_image << 2; /* four bytes per entry */
	buffer1 = (long *)wrkbuf;
	buffer2 = (long *)(wrkbuf+bytes_per_array);
	if ((2 * bytes_per_array) > tf->longest_strip)
		return Err_driver_protocol; /* should never happen */

	curstrip = tf->strip_data;
	strip_counter = tf->strips_per_image;
	while (strip_counter--)
		{
		*buffer1++ = curstrip->offset;
		*buffer2++ = curstrip->size;
		++curstrip;
		}

	/*
	 * remember the current location in the file, then dump the pair of
	 * buffers (which are contiguous) into the file at the current location.
	 */

	if (0 >= (curoffset = ftell(tf->file)))
		return pj_errno_errcode();

	if (2 != fwrite(wrkbuf, bytes_per_array, 2, tf->file))
		return Err_truncated;

	/*
	 * return the file offsets of the strip offsets table and strip bytecounts
	 * table within the file, via the pointers provided by the caller.
	 */

	*offsets = curoffset;
	*counts  = curoffset + bytes_per_array;

	return Success;
}

static Errcode write_next_dirent(Tiff_file *tf, short tag, short type,
								 long count, long offset)
/*****************************************************************************
 * write a directory entry (tag, count, type, offset).
 ****************************************************************************/
{
	Dirent	entry;

	entry.tag	= tag;
	entry.type	= type;
	entry.count = count;

	if (type == TYPE_SHORT && count > 2)
		type = TYPE_LONG;	/* so that we write a long offset into dirent */

	switch (type)
		{
		case TYPE_SHORT:
			entry.value.dword = 0;			/* clear upper word */
			entry.value.word  = offset; 	/* data into lower word */
			break;
		default:
			entry.value.dword = offset;
			break;
		}

	if (1 != fwrite(&entry, sizeof(Dirent), 1, tf->file))
		return pj_errno_errcode();

	return Success;
}

Errcode write_tiftags(Tiff_file *tf, char *wrkbuf)
/*****************************************************************************
 * dump out all the tif tags.
 ****************************************************************************/
{
#define NUM_BASIC_TAGS 13
#define W(a,b,c,d)	if (Success != (err = write_next_dirent(tf, a, b, c, d))) \
						return err;

	Errcode err;
	long	bps_at;
	int 	bpscount;
	long	colormap_at;
	long	stripoffsets_at;
	long	stripsizes_at;
	long	a_longzero = 0;
	short	entrycount = NUM_BASIC_TAGS;
	int 	stripcount = tf->strips_per_image;

	/*
	 * first we have to write out the strip offsets and bytecounts, so that
	 * we'll have the offsets to those tables in the file to use when writing
	 * the corresponding tags to the ifd.
	 *
	 * note that if we only wrote one strip, we don't have to dump the
	 * offsets and counts into a separate area of the file, we can just
	 * embed the values into the tag entries in the ifd.
	 */

	if (stripcount == 1)
		{
		stripoffsets_at = tf->strip_data[0].offset;
		stripsizes_at	= tf->strip_data[0].size;
		}
	else
		{
		if (Success !=
		   (err = write_stripdata_array(tf, wrkbuf, &stripoffsets_at, &stripsizes_at)))
			return err;
		}

	/*
	 * next we write the colormap if the photometric interp indicates we're
	 * writing a colormapped file.	in this case, the number of directory
	 * entries is also increased by one, to account for the extra tag we
	 * have to write later.
	 *
	 * if we're doing an RGB file, we write the bits-per-sample array
	 * (3 values) instead of a colormap.
	 */

	switch (tf->photometric)
		{
		case PHMET_RGB:
			bpscount = 3;
			if (0 >= (bps_at = ftell(tf->file)))
				return pj_errno_errcode();
			if (1 != fwrite(tf->bits_per_sample, sizeof(tf->bits_per_sample),
							1, tf->file))
				return Err_truncated;
			break;
		case PHMET_PALETTE_COLOR:
			if (Success != (err = write_colormap(tf, &colormap_at)))
				return err;
			++entrycount;
			/* fall through */
	   default:
			bpscount = 1;
			bps_at = tf->bits_per_sample[0];
			break;

		}

	/*
	 * now we are ready to write the ifd itself.  we remember where it starts
	 * so that we can go back and patch up the main file header later.
	 * the first thing in the ifd is the count of entries to follow, then
	 * the entries themselves (in ascending numerical sequence), then a
	 * longword zero, to say that there is not another ifd in the file.
	 *
	 * the tiff 5.0 doc says the ifd has to start on an even word boundry,
	 * so we also make sure that happens by writing a pad byte if needed.
	 */

	if (0 >= (tf->off_ifd_start = ftell(tf->file)))
		return pj_errno_errcode();

	if (tf->off_ifd_start & 0x01)
		{
		if (1 != fwrite(&a_longzero, 1, 1, tf->file))
			return pj_errno_errcode();
		++tf->off_ifd_start;
		}

	if (1 != fwrite(&entrycount, sizeof(entrycount), 1, tf->file))
		return Err_truncated;

	/* note that the following lines must stay in numerical tag order... */

	W(TAG_SUBFILE_TYPE, 	 TYPE_SHORT, 1, 		 1);
	W(TAG_IMAGE_WIDTH,		 TYPE_SHORT, 1, 		 tf->width);
	W(TAG_IMAGE_LENGTH, 	 TYPE_SHORT, 1, 		 tf->height);
	W(TAG_BITS_PER_SAMPLE,	 TYPE_SHORT, bpscount,	 bps_at);
	W(TAG_COMPRESSION,		 TYPE_SHORT, 1, 		 tf->compression);
	W(TAG_PHOTOMETRIC_INTERP,TYPE_SHORT, 1, 		 tf->photometric);
	W(TAG_STRIP_OFFSETS,	 TYPE_LONG,  stripcount, stripoffsets_at);
	W(TAG_SAMPLES_PER_PIXEL, TYPE_SHORT, 1, 		 tf->samples_per_pixel);
	W(TAG_ROWS_PER_STRIP,	 TYPE_SHORT, 1, 		 tf->rows_per_strip);
	W(TAG_STRIP_BYTE_COUNTS, TYPE_LONG,  stripcount, stripsizes_at);
	W(TAG_MIN_SAMPLE_VALUE,  TYPE_SHORT, 1, 		 tf->min_sample_value);
	W(TAG_MAX_SAMPLE_VALUE,  TYPE_SHORT, 1, 		 tf->max_sample_value);
	W(TAG_PLANAR_CONFIG,	 TYPE_SHORT, 1, 		 tf->planar_configuration);

	if (tf->photometric == PHMET_PALETTE_COLOR)
		{
		W(TAG_COLORMAP, TYPE_SHORT, 3*256, colormap_at);
		}

	if (1 != fwrite(&a_longzero, sizeof(a_longzero), 1, tf->file))
		return Err_truncated;

	return Success;
}
#undef W

Errcode write_filehdr(Tiff_file *tf)
/*****************************************************************************
 * seek to the beginning of the file, and write (or rewrite) the file header.
 *
 *	this routine is used twice; once right after opening the file, primarily
 *	to advance us to the point where the first image data will be written;
 *	then we do it again just before closing the file, to lay in the proper
 *	offset to the ifd that gets written at the end of the file.
 ****************************************************************************/
{
	Tifhdr	tifhdr;


	fflush(tf->file);

	if (Success != fseek(tf->file, 0, SEEK_SET))
		return pj_errno_errcode();

	tifhdr.machine	= INTEL;
	tifhdr.version	= OUTPUT_TIFF_VERSION;
	tifhdr.firstifd = tf->off_ifd_start;

	if (1 != fwrite(&tifhdr, sizeof(Tifhdr), 1, tf->file))
		return Err_truncated;

	fflush(tf->file);

	return Success;
}

Errcode write_strip(Tiff_file *tf, char *stripbuf,
								   long striplen,
								   int	stripnum)
/*****************************************************************************
 * record offset and size of current strip in strip_data array, write strip.
 ****************************************************************************/
{
	if (0 >= (tf->strip_data[stripnum].offset = ftell(tf->file)))
		return pj_errno_errcode();

	tf->strip_data[stripnum].size = striplen;

	if (1 != fwrite(stripbuf, striplen, 1, tf->file))
		return Err_truncated;

	return Success;

}
