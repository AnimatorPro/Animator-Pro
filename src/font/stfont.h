#ifndef STFONT_H
#define STFONT_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/* This structure is for GEM style fonts.  They live on disk in three parts.
   1st there's the header structure below, then a list of 'x' offsets into
   the data - one 16-bit word for each offset, and 1 offset for each letter
   in the font plus an extra offset at the end.
   This is followed by the data which is a single bitmap.
   */

typedef struct	font_hdr {
	SHORT id;		/* the font type */
	SHORT size;		/* Size in points.  Somehow related to pixel height. */
	char facename[32];	/* Give it a name, don't really matter. */
	SHORT ADE_lo;	/* Lowest ascii character in font */
	SHORT ADE_hi;	/* Highest ascii character in font */
	SHORT top_dist;
	SHORT asc_dist;	/* Ascender to baseline?? */
	SHORT hlf_dist; /* offset to font y center fronm top ?? */
	SHORT des_dist;	/* des for descender. */
	SHORT bot_dist;
	SHORT wchr_wdt;	/* Widest character pixel width. */
	SHORT wcel_wdt;	/* Widest 'cel' width (includes pix to next character) */
	SHORT lft_ofst;
	SHORT rgt_ofst;
	SHORT thckning;
	SHORT undrline;
	SHORT lghtng_m;	/* Lightening mask.  Just use 0x55aa. */
	SHORT skewng_m;	/* Skewing mask for italics. If 1 bit rotate this line. 
					 * 0xaaaa*/
	SHORT flags;	/* Just set to zero.  Half-assed intel swap if otherwise. */
	char *hz_ofst;  /* On disk byte offset from beginning 
					 * intel file to hor. offsets */
	SHORT *ch_ofst;	/* On disk byte offset to beginning 
					 * of ?? kerning ?? data. */
	SHORT *fnt_dta;	/* On disk byte offset to beginning of bitmap. */
	SHORT frm_wdt;	/* Byte width of bitmap. */
	SHORT frm_hgt;	/* Pixel height of bitmap. */
	struct font_hdr	*nxt_fnt; /* Set to 0 */
} Font_hdr; 

/* ST font id's */
#define STPROP 0
#define MFIXED 1
#define MPROP  2

#endif /* STFONT_H Leave at end of file */
