#ifndef PICT_H
#define PICT_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef STDIO_H
	#include <stdio.h> 
#endif

#ifndef SYSLIB_H
	#include "syslib.h"
#endif

#ifndef PICDRIVE_H
	#include "picdrive.h"
#endif

#ifndef I86SWAP_H
	#include "i86swap.h"
#endif

#undef PBOX
#define PBOX printf("%s %d\n", __FILE__,__LINE__ );

#ifdef SLUFFED  /* from a public domain header file */

/* Opcodes */
#define PICT_NOP		0x00
#define PICT_clipRgn		0x01
#define PICT_bkPat		0x02
#define PICT_txFont		0x03
#define PICT_txFace		0x04
#define PICT_txMode		0x05
#define PICT_spExtra		0x06
#define PICT_pnSize		0x07
#define PICT_pnMode		0x08
#define PICT_pnPat		0x09
#define PICT_thePat		0x0A
#define PICT_ovSize		0x0B
#define PICT_origin		0x0C
#define PICT_txSize		0x0D
#define PICT_fgColor		0x0E
#define PICT_bkColor		0x0F
#define PICT_txRatio		0x10
#define PICT_picVersion		0x11
#define PICT_line		0x20
#define PICT_line_from		0x21
#define PICT_short_line		0x22
#define PICT_short_line_from	0x23
#define PICT_long_text		0x28
#define PICT_DH_text		0x29
#define PICT_DV_text		0x2A
#define PICT_DHDV_text		0x2B
#define PICT_frameRect		0x30
#define PICT_paintRect		0x31
#define PICT_eraseRect		0x32
#define PICT_invertRect		0x33
#define PICT_fillRect		0x34
#define PICT_frameSameRect	0x38
#define PICT_paintSameRect	0x39
#define PICT_eraseSameRect	0x3A
#define PICT_invertSameRect	0x3B
#define PICT_fillSameRect	0x3C
#define PICT_frameRRect		0x40
#define PICT_paintRRect		0x41
#define PICT_eraseRRect		0x42
#define PICT_invertRRect	0x43
#define PICT_fillRRect		0x44
#define PICT_frameSameRRect	0x48
#define PICT_paintSameRRect	0x49
#define PICT_eraseSameRRect	0x4A
#define PICT_invertSameRRect	0x4B
#define PICT_fillSameRRect	0x4C
#define PICT_frameOval		0x50
#define PICT_paintOval		0x51
#define PICT_eraseOval		0x52
#define PICT_invertOval		0x53
#define PICT_fillOval		0x54
#define PICT_frameSameOval	0x58
#define PICT_paintSameOval	0x59
#define PICT_eraseSameOval	0x5A
#define PICT_invertSameOval	0x5B
#define PICT_fillSameOval	0x5C
#define PICT_frameArc		0x60
#define PICT_paintArc		0x61
#define PICT_eraseArc		0x62
#define PICT_invertArc		0x63
#define PICT_fillArc		0x64
#define PICT_frameSameArc	0x68
#define PICT_paintSameArc	0x69
#define PICT_eraseSameArc	0x6A
#define PICT_invertSameArc	0x6B
#define PICT_fillSameArc	0x6C
#define PICT_framePoly		0x70
#define PICT_paintPoly		0x71
#define PICT_erasePoly		0x72
#define PICT_invertPoly		0x73
#define PICT_fillPoly		0x74
#define PICT_frameSamePoly	0x78
#define PICT_paintSamePoly	0x79
#define PICT_eraseSamePoly	0x7A
#define PICT_invertSamePoly	0x7B
#define PICT_fillSamePoly	0x7C
#define PICT_frameRgn		0x80
#define PICT_paintRgn		0x81
#define PICT_eraseRgn		0x82
#define PICT_invertRgn		0x83
#define PICT_fillRgn		0x84
#define PICT_frameSameRgn	0x88
#define PICT_paintSameRgn	0x89
#define PICT_eraseSameRgn	0x8A
#define PICT_invertSameRgn	0x8B
#define PICT_fillSameRgn	0x8C
#define PICT_BitsRect		0x90
#define PICT_BitsRgn		0x91
#define PICT_PackBitsRect	0x98
#define PICT_PackBitsRgn	0x99
#define PICT_shortComment	0xA0
#define PICT_longComment	0xA1
#define PICT_EndOfPicture	0xFF

#endif /* SLUFFED */


/* #define PRINTSTUFF  /* if you want printfs of opcodes and file offsets */

/* data types used in Pict files */

typedef unsigned long FPOINT; 

typedef struct picrect {
	SHORT top, left, bot, right; 
} pRect;

typedef struct pictpoint {
	SHORT x,y;
} pPoint;

typedef struct pictrgb {
	USHORT r,g,b;
} RGBColor;


typedef struct picthead {
	USHORT size16;  /* old size ignore this */
	pRect frame;  /* the bounding rect of whole pic */
} Picthead;


/* version 1 style bitmap header */

typedef struct pictbmap {
	LONG myflags; /* baseAddr; */  /* NOT in file !! I use for my own data */
	USHORT rowBytes;   /* must be even, bytes per row */
	pRect Bounds;    /* rectangle bounding this bitMap */ 
} bitMap;

/* pixel map record for version 2 files */

typedef struct pixmap {
	LONG myflags; /* baseAddr; */  /* NOT in file !! I use for my own data */
	USHORT rowBytes;   /* must be even, bytes per row */
	pRect Bounds;    /* rectangle bounding this pixMap */ 
	SHORT version;
	SHORT packType;
	LONG packSize;
	FPOINT hRes;    /* res in DPI always 0048:0000 from what I can see */
	FPOINT vRes;    /* res in DPI always 0048:0000 from what I can see */
	SHORT pixelType;  	/* 0 */
	SHORT pixelSize;   /* bits per pixel */
	SHORT cmpCount;  /* number of components in a pixel */
	SHORT cmpSize;	 /* size of pixel components */
	LONG planeBytes;  /* size of single plane */
	LONG pmTable;
	LONG reserved;
} pixMap;

/* definitions of what part of a pixMap actually goes in the file */

#define PMAP_FSTART  rowBytes
#define PMAP_FSIZE   (sizeof(pixMap)-OFFSET(pixMap,PMAP_FSTART))

/* items put in myflags */

#define PM_NOCTABLE 0x0001 /* there is no Cmap associated with this pixMap */

/* packing types */

enum paktypes {
	PACK_BRUN = 0,   /* byte run for pixels <= 8 bits */
	NOPACK,		 	/* un compressed */
	NOPACK_ODDSIZE,	 /* uncompressed but data is odd size chunks so a
				      * pad byte is added in (apples) core, for 24 bit
					  * rgbs the read in buffer will be rgbrgbrgb as bytes */
	PACK_CHUNKRUN,   /* run by size of pixel chunks */
	PACK_BRUN_CMP,   /* byte run one component at a time in a contiguous
					  * buffer */
};

typedef struct colortable {
	LONG id;
	USHORT ctFlags; 	/* flags of some sort */
	USHORT ctSize; 		/* number of color data entries minus one */
	/* followed by color data in ctEntry array */
} colorTable;

/* bit 15 set in the ctFlags refers to the coloTable as it is on a 
 * "device" bit 14 I found a 
 * reference to being an explicit palette user setting to indicate 
 * reference to the entries as indexed by the pixel values. Apple docs
 * are definately the worst */

#define CT_DEVICE 0x8000
#define CT_INDEXED 0x4000

/* entry in color table */

typedef struct ctentry {
	USHORT pixel_ix;
	USHORT r,g,b;
} ctEntry;

#define CT_ENTRIES(pct)  ((ctEntry *)((pct)+1)) /* pointer to first entry */


/* structure associated with an open Pict file.  Basicly all state info needed
 * by the parser for this file. By keeping it all here and making the reader
 * parser code re-entrant more than one could be open if this is converted to
 * a multi tasking system library */

#define  Pfile struct pict_image_file  /* NOT a typedef */

struct pict_image_file {
	Image_file hdr;
	FILE *file;		/* buffered file handle */
	Rcel *screen;  	/* drawing screen */
	Errcode lasterr;  /* last error returned to host if aborted */
	SHORT flags;
	SHORT mode;     /* scanning ? RGB ? */

	Anim_info ainfo; /* info created with or opened with */

	/* version dependent items */
	USHORT version;  	/* version 2 or version 1 */


	USHORT op;			/* current opcode */
	LONG bytes_since;   /* bytes read since last opcode, only used in 
						 * version 2 */
	LONG opstart;       /* offset of first opcode after header and magic
						 *  The place to start parsing during the read
						 * and display phase */

	Errcode (*read_opcode)(void *ifile);  /* function to read the opcode
										   * version 1 and version 2 use 
										   * different protocals */

	pRect frame;    /* rectangle bounding this picture that every thing else
					 * is relative to */ 

	/* current state info for bitmap parser */

	pixMap pm;			 /* header for the one we are reading */
	colorTable *cTable;  /* color table, only if allocated for 
						  * 1,2,4, and 8 bit depths */
	pRect srcRect;		 /* I don't use this, I guess it could be smaller
						  * than the pixMap */
	pRect dstRect;		 /* where to put the pixMap on the picture */
	SHORT tmode;		 /* transfer mode how to blit pixMap into screen */
	SHORT pixwidth;		 /* the output pixel width in bytes */
	SHORT pmheight;		 /* the height of the pixel map */
	LONG pbsize;         /* pixel crunch buffer size */
	UBYTE *pixbuf;       /* work buffer for pixel crunching */
	UBYTE *maxpix;		 /* end of pixel buffer set to pixbuf + pixwidth */
	UBYTE *bitbuf;		 /* buffer where decompressed bits are put */
	UBYTE *readbuf;		 /* buffer where file chunks are read into */

	USHORT max_read_size; /* max size to allow when reading chunks */
	/* compressed line chunk reader */
	Errcode (*read_line)(Pfile *pf,void *buf,int maxsize); 
	/* bit unpacker */
	Errcode (*unpack_line)(BYTE *packline, UBYTE *buf, int len); 
	/* bit arranger bitbuf to pixbuf converter */
	void (*make_pixels)(UBYTE *bitbuf,UBYTE *pixbuf, UBYTE *maxpix, int bpr);
	/* pixel compositor s == line from file, d == current screen line */ 
	void (*composite_pixels)(UBYTE *s,UBYTE *d);

	/* RGB parsing state */

	LONG first_pmap_oset; /* offset for first pixel map */
	SHORT last_rgbline;	  /* Y value of last rgbline read -1 after seekstart */
	SHORT cur_rgbline;	  /* Y value of current line in file */

	/* drawing state Wishful thinking ! not used */

	pRect lastRect;		 /* current rectangle setting */
	SHORT lastRWidth;    /* width of last rect */
	SHORT lastRHeight;   /* height of last rect */
	Boolean got_bitmap;	 /* Have processed bitmap? */
};

/* flags values */

#define PH_CTAB_INORDER 0x0001  /* color table is in index order */

/* mode values */

#define PF_SCANINFO 	0
#define PF_FULLFRAME    1
#define PF_RGBLINES     2

/* return codes for do_pictops() all < 0 are Errcodes */

Errcode do_pictops(Pfile *);
typedef Errcode (*Do_pictop)(Pfile *); /* typedef for vector table */

#define RET_EOPIC		1
#define RET_ENDSCAN 	2
#define RET_PMAPSTART   3  /* we just initialized the next pixel map */

/* utilities mostly in pictutil.c */

void freez(void *pmem);  /* if *pmem != NULL free and set *pmem to NULL */
stuff_bytes(UBYTE data, void *buf, unsigned count);
void copy_bytes(void *src,void *dst,int count);

/* geometry and graphics utils */
void offset_pRect(pRect *r, SHORT x, SHORT y);
void set_rect_relto(pRect *r, pRect *relto);

/* write calls and utilities */
Errcode pf_write(Pfile *pf, void *buf, int size);
Errcode write_chunk8(Pfile *pf,void *buf, int size);
Errcode write_chunk16(Pfile *pf,void *buf, int size);
char *brun_pack_line(char *src, char *cbuf, int count);
Errcode brun_unpack_line(BYTE *packline, UBYTE *buf, int len);
Errcode wrun_unpack_line(BYTE *packline, UBYTE *buf, int len);
Errcode brun_unpack_3compbytes(BYTE *packline, UBYTE *buf, int len);

/* read calls */
Errcode pf_read_oset(Pfile *pf,void *buf,LONG size,LONG offset);
Errcode pf_read(Pfile *pf,void *buf,LONG size);
Errcode pf_seek_bytes(Pfile *pf, LONG count);

/* these do intel swapping on specific data types */

Errcode read_short(Pfile *pf, SHORT *ps);
Errcode read_pRect(Pfile *pf, pRect *rect);

/* generic fixed size opcode skipper that reads size from a table */

Errcode skip_fixedop(Pfile *pf);

/* 3 basic variable chunk types 8, 16, 32 bit imbedded size 
 * value in size size of following data exclusive of the 
 * size consumed by the imbedded size integer */

Errcode read_chunk8(Pfile *pf, void *buf, unsigned maxsize);
Errcode skip_chunk8(Pfile *pf);
Errcode write_chunk8(Pfile *pf, void *buf, int size);

Errcode read_chunk16(Pfile *pf, void *buf, unsigned maxsize);
Errcode skip_chunk16(Pfile *pf);
Errcode write_chunk16(Pfile *pf, void *buf, int size);

Errcode read_chunk32(Pfile *pf, void *buf);
Errcode skip_chunk32(Pfile *pf);
Errcode write_chunk32(Pfile *pf, void *buf, LONG size);


#endif /* PICT_H */
