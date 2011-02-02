/*****************************************************************************
 * TIFF.H - Header file included in all TIFF driver C modules.
 ****************************************************************************/

#include "stdtypes.h"
#include "errcodes.h"
#include "stdio.h"
#include "syslib.h"
#include "gfx.h"
#include "ptrmacro.h"
#include "picdrive.h"

/*----------------------------------------------------------------------------
 * Tweakable constants...  (recompile all modules after changing these!)
 *--------------------------------------------------------------------------*/

#define UNLZW_TABLE_SIZE			(8 * 4096 + 256) /* ladder + ascii tables */
#define OUTPUT_IDEAL_STRIPSZ		8192
#define OUTPUT_TIFF_VERSION 		42 /* must always be 42 regardless of version */

#define SWAPW(x)					if (tf->swap_bytes) swapw((x))
#define SWAPD(x)					if (tf->swap_bytes) swapd((x))

/*----------------------------------------------------------------------------
 * Constants naming TIFF-defined values...
 *--------------------------------------------------------------------------*/

#define INTEL						0x4949
#define MOTOROLA					0x4d4d

#define SIZE_BYTE					sizeof(char)
#define SIZE_ASCII					sizeof(char)
#define SIZE_SHORT					sizeof(short)
#define SIZE_LONG					sizeof(long)
#define SIZE_RATIONAL				sizeof(Rational)

#define TYPE_BYTE					1
#define TYPE_ASCII					2
#define TYPE_SHORT					3
#define TYPE_LONG					4
#define TYPE_RATIONAL				5

#define CMPRS_NONE					1
#define CMPRS_1DHUFFMAN 			2
#define CMPRS_LZW					5
#define CMPRS_WNONE 				32771
#define CMPRS_PACKBITS				32773

#define PHMET_GREY_0ISWHITE 		0
#define PHMET_GREY_0ISBLACK 		1
#define PHMET_RGB					2
#define PHMET_PALETTE_COLOR 		3
#define PHMET_TRANPARENCY_MASK		4

#define TAG_NEWSUBFILE_TYPE 		0x00fe
#define TAG_SUBFILE_TYPE			0x00ff
#define TAG_IMAGE_WIDTH 			0x0100
#define TAG_IMAGE_LENGTH			0x0101
#define TAG_BITS_PER_SAMPLE 		0x0102
#define TAG_COMPRESSION 			0x0103
#define TAG_PHOTOMETRIC_INTERP		0x0106
#define TAG_THRESHOLDING			0x0107
#define TAG_CELL_WIDTH				0x0108
#define TAG_CELL_LENGTH 			0x0109
#define TAG_FILL_ORDER				0x010a
#define TAG_DOCUMENT_NAME			0x010d
#define TAG_IMAGE_DESCRIPTION		0x010e
#define TAG_MAKE					0x010f
#define TAG_MODEL					0x0110
#define TAG_STRIP_OFFSETS			0x0111
#define TAG_ORIENTATION 			0x0112
#define TAG_SAMPLES_PER_PIXEL		0x0115
#define TAG_ROWS_PER_STRIP			0x0116
#define TAG_STRIP_BYTE_COUNTS		0x0117
#define TAG_MIN_SAMPLE_VALUE		0x0118
#define TAG_MAX_SAMPLE_VALUE		0x0119
#define TAG_X_RESOLUTION			0x011a
#define TAG_Y_RESOLUTION			0x011b
#define TAG_PLANAR_CONFIG			0x011c
#define TAG_PAGE_NAME				0x011d
#define TAG_X_POSITION				0x011e
#define TAG_Y_POSITION				0x011f
#define TAG_FREE_OFFSETS			0x0120
#define TAG_FREE_BYTE_COUNTS		0x0121
#define TAG_GRAY_RESP_UNIT			0x0122
#define TAG_GRAY_RESP_CRVE			0x0123
#define TAG_GROUP_3_OPT 			0x0124
#define TAG_GROUP_4_OPT 			0x0125
#define TAG_RESOLUTION				0x0128
#define TAG_PAGE_NO 				0x0129
#define TAG_COLOR_RSP_CRV			0x012D
#define TAG_PREDICTOR				0x013D
#define TAG_COLORMAP				0x0140
#define TAG_SHORT_STRIP_OFFSETS 	32768  /*Aldus private-short strip offsets */

/*----------------------------------------------------------------------------
 * Typedefs for TIFF data structures...
 *--------------------------------------------------------------------------*/

typedef union onum {			/* a 'value' from a tiff directory entry  */
	char			byte;		/* can be word, dword, file offset, etc.  */
	short			word;
	unsigned short	uword;
	long			dword;
	unsigned long	udword;
	long			offset;
	} Onum;

typedef struct rational {
	long upper;
	long lower;
	} Rational;

typedef struct dirent {
	short	tag;
	short	type;
	long	count;
	Onum	value;
	} Dirent;

typedef struct tifhdr {
		short machine;
		short version;
		long  firstifd;
		} Tifhdr;

/*----------------------------------------------------------------------------
 * Typedefs for structures we use to keep track of reality...
 *--------------------------------------------------------------------------*/

typedef struct strip_data { 	/* used to store strip offset/size pairs */
	long	offset;
	long	size;
	} Strip_data;

typedef struct tiff_file {		/* the main juju; gets passed to everyone...*/

	Image_file	hdr;			 /* PJ Image_file */

	FILE		*file;			 /* input file */
	Rcel		*screen_rcel;	 /* PJ output screen raster */
	long		off_ifd_start;	 /* start of current ifd */
	long		off_ifd_next;	 /* offset of next ifd */
	long		off_ifd_cur;	 /* offset into current ifd */
	int 		image_row_cur;	 /* file row we're currently working on */
	int 		swap_bytes; 	 /* do we need to swap motorola-style data? */
	long		longest_strip;	 /* size of strip buffer */
	UBYTE		*color_table;	 /* -> alloc'd color translation table */
	void		*unlzwtable;	 /* a data table used by unlzw routine */
	Strip_data	*strip_data;	 /* -> allocated strip data array */

	UBYTE		*rbuf;
	UBYTE		*gbuf;
	UBYTE		*bbuf;
	UBYTE		*stripbuf;
	UBYTE		*lzwbuf;
	int 		rows_in_buffer;
	long		offset_in_buffer;
	int 		strip_index;
	Strip_data	*rstrip_data;
	Strip_data	*gstrip_data;
	Strip_data	*bstrip_data;

	long		width;
	long		height;
	short		pixel_depth;
	long		strips_per_image;
	long		rows_per_strip;
	long		compression;
	short		samples_per_pixel;
	short		planar_configuration;
	short		min_sample_value;
	short		max_sample_value;
	short		photometric;
	short		bits_per_sample[3];

	} Tiff_file;

/*----------------------------------------------------------------------------
 * Prototypes for global functions...
 *--------------------------------------------------------------------------*/

/* in tifutil.asm */

extern void swapw(void *addr);
extern void swapd(void *addr);
extern void unpack_samples(char *source, char *dest, int width, int bps);
extern void xlatebuffer(char *xltab, char *input, char *output, int width);
extern void xlate2rgb(Rgb3 *ctab, char *input, char *output, int width);

/* in tifread.c */

extern long    calc_maxdata(Tiff_file *tf);
extern Errcode read_tiftags(Tiff_file *tf);
extern Errcode read_filehdr(Tiff_file *tf);
extern Errcode read_strip(Tiff_file *tf, char *stripbuf, Strip_data *psdata);

/* in tifwrite.c */

extern Errcode write_tiftags(Tiff_file *tf, char *wrkbuf);
extern Errcode write_filehdr(Tiff_file *tf);
extern Errcode write_strip(Tiff_file *tf, char *stripbuf, long striplen, int stripnum);

/* in mtoscrn.c */

extern Errcode toscreen_monoplane_image(Tiff_file *tf);

/* in mfrnscrn.c */

extern Errcode fromscreen_monoplane_image(Tiff_file *tf, int photomet, int comp);

/* in unhufman.asm */

extern void *decmprs2(void *input, void *output, int line_length);

/* in unlzw.asm */

extern void    unlzw_init(void *datatable);
extern Errcode unlzw(void *input, void *output, void *datatable, int buflen);

/* in lzwcmprs.c */

extern Errcode	lzw_init(int bufmaxlen);
extern void 	lzw_cleanup(void);
extern Errcode	lzw_compress(UBYTE *inbuf, UBYTE *outbuf, int count);

/* in packbits.c */

extern int packbits(char *input, char *output, int input_line_length);
extern void *unpackbits(char *input, char *output, int output_line_length);

/* in rgblines.c */

Errcode rgb_readline(Tiff_file *tf, Rgb3 *linebuf);
Errcode rgb_seekstart(Tiff_file *tf);

/* for debugging... */

#if defined(DEBUG_TOSCREEN)
  #define DEBUG_OUTPUT 1
  extern void debug_output(char *string);
  extern void dos_put_char(int chr);
#elif defined(DEBUG_TOFILE)
  #define DEBUG_OUTPUT 1
  extern void debug_output(char *string);
#else
  #define debug_output(a) /**/
#endif

/* end of TIFF.H */
