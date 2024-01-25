/*****************************************************************************
 * TIFREAD.C  - Routines to read tif file header, tags, and strips.
 *				File header and tags are interpreted and the values are
 *				stored into our own data structures.  strips are just
 *				read and passed back to the caller as raw data.
 ****************************************************************************/

#include "tiff.h"

static union {
	long  dword;
	struct {
		unsigned short word1;
		unsigned short word2;
		} words;
	} convert;


long calc_maxdata(Tiff_file *tf)
/*****************************************************************************
 * calc the max number of bytes that could be in a strip after decompression.
 * note that there is no fudge factor built in, the return value s/b exact.
 ****************************************************************************/
{
	int 		rps;
	int 		nsamples;
	long		maxdata;

	rps = (tf->rows_per_strip < tf->height) ? tf->rows_per_strip : tf->height;
	nsamples = tf->width * rps;
	maxdata = (nsamples * tf->bits_per_sample[0] + 7) / 8;
	if (tf->samples_per_pixel == 3 && tf->planar_configuration == 1)
		{
		maxdata += (nsamples * tf->bits_per_sample[1] + 7) / 8;
		maxdata += (nsamples * tf->bits_per_sample[2] + 7) / 8;
		}
	return maxdata;
}

static Errcode read_color_map(Tiff_file *tf, long offset, int count)
/*****************************************************************************
 * load and convert the tiff color map data to our palette format.
 ****************************************************************************/
{
	int 	counter;
	USHORT	colorbuf[3][COLORS];
	USHORT	maxcolor = 0;
	USHORT	*scanptr;
	USHORT	scanval;
	int 	shftamt;
	Rgb3	*ctab;

	/*------------------------------------------------------------------------
	 * allocate and load the color table...
	 *----------------------------------------------------------------------*/
	if (NULL == (tf->color_table = malloc(3*COLORS)))
		return Err_no_memory;

	if (Success != fseek(tf->file, offset, SEEK_SET))
		return pj_errno_errcode();

	if (count != fread(colorbuf, sizeof(short), count, tf->file))
		return Err_truncated;

	/*------------------------------------------------------------------------
	 * scan the color table for values greater than 255.
	 * if we find such values, we assume the colors need to be scaled by
	 * right-shifting 8 bits (scale 0-65535 to 0-255 range).  if we don't
	 * find any values over 255, the map is already scaled to our range.
	 * (and finagle damn all those write so-called specification documents
	 * that leave such matters open to interpretation).
	 *----------------------------------------------------------------------*/

	scanptr = (USHORT *)colorbuf;
	for (counter = 0; counter < 3*COLORS; ++counter)
		{
		scanval = *scanptr++;
		if (scanval > maxcolor)
			maxcolor = scanval;
		}

	if (maxcolor > 255)
		shftamt = 8;
	else
		shftamt = 0;

	/*------------------------------------------------------------------------
	 * copy the loaded color map to our Rgb3 format, scaling as we go...
	 *----------------------------------------------------------------------*/

	ctab = (Rgb3 *)tf->color_table;
	for (counter = 0; counter < COLORS; ++counter, ++ctab)
		{
		ctab->r = colorbuf[0][counter] >> shftamt;
		ctab->g = colorbuf[1][counter] >> shftamt;
		ctab->b = colorbuf[2][counter] >> shftamt;
		}

	return Success;
}

static Errcode read_strip_offsets(Tiff_file *tf, long offset, int type, int strip_count)
/*****************************************************************************
 * allocate the strip_data array and load strip offsets into it.
 *	note that we allocate one extra Strip_data slot in the array, in case we
 *	have to calculate strip bytecounts from the offsets later.
 ****************************************************************************/
{
	int 		counter;
	Strip_data	*curstrip;
	Onum		readbuf;

	tf->strips_per_image = strip_count;

	if (NULL != tf->strip_data) 	/* somehow got strip offset data twice */
		return Err_format;

	if (NULL == (tf->strip_data = malloc((strip_count+1) * sizeof(Strip_data))))
		return Err_no_memory;

	curstrip = tf->strip_data;

	if (strip_count == 1 && type == TYPE_LONG)		/* 1 longword is easy */
		{
		curstrip->offset = offset;
		}
	else if (strip_count == 2 && type == TYPE_SHORT)/* 2 shorts is tricky */
		{
		convert.dword = offset;
		curstrip->offset = convert.words.word1;
		++curstrip;
		curstrip->offset = convert.words.word2;
		}
	else											/* else we have to read */
		{
		if (Success != fseek(tf->file, offset, SEEK_SET))
			return pj_errno_errcode();

		counter = strip_count;
		while (counter--)
			{
			if (type == TYPE_SHORT)
				{
				if (1 != fread(&readbuf, sizeof(short), 1, tf->file))
					return Err_truncated;
				SWAPW(&readbuf.word);
				curstrip->offset = readbuf.uword;
				}
			else
				{
				if (1 != fread(&curstrip->offset, sizeof(long), 1, tf->file))
					return Err_truncated;
				SWAPD(&curstrip->offset);
				}
			++curstrip;
			}
		}

	return Success;

}

static Errcode read_strip_bytecounts(Tiff_file *tf, long offset, int type, int strip_count)
/*****************************************************************************
 * load strip bytecounts into strip_data array, also remember biggest value.
 ****************************************************************************/
{
	int 		counter;
	long		highcount = 0;
	Onum		readbuf;
	Strip_data	*curstrip = tf->strip_data;

	if (NULL == curstrip)	/* we need to have offset data already or we die */
		return Err_format;

	if (strip_count != tf->strips_per_image)	/* must have same # of byte */
		return Err_format;						/* counts as we had offsets */

	if (strip_count == 1 && type == TYPE_LONG)		 /* 1 long is easy */
		{
		curstrip->size = highcount = offset;
		}
	else if (strip_count == 2 && type == TYPE_SHORT) /* 2 shorts is tricky */
		{
		convert.dword = offset;
		curstrip->offset = convert.words.word1;
		++curstrip;
		curstrip->offset = convert.words.word2;
		}
	else
		{
		if (Success != fseek(tf->file, offset, SEEK_SET))
			return pj_errno_errcode();

		counter = strip_count;
		while (counter--)
			{
			if (type == TYPE_SHORT)
				{
				if (1 != fread(&readbuf, sizeof(short), 1, tf->file))
					return Err_truncated;
				SWAPW(&readbuf);
				curstrip->size = readbuf.uword;
				}
			else
				{
				if (1 != fread(&curstrip->size, sizeof(long), 1, tf->file))
					return Err_truncated;
				SWAPD(&curstrip->size);
				}
			if (curstrip->size > highcount)
				highcount = curstrip->size;
			++curstrip;
			}
		}

	tf->longest_strip = highcount;
	return Success;

}

static Errcode calc_strip_bytecounts(Tiff_file *tf)
/*****************************************************************************
 * if strip bytecounts were not found in the file, try to calculate them.
 *
 *	this routine attempts to fill in strip bytecounts information when it is
 *	missing from the file.	the size of each strip is calculated based on the
 *	offset to the next strip.  the last strip size is handled as follows:
 *	> if the last strip offset is less than the current ifd offset, the strip
 *	  precedes the ifd in the file. the last size is the offset of the start
 *	  of the current ifd minus the offset of the strip.
 *	> if the last strip offset is greater than the current ifd offset, the
 *	  strip follows the ifd, and we assume its size to be from the last strip
 *	  offset to the start of the next ifd, or to the end of the file.
 *
 *	since this concept makes some pretty liberal assumptions about the data
 *	layout in the file, we keep an eye out for negative sizes, and return
 *	an error status if that happens.  (never should.  if a file was written
 *	without bytecounts, it almost *has* to be in sequential strip order if
 *	there is to be any hope of reading it.)  also, we often end up thinking
 *	the last strip is a little bigger than it really is, because our calcs
 *	will probably end up counting the strip offsets table and/or other non-
 *	image data as part of the last strip.  this doesn't really hurt anything,
 *	since the actual decompression/output routines are driven by the width &
 *	height values, not by the size of the input strips.
 *
 *	this has been tweaked to work for rgb in planar config 1. (01/20/91).
 *
 *	Note to self:  this whole concept will die horribly if there are multiple
 *	images in the file.  the last strip could end up looking to be the size
 *	of all the remaining images in the file, ie, megabytes in worst-case.
 *	 (Er, now we fudge around this _a bit_ by limiting the size to maxdata,
 *	  the calculated maximum size of a strip, assuming no compression. we
 *	  also now look for another ifd following the last strip.  if there is
 *	  one, the distance between it and the last strip becomes the last strip
 *	  size.  if all the ifds are at the start of the file, we have to rely
 *	  on the maxdata hack to save us.  all in all, this routine is now
 *	  reasonably robust.)
 ****************************************************************************/
{
	long		maxdata;
	long		size;
	int 		counter;
	long		dmyoffset;
	long		lastoffset;
	int 		strips	  = tf->strips_per_image;
	Strip_data	*sdata	  = tf->strip_data;
	long		highcount = 0;

	if (NULL == sdata)	/* we need to have offset data already or we die */
		return Err_format;

	/*
	 * take a guess at what the maximum size of the strip should be.
	 * we figure the number of samples (width*rows_per_strip) times the number
	 * of bits per sample, divided by 8 to make it a byte count, then we add
	 * 25% to allow for worst-case ocurrance of a compression scheme.  all
	 * of this is just to prevent assigning a vastly huge buffer to the last
	 * strip when the file contains multiple images.
	 */

	maxdata = calc_maxdata(tf);
	maxdata += maxdata >> 2;

	/*
	 * first we fudge in an offset for the dummy end strip...
	 */

	lastoffset = sdata[strips-1].offset;
	if (lastoffset < tf->off_ifd_start)
		{									/* strips before ifd in file... */
		dmyoffset = tf->off_ifd_start;
		}
	else
		{									/* strips after ifd in file...	*/
		if (tf->off_ifd_next > lastoffset)
			dmyoffset = tf->off_ifd_next;	/* if another ifd follows last	*/
		else								/* strip, use its offset, else	*/
			{								/* we have to assume EOF offset */
			if (Success != fseek(tf->file, 0, SEEK_END))
				return pj_errno_errcode();
			dmyoffset = ftell(tf->file);
			if (dmyoffset > lastoffset+maxdata)
				dmyoffset = lastoffset+maxdata;
			}
		}

	sdata[strips].offset = dmyoffset;

	/*
	 * now do the loop to calc the bytecounts...
	 */

	 for (counter = 0; counter < strips; ++counter)
		{
		size = sdata[counter+1].offset - sdata[counter].offset;
		if (size < 0)
			return Err_format;
		if (highcount < size)
			highcount = size;
		sdata[counter].size = size;
		}

	 tf->longest_strip = highcount;
	 return Success;
}

static Errcode read_next_dirent(Tiff_file *tf, Dirent *entry)
/*****************************************************************************
 * load a directory entry (tag, count, type, offset) from the current ifd.
 ****************************************************************************/
{
	if (Success != fseek(tf->file, tf->off_ifd_cur, SEEK_SET))
		return pj_errno_errcode();

	if (1 != fread(entry, sizeof(*entry), 1, tf->file))
		return pj_errno_errcode();

	if (tf->swap_bytes)
		{
		swapw(&entry->tag);
		swapw(&entry->type);
		swapd(&entry->count);
		if (entry->type == TYPE_SHORT && entry->count <= 2)
			{
			swapw(&entry->value.word);
			if (entry->count == 2)
				swapw(((char *)(&entry->value.word))+2);
			}
		else
			swapd(&entry->value.dword);
		}

	return Success;
}

Errcode read_tiftags(Tiff_file *tf)
/*****************************************************************************
 * load all the tags in an ifd, save the data we care about along the way.
 *	after reading everything in the ifd, this routine attempts to fudge in
 *	any data that was missing (eg, strip offsets, strip bytecounts).  it also
 *	verifies that the required fields were present (our idea of required
 *	fields, not tiff's idea.  we require image width and height; we can fake
 *	just about anything else.)
 ****************************************************************************/
{
	Errcode err;
	short	entry_count;
	Dirent	entry;
	int 	bps_count;

	/*
	 * plug in default values for things we care about...
	 */

	tf->width = 0;
	tf->height = 0;
	tf->longest_strip = 0;
	tf->strips_per_image = 0;
	tf->planar_configuration = 1;
	tf->rows_per_strip = 0x7fffffff;
	tf->samples_per_pixel = 1;
	tf->bits_per_sample[0] = 1;
	tf->bits_per_sample[1] = 0;
	tf->bits_per_sample[2] = 0;
	tf->compression = CMPRS_NONE;
	tf->photometric = PHMET_GREY_0ISBLACK;
	tf->min_sample_value = 0;
	tf->max_sample_value = 0;	/* default will be calc'd later if needed */

	/*
	 * the saved offset of the next ifd becomes the offset to the current
	 * ifd.  we seek to that location (just in case), and load the count
	 * of entries in the ifd, which will be used as a loop counter.
	 */

	tf->off_ifd_start = tf->off_ifd_cur = tf->off_ifd_next;
	if (Success != fseek(tf->file, tf->off_ifd_cur, SEEK_SET))
		return pj_errno_errcode();
	if (1 != fread(&entry_count, sizeof(entry_count), 1, tf->file))
		return pj_errno_errcode();
	SWAPW(&entry_count);
	tf->off_ifd_cur += sizeof(entry_count);

	/*
	 * loop through all the entries (tags) in the ifd, storing off the data
	 * we care about into our own structure as we go.
	 */

	while (entry_count--)
		{
		if (Success != (err = read_next_dirent(tf, &entry)))
			return err;

		switch (entry.tag)
			{

			case TAG_IMAGE_WIDTH:

				tf->width = entry.value.word;
				break;

			case TAG_IMAGE_LENGTH:

				tf->height = entry.value.word;
				break;

			case TAG_BITS_PER_SAMPLE:

				bps_count = entry.count;
				switch (entry.count)
					{
					case 1:
						tf->bits_per_sample[0] = entry.value.word;
						if (tf->bits_per_sample[0] > 8)
							return Err_wrong_type;
						break;
					case 3:
						if (Success != fseek(tf->file, entry.value.offset, SEEK_SET))
							return pj_errno_errcode();
						if (3 != fread(tf->bits_per_sample, sizeof(short), 3, tf->file))
							return Err_truncated;
						if (tf->swap_bytes)
							{
							swapw(&tf->bits_per_sample[0]);
							swapw(&tf->bits_per_sample[1]);
							swapw(&tf->bits_per_sample[2]);
							}
						if (tf->bits_per_sample[0] != 8 ||
							tf->bits_per_sample[1] != 8 ||
							tf->bits_per_sample[2] != 8 )
							return Err_wrong_type;
						break;
					default:
						return Err_wrong_type;
					}
				break;

			case TAG_COMPRESSION:

				tf->compression = entry.value.uword;
				break;

			case TAG_PHOTOMETRIC_INTERP:

				tf->photometric = entry.value.word;
				break;

			case TAG_STRIP_OFFSETS:
			case TAG_SHORT_STRIP_OFFSETS:	/* Aldus private tag 32768 */

				if (Success != (err = read_strip_offsets(tf, entry.value.offset,
									entry.type, entry.count)))
					return err;
				break;

			case TAG_SAMPLES_PER_PIXEL:

				tf->samples_per_pixel = entry.value.word;
				switch (tf->samples_per_pixel)
					{
					case 1:
						if (tf->photometric == PHMET_RGB)
							return Err_format;
						break;
					case 3:
						tf->photometric = PHMET_RGB;	/* force RGB interp */
						break;
					default:
						return Err_wrong_type;
					}
				break;

			case TAG_ROWS_PER_STRIP:

				tf->rows_per_strip =
					(entry.type == TYPE_SHORT) ? entry.value.word : entry.value.dword;
				break;

			case TAG_STRIP_BYTE_COUNTS:

				if (Success != (err = read_strip_bytecounts(tf, entry.value.offset,
									entry.type, entry.count)))
					return err;
				break;

			case TAG_MIN_SAMPLE_VALUE:
			case TAG_MAX_SAMPLE_VALUE:
				/* we ignore these fields and calc values below.
				 * we use the min/max values to calculate greyscale
				 * pallete entries, but on the advice of of the TIFF 5.0
				 * doc ('these values should not affect the visual display
				 * of the data, they are for statistical purposes only'),
				 * we use 2**bits_per_sample instead of max_sample_value.
				 */
				 break;

			case TAG_PLANAR_CONFIG:

				tf->planar_configuration = entry.value.word;
				break;

			case TAG_COLORMAP:

				if (Success != (err = read_color_map(tf, entry.value.offset, entry.count)))
					return err;
				break;

			default:
				break;
			}
		tf->off_ifd_cur += sizeof(entry);
		}

	/*
	 * read the offset to the next ifd; we may need it for multi-image file
	 */

	if (Success != fseek(tf->file, tf->off_ifd_cur, SEEK_SET))
		return pj_errno_errcode();
	if (1 != fread(&tf->off_ifd_next, sizeof(tf->off_ifd_next), 1, tf->file))
		return Err_truncated;
	SWAPD(&tf->off_ifd_next);

	/*
	 * if we have RGB data but we only got one bits_per_sample value,
	 * propogate it to the other two entries in the array.
	 * calculate the total pixel depth.
	 */

	if (tf->photometric == PHMET_RGB && bps_count != 3)
		{
		tf->bits_per_sample[1] = tf->bits_per_sample[0];
		tf->bits_per_sample[2] = tf->bits_per_sample[0];
		}

	tf->pixel_depth = tf->bits_per_sample[0] +
					  tf->bits_per_sample[1] +
					  tf->bits_per_sample[2];

	/*
	 * calculate max_sample_value as (2**bits_per_sample)-1...
	 */

	if (tf->max_sample_value == 0)
		tf->max_sample_value = (0x01 << tf->bits_per_sample[0]) - 1;

	/*
	 * hmmm.  it seems some tif files don't have strip bytecount entries.
	 * we'll see symptoms of that here if the longest strip size is zero
	 * after reading all the tags.	when this occurs, we call a routine
	 * to calculate the byte counts.  the routine does some sanity checking
	 * and will return an error if the counts look totally unreasonable.
	 */

	if (tf->longest_strip == 0)
		if (Success != (err = calc_strip_bytecounts(tf)))
			return err;

	/*
	 * all entries in this ifd have been dealt with, make sure we have
	 * the minimum required data to run the rest of the program...
	 *	 checking strips_per_image ensures that we got strip offsets.
	 *	 checking longest_strip ensures we got (or calc'd) strip bytecounts.
	 */

	 if (tf->width == 0 ||
		 tf->height == 0 ||
		 tf->strips_per_image == 0 ||
		 tf->longest_strip == 0)
		return Err_format;


	return Success;
}

Errcode read_filehdr(Tiff_file *tf)
/*****************************************************************************
 * read tiff file header, validate magic numbers.
 ****************************************************************************/
{
	Tifhdr	tifhdr;

	if (Success != fseek(tf->file, 0, SEEK_SET))
		return pj_errno_errcode();

	if (1 != fread(&tifhdr, sizeof(tifhdr), 1, tf->file))
		return Err_truncated;

	if (tifhdr.machine == MOTOROLA)
		tf->swap_bytes = TRUE;
	else if (tifhdr.machine == INTEL)
		tf->swap_bytes = FALSE;
	else
		return Err_bad_magic;

	SWAPW(&tifhdr.version);
	SWAPD(&tifhdr.firstifd);

	if (tifhdr.version < 42 || tifhdr.version > 59) /* we do tiff versions */
		return Err_bad_magic;						/* 4.x and 5.x only.   */

	if (tifhdr.firstifd < 8)
		return Err_bad_magic;

	tf->off_ifd_next = tifhdr.firstifd;

	return Success;
}

Errcode read_strip(Tiff_file *tf, char *stripbuf, Strip_data *curstrip)
/*****************************************************************************
 * load a complete tiff strip into the strip buffer.
 ****************************************************************************/
{
	if (Success != fseek(tf->file, curstrip->offset, SEEK_SET))
		return pj_errno_errcode();

	if (1 != fread(stripbuf, curstrip->size, 1, tf->file))
		return Err_truncated;

	return Success;
}
