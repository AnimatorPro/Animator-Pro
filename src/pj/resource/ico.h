
/***************************************************************************
 * Ico.h - Description of MicroSoft Windows .ICO file format: 
 *
 * Alas I can't find any documentation at the moment, so the following
 * is reverse engineered from looking at a couple of sample .ico files.
 *
 * Here is an overview of the file structure:
 *
 *	Ico_file_header    ........................ 6 bytes
 *		Ico_file_dir   ........................16 bytes
 *		Ico_file_dir
 *			...
 *		ico_head       ........................28 bytes
 *			color-map
 *				b g r reserved..................4 bytes
 *				b g r reserved
 *			pixel-data
 *			mask-data
 *		ico_head
 *			color-map
 *			pixel-data
 *			mask-data
 *			...
 *
 * A .ICO file can contain several images.  The file starts out with a
 * six byte header that says how many icons are in the file.  This is
 * followed by a "directory" entry of 16 bytes for each icon.  The directory
 * entry among other things includes the offset of the icon proper.
 *
 * An Icon starts with a header.  This may be variable size because the
 * size of the header (or is it the offset to the color map?) is the
 * first word of the icon header.  The icon header among other things
 * says how many colors are in the icon.  I've seen this be either 2, 8,
 * or 16 - though I'm sure on some systems it can be 32.  The curious
 * thing is that 8 color pixels still seem to be allocated 4 bits of
 * data in the image bits proper.  There is a field I'm pretty sure
 * says how many bits are used per pixel in storage.
 *
 * Anyway following the image header is the color-map in RGB? format.
 * The 4th byte seems to be zero.  The number of color map entries
 * seems to be related to the storage bits per pixel rather than the
 * number of colors.
 *
 * After the color map is the color image which seems to
 * be stored in packed pixel format (ie 2 pixels to the byte in 16 & 8 color
 * modes,  1 per byte in 256 color mode, 8 per byte in 2 color.
 * Finally there's a transparency mask for the image.  It is one
 * bit per pixel.  
 ***************************************************************************/

typedef struct ico_file_head
	{
	short u0_0;	/* Unknown, contents 0? */
	short u2_1;  /* Unknown, contents 1? */
	short image_count;	/* # of Icons in this file */
	} Ico_file_head;

typedef struct ico_file_dir	/* One of these for each icon in file */
	{
	char  width;  		/* Pixel width - usually 32 */
	char height;  		/* Pixel height - 16 or 32  */
	short num_colors;	/* Number of colors - 2, 8, 16, 256? */
	short u4_0;			/* Unknown, contents 0? */
	short u6_0;			/* Unknown, contents 0? */
	long icon_size;		/* Size of icon image */
	long icon_offset;	/* Individual icon start position in file */
	} Ico_file_dir;

typedef struct ico_head
/* This structure looks much like a BITMAPINFOHEADER in
 * windows.h, except the Height field isn't accurate. */

	{
	long  head_size;	/* Header size? Always 0x28 though */
	long  width;		/* Accurate?  Always 0x20? */
	long  height;		/* Accurate?  0x20 when it *is* 0x10! */
	short planes;		/* Number of planes of image - usually 1. */
	short bits_per_pixel;	/* Number of bits/pixel in each plane. */
	long  compression;	/* 0 for uncompressed. 1 & 2 are run-length. */
	long image_size;	/* Size if image - mask and pixels together? */
	short reserved[8];		/* Unknown - 0's? */
	} Ico_head;

typedef struct ico_rgb
	{
	unsigned char b,g,r,reserved;	/* Values from 0 to 255 */
	} Ico_rgb;

