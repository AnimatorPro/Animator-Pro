
#ifndef GEMFONT_H
#define GEMFONT_H
/* This file supports GEM style fonts.  They live on disk in three parts.
   1st there's the header structure below, then a list of 'x' offsets into
   the data - one 16-bit word for each offset, and 1 offset for each letter
   in the font plus an extra offset at the end.

   This is followed by the data which is a single bitmap.
   */

struct	font_hdr {
int	id;			/* some random number, doesnt matter */
int	size;		/* Size in points.  Somehow related to pixel height. */
char	facename[32];	/* Give it a name, don't really matter. */
int	ADE_lo;		/* Lowest ascii character in font */
int	ADE_hi;		/* Highest ascii character in font */
int	top_dist;
int	asc_dist;	/* Ascender to baseline?? */
int	hlf_dist;
int	des_dist;	/* des for descender. */
int	bot_dist;
int	wchr_wdt;	/* Widest character width. */
int	wcel_wdt;	/* Widest 'cell' width (includes distance to next character) */
int	lft_ofst;
int	rgt_ofst;
int	thckning;
int	undrline;
int	lghtng_m;	/* Lightening mask.  Just use 0x55aa. */
int	skewng_m;	/* Skewing mask for italics. If 1 bit rotate this line. 0xaaaa*/
int	flags;		/* Just set to zero.  Half-assed intel swap if otherwise. */
char *hz_ofst;  /* On disk byte offset from beginning of file to hor. offsets */
int	*ch_ofst;	/* On disk byte offset to beginning of ?? kerning ?? data. */
int	*fnt_dta;	/* On disk byte offset to beginning of bitmap. */
int	frm_wdt;	/* Byte width of bitmap. */
int	frm_hgt;	/* Pixel height of bitmap. */
struct font_hdr	*nxt_fnt; /* Set to 0 */
}; 

/* face identifer for font loaded from disk... */
#define CYP_CUSTOM_FONT	0xabc

extern struct font_hdr cfont;
extern struct font_hdr sixhi_font;
extern struct font_hdr *usr_font;

#define STPROP 0
#define MFIXED 1
#define MPROP 2

#endif GEMFONT_H
