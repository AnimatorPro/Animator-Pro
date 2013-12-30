/*****************************************************************************
 * TARGA.H - Header file for Targa PDR.
 ****************************************************************************/

#include "picdrive.h"
#include "errcodes.h"
#include "stdio.h"
#include "gfx.h"

/*----------------------------------------------------------------------------
 * constants for targa-defined values...
 *--------------------------------------------------------------------------*/

#define MAP_NONE		0	/* file has no color map	*/
#define MAP_LUT 		1	/* file has default color map, no compression	*/

#define NO_IMAGE		0	/* file has no image (thus, no career in politics) */
#define MAPPED_IMAGE	1	/* image color mapped, i.e. data are indices */
#define RL_MAPPED_IMAGE 9	/* RL image color mapped */
#define RGB_IMAGE		2	/* RGB data in image	*/
#define RL_RGB_IMAGE	10	/* Run Length RGB data in image */
#define BW_IMAGE		3	/* black & white image	*/
#define RL_BW_IMAGE 	11	/* Run Length black & white image	*/
#define RDH_IMAGE		32	/* run-length, delta, huffman	*/
#define RDHBLK_IMAGE	33	/* RDH, in block format 	*/

#define INTLEVMASK		0xC0
#define INTLEV0 		0	/* no interleave	*/
#define INTLEV1 		1	/* odd/even interleave	*/
#define INTLEV2 		2	/* 4-way interleave */

#define SCRORG			0x20

/*----------------------------------------------------------------------------
 * typedefs for targa file headers...
 *--------------------------------------------------------------------------*/

typedef struct bgr3
{
	UBYTE	b;
	UBYTE	g;
	UBYTE	r;
} Bgr3;

typedef struct tcmap
{
	SHORT	 mapidx;		 /*index of first map entry in LUT*/
	SHORT	 mapcnt;		 /*num of elements to be loaded*/
	UBYTE	 mapbits;		 /*number of bits in each element*/
} Tcmap;

typedef struct imgspec
{
	SHORT	 xorg;			 /*x position of lower left corner of image*/
	SHORT	 yorg;			 /*y position of lower left corner of image*/
	SHORT	 width; 		 /*width of image in pixels*/
	SHORT	 height;		 /*height of image in pixels*/
	UBYTE	 pixsiz;		 /*num of bits per pixel*/
	UBYTE	 imgdesc;		 /*contains bit fields of descriptor info*/
} Imgspec;

typedef struct tgaheader
{
	UBYTE	idlength;		/*length of id field*/
	UBYTE	maptype;		/*color map type*/
	UBYTE	imgtype;		/*image storage type*/
	Tcmap	tcmdata;		/*color map info  */
	Imgspec imgdata;		/*image info*/
} Tgaheader;

/*----------------------------------------------------------------------------
 * typedef for our main control structure...
 *-------------------------------------------------------------------------*/

typedef struct targa_file { 	/* the main juju; gets passed to everyone...*/

	Image_file	hdr;			/* PJ Image_file */

	Tgaheader	tgahdr; 		/* TARGA file header data */

	Boolean 	is_rgb; 		/* is it RGB (as opposed to colormapped)? */
	Boolean		is_grey;		/* is it greyscale image */
	Boolean 	is_compressed;	/* is it RLE compressed? */
	Boolean 	is_flipped; 	/* is it upside down in the file? */
	int 		width;			/* image width */
	int 		height; 		/* image height */
	int 		pdepth; 		/* pixel depth */
	int 		bpp;			/* bytes per pixel */
	int 		bpr;			/* bytes per row */
	FILE		*file;			/* file */
	Rcel		*screen_rcel;	/* PJ screen raster */
	int 		curx;			/* screen column we're currently working on */
	int 		cury;			/* screen row we're currently working on */
	Pixel		*hsegbuf;		/* -> current image scanline (pj 8-bit pix) */
	UBYTE		*lbuf;			/* -> current input line buffer */
	UBYTE		*over;			/* -> buffer overflow for decompression */
	int 		over_count; 	/* bytes in overflow buffer */
	long		data_offset;	/* offset of start-of-data in file */

	} Targa_file;

/*----------------------------------------------------------------------------
 * prototypes...
 *--------------------------------------------------------------------------*/

Errcode write_targa_image(Targa_file *tf);
Errcode write_targa_header(Targa_file *tf);

Errcode read_nextline(Targa_file *tf, void *destbuf);
Errcode read_seekstart(Targa_file *tf);
Errcode read_cmapped_image(Targa_file *tf);
Errcode read_targa_header(Targa_file *tf);

