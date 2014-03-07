/***************************************************************************
 * bmp.c - The Microsoft Windows 3.0 compatible .BMP (Device independent
 * bitmap) picture driver. 
 ***************************************************************************/

#include <assert.h>
#include <string.h>
#define REXLIB_INTERNALS
#include "animinfo.h"
#include "errcodes.h"
#include "memory.h"
#include "picdrive.h"
#include "util.h"
#include "xfile.h"

/*-------------------------BMP STRUCTURE SECTION----------------------------*/

/***************************************************************************
 * Description of MicroSoft Windows .BMP file format: 
 *
 * Here is an overview of the file structure:
 *
 *  BITMAPFILEHEADER   ........................14 bytes
 *		BITMAPINFOHEADER   ....................40 bytes
 *			B G R reserved (color map)..........4 bytes
 *				...
 *			B G R reserved (color map)..........4 bytes
 *		PIXEL-DATA
 *
 * The BITMAPFILEHEADER includes the file-type identifier (which is 
 * ascii BM),  size of the file and an offset to the 
 * actual bitmap in the file.
 *
 * The BITMAPINFOHEADER contains your usual width/height/color depth.
 * Also some misc other bits which for the most part we'll ignore.
 * Part of the BITMAPINFOHEADER says how long that structure is.
 * In older .BMP's it will be 12 bytes while in newer ones it'll be
 * 40 bytes.  
 *
 * Next is the color map.  Normally each entry is 4 bytes with the 4th byte
 * ignored (it's always zero in the files I've seen).  Why exactly
 * RGB data is stored in BGR order must have something to do with
 * Intel byte-ordering or something.  If the BITMAPINFOHEADER is old
 * style though each entry is only 3 bytes, still in BGR order.
 *
 * After the color map is the PIXEL DATA which seems to
 * be stored in packed pixel format (ie 2 pixels to the byte in 16 
 * mode,  1 per byte in 256 color mode, 8 per byte in 2 color.
 * Though the info-header contains a field for number of planes
 * the SDK documentation assures us that this will always be one.
 * Further it restricts the number of bits-per-pixel to 1, 4, 8, or 24.
 *
 * The PIXEL DATA is stored a line at a time going from bottom to
 * top (upside-down from usual for us.)  Each line ends on a 4-byte
 * boundary with zero pad bytes inserted as necessary.
 ***************************************************************************/


typedef struct GCC_PACKED
/* The first thing in a bitmap file... */
{
	uint16_t bfType;        /* Always BM_MAGIC. */
	uint32_t bfSize;        /* File size.  Includes this header. */
	uint16_t bfReserved1;   /* Always 0 at the moment. */
	uint16_t bfReserved2;   /* Always 0 at the moment. */
	uint32_t bfOffBits;     /* Where the pixels start. (Right after color map.) */
} BITMAPFILEHEADER;	
STATIC_ASSERT(bmp, sizeof(BITMAPFILEHEADER) == 14);

	/* Value in bfType field.  It's 'BM' ascii.... */
#define BM_MAGIC 0x4D42


typedef struct
/* This guy tells us the dimensions of the file and other interesting
 * goodies. */
{
	uint32_t biSize;        /* Info header size.  Always 0x28 in Windows 3.0. */
	 int32_t biWidth;       /* Width of bitmap in pixels. */
	 int32_t biHeight;      /* Height of bitmap in pixels. */
	uint16_t biPlanes;      /* Number of planes. Always 1, at least in 1991. */
	uint16_t biBitCount;    /* Bits per plane.  1, 4, 8, or 24 in 1991. */
	uint32_t biCompression; /* Compression type.  See #defines below for values. */
	uint32_t biSizeImage;   /* Size in bytes of the pixel parts. */
	 int32_t biXPelsPerMeter; /* Pixels per meter of image's target device. */
	 int32_t biYPelsPerMeter;
	uint32_t biClrUsed;     /* # of colors used in image or 0 if all are. */
	uint32_t biClrImportant;/* Number of "Important" colors, or 0 if all are. */
} BITMAPINFOHEADER;
STATIC_ASSERT(bmp, sizeof(BITMAPINFOHEADER) == 40);

typedef struct
/* Earlier version of info-header. */
{
	uint32_t biSize;        /* Info header size.  Always 0x0C. */
	uint16_t biWidth;       /* Width of bitmap in pixels. */
	uint16_t biHeight;      /* Height of bitmap in pixels. */
	uint16_t biPlanes;      /* Number of planes.  Always 1, at least in 1991. */
	uint16_t biBitCount;    /* Bits per plane.  1, 4, 8, or 24 in 1991. */
} OLDBITMAPINFOHEADER;
STATIC_ASSERT(bmp, sizeof(OLDBITMAPINFOHEADER) == 12);

/* Is it new or old type of info? */
enum Info_type 
{
	INFO_TYPE_NEW,
	INFO_TYPE_OLD,
};


/* constants for the biCompression field */
#define BI_RGB      0L
#define BI_RLE8     1L
#define BI_RLE4     2L

typedef struct
/* Color map in Intel type ordering... */
{
	unsigned char b, g, r, a;
} BGRA;

typedef struct
/* Color map in Intel type ordering... */
{
	unsigned char b, g, r;
} BGR;


/*-------------------------UTILITY SECTION---------------------------------*/

static Errcode read_buf(XFILE *f, void *buf, int size)
/* Read a buffer of a certain size */
{
	if (xfread(buf, size, 1, f) != 1)
		return(Err_truncated);
	else
		return(Success);
}

static Errcode write_buf(XFILE *f, void *buf, int size)
/* Write a buffer of a certain size */
{
	if (xfwrite(buf, size, 1, f) != 1)
		return(Err_write);
	else
		return(Success);
}


#define read_var(f,s) read_buf(f,&s,sizeof(s))
/* Read a variable from file. */
#define write_var(f,s) write_buf(f,&s,sizeof(s))
/* Write a variable to file. */

/*--------------------------STRING SECTION---------------------------------*/

static Boolean suffix_in(char *string, char *suff)
/* Return TRUE if string ends with suff. */
{
	string += strlen(string) - strlen(suff);
	return( txtcmp(string, suff) == 0);
}

/*---------------SPLICE & DICE BITS TO BYTES SECTION-----------------------*/

	/* Find next number greater than or equal to input but even. */
#define NEXT_EVEN(x) (((x)+1)&0xfffffffe)
	/* Find next number greater than or equal to input but divisible by 4. */
#define NEXT_QUAD(x) (((x)+3)&0xfffffffc)

static int bytes_per_row(int32_t width, uint16_t bits_per_pixel)
/* Figure out how many bytes in a line of the picture.  Round to
 * nearest byte boundary, and then to nearest long-word boundary.
 * That is the result will always be a multiple of 4. */
{
	int bpr;	/* result */

	switch (bits_per_pixel)
	{
	/* We handle a few more cases than required her in anticipation of
	 * Windows 4.0 */
		case 1:
			bpr = (width+7)>>3;
			break;
		case 2:
			bpr = (width+3)>>2;
			break;
		case 4:
			bpr = (width+1)>>1;
			break;
		case 8:
			bpr = width;
			break;
		case 16:
			bpr = width << 1;
			break;
		case 24:
			bpr = width*3;
			break;
		case 32:
			bpr = width<<2;
			break;
		default:
			assert(0);
			return 0;
	}
	return (bpr+3)&0xfffc;		/* Round up to longword boundary. */
}

static void bits_to_bytes(char *in, char *out, int in_size,
	char zero_color, char one_color)
/* Convert 1 bits in "in" to one_color bytes in out,  and likewise
 * zero bits into zero_color bytes.  Out better be in_size*8 big. */
{
	int mask;
	int source;
	while (--in_size >= 0)
	{
		source = *in++;
		for (mask = (1<<7); mask ; mask >>= 1)
		{
			if (source & mask)
				*out++ = one_color;
			else
				*out++ = zero_color;
		}
	}
}

static void nibbles_to_bytes(char *in, char *out, int in_size)
/* Convert nibbles (4 bit quantities) to bytes.  Out better be 
 * in_size*2 big. */
{
	int source;

	while (--in_size >= 0)
	{
		source = *in++;
		*out++ = ((source>>4) & 0xf);
		*out++ = source & 0xf;
	}
}



/*-------------------------BMP READ SECTION--------------------------------*/
static enum Info_type upgrade_old_info(BITMAPINFOHEADER *h)
/* Upgrade old style info header to new style. */
{
	OLDBITMAPINFOHEADER *oh;
	BITMAPINFOHEADER  fixed;

	if (h->biSize == 0x0C)		/* Old headers were 12 bytes */
	{
		oh = (OLDBITMAPINFOHEADER *)h;
		clear_struct(&fixed);
		fixed.biSize = oh->biSize;
		fixed.biWidth = oh->biWidth;
		fixed.biHeight = oh->biHeight;
		fixed.biPlanes = oh->biPlanes;
		fixed.biBitCount = oh->biBitCount;
		fixed.biCompression = BI_RGB;
		*h = fixed;
		return INFO_TYPE_OLD;
	}
	else
	{
		return INFO_TYPE_NEW;
	}
}

static Errcode read_head(XFILE *f
, 	BITMAPFILEHEADER *head, BITMAPINFOHEADER *info)
/* Read in the file header and info.  Verify the type fields and
 * generally make sure the values look reasonable. */
{
	Errcode err;
	enum Info_type itype;
	uint16_t bits;

	if ((err = read_var(f, *head)) < Success)
		goto ERROR;
	/* Check type field in header. */
	if (head->bfType != BM_MAGIC)
	{
		err = Err_bad_magic;
		goto ERROR;
	}
	/* Do a quick consistency check on other fields.  Make sure
	 * file size and data offset are both at least as big as the
	 * header parts,  and that the data offset is less than the file
	 * size. */
	if (head->bfSize < sizeof(*head) + sizeof(*info)
	||		head->bfOffBits < sizeof(*head) + sizeof(*info)
	||		head->bfSize < head->bfOffBits)
	{
		err = Err_format;
		goto ERROR;
	}
	if ((err = read_var(f, *info)) < Success)
		goto ERROR;
	itype = upgrade_old_info(info);
	/* Make sure the pixel depth etc. is one we can handle.  Return
	 * Err_version here because I suspect Microsoft will probably
	 * relax some of their restrictions on pixel depth in future
	 * versions of Windows. */
	if (info->biPlanes != 1)
	{
		err = Err_version;
		goto ERROR;
	}
	bits = info->biBitCount;
	switch (bits)
	{
		case 1:
		case 4:
		case 8:
		case 24:
			break;
		default:
		{
			err = Err_version;
			goto ERROR;
		}
	}
	/* Verify the compression field is one we know about,  and that
	 * it corresponds with the bit-per-pixel field. */
	switch (info->biCompression)
	{
		case BI_RGB:
			break;
		case BI_RLE8:
			if (bits != 8)
			{
				err = Err_format;
				goto ERROR;
			}
			break;
		case BI_RLE4:
			if (bits != 4)
			{
				err = Err_format;
				goto ERROR;
			}
			break;
		default:
			err = Err_version;
			goto ERROR;
			break;
	}
	err = itype;
ERROR:
	return err;
}

static Errcode read_new_colors(XFILE *f, Cmap *c, unsigned int count)
/* Read a bunch of colors in BGRA format from file into color map. */
{
	Rgb3 *out = c->ctab;
	BGRA in;
	Errcode err = Err_bad_input;
	assert(count > 0);

	while (count-- > 0)
	{
		if ((err = read_var(f, in)) < Success)
			break;
		out->r = in.r;
		out->g = in.g;
		out->b = in.b;
		++out;
	}
	return err;
}

static Errcode read_old_colors(XFILE *f, Cmap *c, unsigned int count)
/* Read a bunch of colors in BGR format from file into color map. */
{
	Rgb3 *out = c->ctab;
	BGR in;
	Errcode err = Err_bad_input;
	assert(count > 0);

	while (count-- > 0)
	{
		if ((err = read_var(f, in)) < Success)
			break;
		out->r = in.r;
		out->g = in.g;
		out->b = in.b;
		++out;
	}
	return err;
}

static Errcode read_uncompressed(XFILE *f, BITMAPINFOHEADER *info, Rcel *screen)
/* Read in uncompressed pixels a line at a time from upside-down file to
 * screen.  Adjust for long-word line padding. Unpack bit-a-pixel or
 * nibble-a-pixel representation to byte-a-pixel. */
{
	int32_t width = info->biWidth;
	uint16_t bits = info->biBitCount;
	int bpr = bytes_per_row(width,bits);
	int32_t i = info->biHeight;
	void *data_buf = NULL;
	void *pixel_buf = NULL;
	Errcode err = Success;

	if ((err = ealloc(&data_buf, bpr)) < Success)
		goto ERROR;
	/* Allocate buffer to hold byte-a-pixel version of data. */
	switch (bits)
	{
		case 1:
			err = ealloc(&pixel_buf, (width+7)&0xfffffff8);
			break;
		case 4:
			err = ealloc(&pixel_buf, (width+1)&0xfffffffe);
			break;
		case 8:
			pixel_buf = data_buf;
			break;
		default:
			err = Err_unimpl;
			break;
	}
	if (err < Success)
		goto ERROR;
	while (--i >= 0)
	{
		if ((err = read_buf(f, data_buf, bpr)) < Success)
			goto ERROR;
		/* Convert data to byte-a-pixel */
		switch (bits)
		{
			case 1:
				bits_to_bytes(data_buf, pixel_buf, (width+7)>>3, 0, 1);
				break;
			case 4:
				nibbles_to_bytes(data_buf, pixel_buf, (width+1)>>1);
				break;
		}
		pj_put_hseg(screen, pixel_buf, 0, i, width);
	}
ERROR:
	if (pixel_buf != data_buf)
		pj_freez(&pixel_buf);
	pj_freez(&data_buf);
	return err;
}

static Errcode read_rle(XFILE *f, BITMAPINFOHEADER *info, Rcel *screen)
/* Read in a BI_RLE4 and BI_RLE8 compressed image.  Like all BMP's this will be
 * stored from bottom to top.  We know this one will by byte-a-pixel.
 * The compression scheme is rather complex and actually not that
 * good.  Here's some relevant quotes from the Microsoft SDK manual
 * Reference volume 2 p 7-14:
 *			This format may be compresses in either of two modes -
 *		encoded or absolute.  Both modes can occur anywhere throughout
 *		a single bitmap.  
 *			Encoded mode consists of two bytes:  the first byte specifies
 *		the number of consecutive pixels to be drawn using the color index
 *		in the second byte.  In addition the first byte of the pair can
 *		be set to zero to indicate an escape that denotes an end of line,
 *		end of bitmap,  or a delta.  The interpretation of the escape
 *		depends on the value of the second byte of the pair.  The following
 *		list shows the meaning of the second byte:
 *		Second byte of escape:	Meaning:
 *		---------------------   -------------------------------------------
 *		0						End of line.
 *		1						End of bitmap.
 *		2						Delta.  The two bytes following the escape 
 *								contain unsigned values indicating the 
 *								horizontal and vertical offset of the next 
 *								pixel from the current position.
 *      --------------------------------------------------------------------
 *			Absolute mode is signalled by the first byte set to zero and the
 *		second byte set to a value between 03h and FFh.  In absolute mode,
 *		the second byte represents the number of bytes which follow, each of
 *		which contain the color index of a single pixel.  When the second byte
 *		is set to 2 or less, the escape has the same meaning as in encoded 
 *		mode.  In absolute mode each run must be alligned on a word boundary.
 */
{
	struct  
	{ 
		unsigned char first,second; 
	} pair; 	/* Read file pair at a time. */
	Boolean is_nibbled = (info->biCompression == BI_RLE4);
	int32_t height = info->biHeight;
	int32_t width = info->biWidth;
	int32_t even_width = NEXT_EVEN(width); /* Width rounded up to next even #. */
	int curx, cury;						/* Current screen position. */
	void *data_buf = NULL;				/* Read literal data into here. */
	void *pixel_buf = NULL;				/* Convert to pixel data here. */
	Errcode err = Success;
	unsigned char c;
	unsigned char *pt;
	int i;

	/* Allocate buffers for processing literal data.  If in byte-a-pixel
	 * mode can take a short-cut and just use the disk buffer as the
	 * pixel buffer too since no conversion is necessary. */
	if ((err = ealloc(&data_buf, even_width)) < Success)
		goto ERROR;
	if (is_nibbled)
	{
		if ((err = ealloc(&pixel_buf, even_width)) < Success)
			goto ERROR;
	}
	else
	{
		pixel_buf = data_buf;
	}
	cury = height-1;		/* File reads from bottom to top. */
	curx = 0;				/* (But left to right as usual.)  */
	for (;;)	/* Loop until get an end of file escape sequence. */
	{
		if ((err = read_var(f, pair)) < Success)
			goto ERROR;
		if (pair.first != 0)	/* it's a run! */
		{
			if (is_nibbled)	/* Oh no - it's a nibble run. */
			{
				c = (pair.second>>4);
				pair.second &= 0xf;
				if (c == pair.second)	/* Hi & lo nibbles same. */
				{
					pj_set_hline(screen, c, curx, cury, pair.first);
				}
				else					/* Hi & lo nibbles different */
				{
					i = ((pair.first+1)>>1); /* # of double pixels to produce */
					pt = pixel_buf;
					while (--i >= 0)
					{
						*pt++ = c;
						*pt++ = pair.second;
					}
					pj_put_hseg(screen, pixel_buf, curx, cury, pair.first);
				}
			}
			else	/* Simple easy byte-a-pixel run */
			{
				pj_set_hline(screen, pair.second, curx, cury, pair.first);
			}
			curx += pair.first;
		}
		else	/* It's an escape or a bunch of literal data. */
		{
			switch (pair.second)
			{
				case 0:		/* End of line. */
					curx = 0;
					--cury;
					break;
				case 1:		/* End of file. */
					goto OUT;
				case 2:		/* "Delta" */
					if ((err = read_var(f, pair)) < Success)
						goto ERROR;
					curx += pair.first;
					cury -= pair.second;
					break;
				default:	/* Copy some bytes (and skip word pad). */
					if (pair.second + curx > width)
					/* Sanity check on data. */
					{
						err = Err_format;
						goto ERROR;
					}
					if (is_nibbled) /* Nibble-a-pixel literal run */
					{
						if ((err = read_buf(f, data_buf
						,	NEXT_QUAD(pair.second)>>1)) < Success)
							goto ERROR;
						nibbles_to_bytes(data_buf, pixel_buf
						,	NEXT_EVEN(pair.second)>>1);
						pj_put_hseg(screen, pixel_buf, curx, cury, pair.second);
					}
					else	/* Byte a pixel literal run. */
					{
						if ((err = read_buf(f, data_buf
						,	NEXT_EVEN(pair.second))) < Success)
							goto ERROR;
						pj_put_hseg(screen, data_buf, curx, cury, pair.second);
					}
					curx += pair.second;
					break;
			}
		}
	}
OUT:
ERROR:
	if (pixel_buf != data_buf)
		pj_freez(&pixel_buf);
	pj_freez(&data_buf);
	return err;
}

static Errcode read_after_header(XFILE *f
,	BITMAPFILEHEADER *head, BITMAPINFOHEADER *info, enum Info_type itype
,	Rcel *screen)
/* Read everything after the file header and info - colors and pixels. 
 * Do some seeks to first the color position and then the pixel
 * position instead of reading linearly just in case later on
 * they change some of the sizes of things and the offsets become
 * necessary.  Decides whether it's a new-style header or old style,
 * and calls appropriate color-map reader.  Decides how pixels are
 * compressed and calls appropriate pixel reader. */
{
	long err;	/* This is long since could be returned from fseek. */
	uint16_t bits; /* Bits per pixel. */

	/* Read colors if not in true-color mode. */
	bits = info->biBitCount;
	if (bits < 16)		/* I figure 16 bits a pixel might be true-color
						 * threshold even in Windows 4.0.  In 3.0 you
						 * jump from 8 bits a pixel to 24, so this is safe. */
	{
		/* Seek to color start. */
		if ((err = xfseek(f
		,	sizeof(*head) + info->biSize, XSEEK_SET)) < Success)
			return err;
		if (itype == INFO_TYPE_NEW)
		{
			if ((err = read_new_colors(f
			,	screen->cmap, (1<<info->biBitCount))) < Success)
				return err;
		}
		else
		{
			if ((err = read_old_colors(f
			,	screen->cmap, (1<<info->biBitCount))) < Success)
				return err;
		}
	}
	pj_cmap_load(screen,screen->cmap); /* update hardware cmap if needed */
	/* Seek to pixel start. */
	if ((err = xfseek(f, head->bfOffBits, XSEEK_SET)) < Success)
		return err;
	/* Read pixels. */
	switch (info->biCompression)
	{
		case BI_RGB:
			return read_uncompressed(f, info, screen);
		case BI_RLE8:
		case BI_RLE4:
			return read_rle(f, info, screen);
		default:
			return Err_version;

	}
}

/*-------------------------BMP READ RGB SECTION------------------------------*/

static Errcode bmp_read_rgb_line(XFILE *f, BITMAPINFOHEADER *info, Rgb3 *out)
/* Read in a single line of RGB data from a 24 bit BMP and convert it
 * from BGR to RGB format. */
{
	BGR a;
	int32_t width = info->biWidth;
	int read_width;
	Errcode err;

	while (--width >= 0)
	{
		if ((err = read_var(f, a)) < Success)
			return err;
		out->r = a.r;
		out->g = a.g;
		out->b = a.b;
		++out;
	}
	/* Skip a few bytes possibly to round to next longword boundary. */
	read_width = info->biWidth*3;
	if ((width = NEXT_QUAD(read_width) - read_width) != 0)
		xfseek(f, width, XSEEK_CUR);
	return Success;
}

/*-------------------------BMP WRITE SECTION--------------------------------*/
static void init_header(int32_t width, int32_t height
, 	BITMAPFILEHEADER *head, BITMAPINFOHEADER *info)
/* Set up the headers with proper values for this width and height
 * (and 256 colors.) */
{
	long pixel_data_size = bytes_per_row(width,8) * height;

	clear_struct(head);
	head->bfType = BM_MAGIC;
	head->bfOffBits = sizeof(*head) + sizeof(*info)	+ COLORS*4;
	head->bfSize = head->bfOffBits + pixel_data_size;

	info->biSize = sizeof(*info);
	info->biWidth = width;
	info->biHeight = height;
	info->biPlanes = 1;
	info->biBitCount = 8;
	info->biCompression = BI_RGB;
	info->biSizeImage = pixel_data_size;
	info->biXPelsPerMeter = 0;			
	info->biYPelsPerMeter = 0;
	info->biClrUsed = COLORS;
	info->biClrImportant = COLORS;
}

static Errcode write_colors(XFILE *f, Cmap *c)
/* Write out all the colors in bgr order. */
{
	Rgb3 *in = c->ctab;
	BGRA out;
	int i;
	Errcode err;

	out.a = 0;		/* zero out the extra field. */
	i = COLORS;
	while (--i >= 0)
	{
		out.r = in->r;
		out.g = in->g;
		out.b = in->b;
		if ((err = write_var(f, out)) < Success)
			break;
		++in;		/* Go on to next color. */
	}
	return err;
}

static Errcode write_pixels(XFILE *f, BITMAPINFOHEADER *info, Rcel *screen)
/* Write out the pixels from screen.  Write from bottom to top, and
 * pad each line with zeroes as necessary. */
{
	void *buf = NULL;
	int32_t width = info->biWidth;
	int bpr = bytes_per_row(width,8);
	Errcode err = Success;
	int32_t i = info->biHeight;

	if ((buf = pj_zalloc(bpr)) == NULL)
	{
		err = Err_no_memory;
		goto ERROR;
	}
	while (--i >= 0)
	{
		pj_get_hseg(screen, buf, 0, i, width);
		if ((err = write_buf(f, buf, bpr)) < Success)
			goto ERROR;
	}
ERROR:
	pj_freez(&buf);
	return err;
}

static Errcode write_after_header(XFILE *f
,	BITMAPFILEHEADER *head, BITMAPINFOHEADER *info, Rcel *screen)
/* Write out everything after the file header and info - colors and pixels. */
{
	Errcode err;
	(void)head;

	if ((err = write_colors(f, screen->cmap)) < Success)
		return err;
	return write_pixels(f, info, screen);
}


/*-------------------------PJ GLUE SECTION---------------------------------*/

typedef struct bmp_image_file 
/* This is the structure we return as a handle to open image file to 
 * the PDR client. */
{
	Image_file hdr;				/* It better start with and Image_file. */
	XFILE *file;
	BITMAPFILEHEADER bh;
	BITMAPINFOHEADER bi;
	enum Info_type info_type;
} Bmp_image_file;

static Boolean is_open = 0;						/* lock data structures
											     * to prevent double open.
												 * etc. */


static Boolean spec_best_fit(Anim_info *ainfo)
/* Tell host that we can only write 8 bit-a-pixel images,  and only one
 * frame.  No need to check width and height, since BMP format handles
 * any width/height. */
{
	Boolean nofit;

	nofit = (ainfo->depth == 8
			 && ainfo->num_frames == 1);
	ainfo->depth = 8;
	ainfo->num_frames = 1;
	return(nofit);	/* return whether fit was exact */
}

static void close_file(Image_file **pif)
/* Clean up resources used by BMP reader/writer */
{
	Bmp_image_file **pf = (Bmp_image_file **)pif;
	Bmp_image_file *f;

	if(pf == NULL || (f = *pf) == NULL)
		return;
	if(f->file)
		xfclose(f->file);
	pj_free(f);
	*pf = NULL;
	is_open = FALSE;
}

static Errcode open_helper(Bmp_image_file **pf, char *path, char *rwmode)
/* Check path suffix.  Allocate Bmp_image_file structure.  Open up a file.  
 * Return Errcode if any problems. */
{
	Errcode err = Success;
	Bmp_image_file *f;

	*pf = NULL;

	if(is_open)
		return(Err_too_many_files);

	if (!suffix_in(path, ".BMP") 
	&& !suffix_in(path, ".RLE") 
	&& !suffix_in(path, ".DIB"))
		return(Err_suffix);

	if((f = pj_zalloc(sizeof(*f))) == NULL)
		return(Err_no_memory);

	if ((f->file = xfopen(path, rwmode)) == NULL)
		err = xerrno();

	is_open = TRUE;
	*pf = f;
	return(err);
}


static Errcode open_file(Pdr *pd, char *path, Image_file **pif, 
							 Anim_info *ainfo )
/* Check path for BMP suffix.  If there open it and read in header.
 * Return Errcode if header is bad or other failure. */
{
	Errcode err;
	Bmp_image_file **pf,*f;
	(void)pd;

	pf = (Bmp_image_file **)pif;

	if ((err = open_helper(pf, path, "rb")) < Success)
		goto ERROR;
	f = *pf;
	if ((err = read_head(f->file, &f->bh, &f->bi)) < Success)
		goto ERROR;
	f->info_type = err;
	if(ainfo)
	{
		memset(ainfo,0,sizeof(*ainfo));
		ainfo->width = f->bi.biWidth;
		ainfo->height = f->bi.biHeight;
		ainfo->num_frames = 1;
		ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
		if (f->bi.biBitCount > 8)
		{
			ainfo->depth = f->bi.biBitCount;
		}
		else
		{
			ainfo->depth = 8;		/* we'll always convert it to 8 bits */
		}
	}
	return(Success);

ERROR:
	close_file(pif);
	return(err);
}

static Errcode create_file(Pdr *pd, char *path, Image_file **pif, 
							   Anim_info *ainfo )
/* Make sure path  has .BMP suffix. Create BMP file (but don't write 
 * anything to it yet).  Save ainfo where we can get to it later. */
{
	Errcode err;
	Bmp_image_file **pf,*f;
	(void)pd;

	pf = (Bmp_image_file **)pif;
	if((err = open_helper(pf, path, "wb")) < Success)
		goto ERROR;
	f = *pf;
	init_header(ainfo->width, ainfo->height, &f->bh, &f->bi);
	if ((err = write_var(f->file, f->bh)) < Success)
		goto ERROR;
	if ((err = write_var(f->file, f->bi)) < Success)
		goto ERROR;
	return(Success);

ERROR:
	close_file(pif);
	return(err);
}

static Errcode read_first_frame(Image_file *ifile, Rcel *screen)
/* Seek to the beginning of an open  BMP file, and then read in the
 * first frame of image into screen.  (In our case read in the only
 * frame of image.) */
{
	Bmp_image_file *f = (Bmp_image_file *)ifile;
	return(read_after_header(f->file, &f->bh, &f->bi, f->info_type, screen));
}

static Errcode read_next(Image_file *ifile,Rcel *screen)
/* Read in subsequent frames of image.  Since we only have one  this
 * routine is pretty trivial. */
{
	(void)ifile;
	(void)screen;
	return(Success);
}


static Errcode save_frame(Image_file *ifile, Rcel *screen, ULONG num_frames,
						      Errcode (*seek_frame)(int ix,void *seek_data),
						      void *seek_data, Rcel *work_screen ) 
/* Save file.   If this were a multi-frame file format it could get
 * complex (see comments in picdrive.h).  For us just save what's
 * in screen to the open ifile. */
{
	Bmp_image_file *f = (Bmp_image_file *)ifile;
	(void)num_frames;
	(void)seek_frame;
	(void)seek_data;
	(void)work_screen;

	return(write_after_header(f->file, &f->bh, &f->bi, screen));
}

static Errcode rgb_read_nextline(Image_file *ifile, Rgb3 *outbuf)
/* Read in next line of RGB data. */
{
	Bmp_image_file *f = (Bmp_image_file *)ifile;
	return bmp_read_rgb_line(f->file, &f->bi, outbuf);
}

static Errcode rgb_read_seekstart(Image_file *ifile)
/* Seek to start of RGB data. */
{
	Bmp_image_file *f = (Bmp_image_file *)ifile;
	long fpos;

	if ((fpos = xfseek(f->file, f->bh.bfOffBits, XSEEK_SET)) < Success)
		return fpos;
	return 1;		/* It's flipped. */
}

/*--------------------DRIVER HEADER SECTION--------------------------------*/

static char bmp_pdr_name[] = "BMP.PDR";
static char title_info[] = "Microsoft Windows BMP, DIB and RLE Files.";

static Pdr bmp_pdr_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, NULL, NULL, NULL },
	title_info,  		/* title_info */
	"",  				/* long_info */
	".BMP;.RLE;.DIB", 	/* default_suffi */
	1,1,		 		/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	create_file,		/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file,			/* (*close_image_file)() */
	read_first_frame,	/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	save_frame,			/* (*save_frames)() */
	NULL,				/* Pdroptions pointer */
	rgb_read_seekstart, 	/* (*rgb_seekstart)() */
	rgb_read_nextline,		/* (*rgb_readline()() */
};

Local_pdr bmp_local_pdr = {
	NULL,
	bmp_pdr_name,
	&bmp_pdr_header
};
