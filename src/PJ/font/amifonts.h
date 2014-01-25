
#ifndef AMIFONTS_H
#define AMIFONTS_H

#include "stdtypes.h"

#define FPF_PROPORTIONAL (1L<<5)

#define AMIF_TYPE 0x0c

typedef struct afont_head1
	{
	UBYTE garbage[0x2c];	/* ignore */
	UBYTE ln_type;			/* == 0x0c */
	UBYTE ln_priority;		/* ignore */
	ULONG ln_name;			/* ignore */
	USHORT dfh_id;			/* 0x0f80 on 68000 */
	USHORT revision;			/* 1 on 68000 */
	ULONG segment;			/* ignore */
	char name[32];			/* actually not filled in */
	} Afont_head1;
typedef struct afont_head2
	{
	ULONG fln_succ;			/* ignore */
	ULONG fln_pred;			/* ignore */
	UBYTE fln_type;			/* == 0x0c */
	UBYTE fln_priority;		/* ignore */
	ULONG fln_name;			/* ignore */
	ULONG mn_replyport;		/* ignore */
	USHORT mn_length;		/* size of font from fln_succ to end of file */
	USHORT tf_ysize;			/* font image height */
	UBYTE tf_style;			/* style of font */
	UBYTE tf_flags;			/* mono? proportional?  designed? */
	USHORT tf_xsize;			/* widest cel width */
	USHORT tf_baseline;		/* distance from top of image to base-line */
	USHORT tf_boldsmear;		/* good to make bold by double-strike? */
	USHORT tf_accessors;		/* ignore */
	UBYTE tf_lochar;		/* lowest char in font */
	UBYTE tf_hichar;		/* highest char in font */
	ULONG tf_chardata;		/* offset to character data */
	USHORT tf_modulo;		/* # of bytes in line of font image */
	ULONG tf_charloc;		/* offset to location table */
	ULONG tf_charspace;		/* offset to spacing table */
	ULONG tf_charkern;		/* offset to kerning table */
	} Afont_head2;

typedef struct afont_head
	{
	Afont_head1 h1;
	Afont_head2 h2;
	} Afont_head;

typedef struct font_loc
	{
	USHORT startx;
	USHORT width;
	} Font_loc;

typedef struct afcb		/* Amiga font control block */
	{
	Afont_head head;
	UBYTE *image;
	Font_loc *loc;
	USHORT *spacing;
	SHORT *kerning;
	UBYTE is_prop;
	UBYTE is_kerned;
	} Afcb;


#endif /* AMIFONTS_H */
