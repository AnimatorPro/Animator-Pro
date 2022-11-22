/*****************************************************************************
 * FLI.H - Full mapping of data structures internal to a flic file.
 *
 *	Note:  This is the FlicLib PJFLI.H file, slightly munged.
 *		   DON'T LET THIS FIND ITS WAY BACK INTO THE FLICLIB DIRECTORY!!!
 *
 *	Note:  The STDTYPES.H file must be included before this this file.
 ****************************************************************************/

#ifndef FLI_H
#define FLI_H

/*----------------------------------------------------------------------------
 * magic numbers for Animator Pro file types.
 *	These values appear in the type field of a file header.
 *--------------------------------------------------------------------------*/

#define PIC_MAGIC		0x9500U 	/* animator temp file picture format */
#define FLIH_MAGIC		0xAF11U 	/* low res animator FLI Magic */
#define FLIHR_MAGIC 	0xAF12U 	/* Hi res FLI magic */
#define CMAP_MAGIC		0xB123U 	/* color map file */

/*----------------------------------------------------------------------------
 * chunk id constants.
 *	These values appear in the type field of a Chunk within a file.
 *
 *	The prefixes indicate their usage, as follows:
 *		FCID_something	= File-level chunks
 *		FP_something	= Prefix chunk sub-chunks
 *		FLI_something	= Frame chunk sub-chunks
 *		FPS_something	= Postage stamp data-chunk sub-chunks
 *--------------------------------------------------------------------------*/

#define INVALID_CHUNK_TYPE (-1) 	/* the type -1 is reserved for the parser */

#define FCID_PREFIX  0xf100U		/* Fli prefix chunk (pre frame) */
#define FCID_FRAME	 0xf1faU		/* Fli frame chunk */

enum {								/* prefix sub-chunk types... */
	FP_FREE 		= 0,			/*	unused space */
	FP_FLIPATH		= 1,			/*	fli path, valid in a cel or flx file */
	FP_VSETTINGS	= 2,			/*	vsettings chunk */
	FP_CELDATA		= 3,			/*	cel information chunk */
	};

enum {								/* frame sub-chunk types... */
	FLI_COL 		= 0,
	FLI_WRUN		= 1,
	FLI_WSKIP		= 2,
	FLI_WSKIP2		= 3,
	FLI_COLOR256	= 4,			/*	compressed 256-level color map */
	FLI_WSKIPRUN	= 5,
	FLI_BSKIP		= 6,
	FLI_SS2 		= 7,			/*	normal ss2 delta encoded */
	FLI_BSC 		= 8,
	FLI_SBSC		= 9,
	FLI_SBSRSC		= 10,
	FLI_COLOR		= 11,
	FLI_LC			= 12,			/*	normal FLI delta encoded */
	FLI_COLOR_0 	= 13,			/*	whole frame is color 0 */
	FLI_ICOLORS 	= 14,
	FLI_BRUN		= 15,			/*	normal byte run-length encoded */
	FLI_COPY		= 16,			/*	no-compression, straight copy */
	FLI_SOLID		= 17,
	FLI_PSTAMP		= 18,			/*	"postage stamp" chunk */
	};

enum {								/* postage stamp sub-chunk types... */
	FPS_BRUN	= FLI_BRUN, 		/*	byte run-length encoded */
	FPS_COPY	= FLI_COPY, 		/*	no-compression, straight copy */
	FPS_XLAT256 = FLI_PSTAMP,		/*	translation table */
	};

/*----------------------------------------------------------------------------
 * Fli_head flags contants.
 *--------------------------------------------------------------------------*/

#define FLI_FINISHED 0x0001 		/* flic file was properly closed */
#define FLI_LOOPED	 0x0002 		/* flic file contains ring frame */

/*----------------------------------------------------------------------------
 * postage stamp sizes and color flags.
 *--------------------------------------------------------------------------*/

#define PSTAMP_NOXLAT	0
#define PSTAMP_SIXCUBE	1

#define PSTAMP_W 100
#define PSTAMP_H 63

/*----------------------------------------------------------------------------
 * The Chunk_id datatypes.
 *--------------------------------------------------------------------------*/

typedef struct chunk_id {
	LONG	size;					/* total size of this chunk */
	USHORT	type;					/* type of data in this chunk */
	} Chunk_id;

typedef struct fat_chunk {
	LONG	size;					/* total size of this chunk */
	USHORT	type;					/* type of data in this chunk */
	USHORT	version;				/* version of data in this chunk */
	} Fat_chunk;

/*----------------------------------------------------------------------------
 * The Fli_id datatype.
 *
 *	The time is stored in DOS format, date in high order, time in low order.
 *	The create_user and update_user fields contain the product serial number
 *	of the Animator Pro program that created/updated the file.	If the file
 *	was created by the fliclib routines, the value will be "FLIB".
 *--------------------------------------------------------------------------*/

typedef struct fli_id {
	LONG	create_time;			/* time file created */
	LONG	create_user;			/* user id of creator */
	LONG	update_time;			/* time of last update */
	LONG	update_user;			/* user id of last update */
	} Fli_id;

/*----------------------------------------------------------------------------
 * The Fhead_1_0 datatype (for old-style 320x200 FLI files).
 *
 *	These fields are identical to the first eight fields in the header of
 *	a new-style FLC file, but the speed value is interpreted differently.
 *--------------------------------------------------------------------------*/

typedef struct fhead_1_0 {
	LONG	size;			/* total file size */
	USHORT	type;			/* file type, FLIH_MAGIC for old-style FLI file */
	USHORT	frame_count;	/* frames in flic (not including ring frame) */
	USHORT	width;			/* flic width  (always 320) */
	USHORT	height; 		/* flic height (always 200) */
	USHORT	bits_a_pixel;	/* pixel depth (always 8) */
	SHORT	flags;			/* (mapped out above) */
	SHORT	jiffy_speed;	/* speed (in jiffies for old-style FLI file) */
	UBYTE	pad[110];		/* pad out to 128 bytes */
	} Fhead_1_0;

/*----------------------------------------------------------------------------
 * The Fli_head datatype (for new-style variable-resolution FLC files).
 *	When the type field is FLIH_MAGIC, only the first eight fields are valid.
 *--------------------------------------------------------------------------*/

typedef struct fli_head {
	LONG	size;			/* total file size */
	USHORT	type;			/* file type, FLIH_MAGIC or FLIHR_MAGIC */
	USHORT	frame_count;	/* frames in flic (not including ring frame) */
	USHORT	width;			/* flic width */
	USHORT	height; 		/* flic height */
	USHORT	bits_a_pixel;	/* pixel depth (always 8 for now) */
	SHORT	flags;			/* (mapped out above) */
	LONG	speed;			/* speed (millisecs for FLIHR or jiffies for FLIH) */
	USHORT	unused; 		/* reserved word */
	Fli_id	id; 			/* creator and updater id */
	USHORT	aspect_dx;		/* aspect ratio x (most always 1) */
	USHORT	aspect_dy;		/* aspect ratio y (most always 1) */
	UBYTE	commonpad[38];	/* size down to here s/b total of 80 bytes */
	LONG	frame1_oset;	/* file offset to first frame chunk */
	LONG	frame2_oset;	/* file offset to second frame chunk (for ring) */
	UBYTE	padfill[40];	/* pad to total size of 128 bytes */
	} Fli_head;

/*----------------------------------------------------------------------------
 * The Flifile datatype.
 *	This datatype holds a Fli_head structure for the flic, plus handles for
 *	the file managment routines and data compression routines.	For clients
 *	of the fliclib, the handles point to black-box structures.
 *--------------------------------------------------------------------------*/

typedef struct flifile {
	Fli_head		hdr;		/* the fli hdr for the flic */
	struct jfl		*fd;		/* -> jfile object for this flic */
	struct complib	*comp;		/* -> compression object for output flic */
	} Flifile;

/*----------------------------------------------------------------------------
 * The Fli_frame datatype.
 *--------------------------------------------------------------------------*/

typedef struct fli_frame {
	LONG	 size;
	USHORT	 type;		/* type = FCID_FRAME or FCID_PREFIX */
	SHORT	chunks;
	char	pad[8];
	} Fli_frame;

/*----------------------------------------------------------------------------
 * The Pstamp_chunk datatype.
 *--------------------------------------------------------------------------*/

typedef struct pstamp_chunk {
	LONG	 size;
	USHORT	 type;		/* type = FLI_PSTAMP */
	SHORT	 height;
	SHORT	 width;
	SHORT	 xlat_type;
	Chunk_id data;		/* this is a fli chunk either copy or brun */
	} Pstamp_chunk;

/*----------------------------------------------------------------------------
 * a random prototype or two...
 *--------------------------------------------------------------------------*/

extern void 	pj_fli_close(Flifile *flif);
extern Errcode	pj_fli_open(char *path, Flifile *flif, int jmode);
extern Errcode	pj_fli_read_uncomp(char *name, Flifile *flif, Rcel *fscreen,
								   Fli_frame *ff, int colors);
extern Errcode	pj_fli_alloc_cbuf(Fli_frame **pcbuf, USHORT width, USHORT height,
								  LONG num_colors);
#endif /* FLI_H */
