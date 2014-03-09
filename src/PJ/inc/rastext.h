#ifndef RASTEXT_H
#define RASTEXT_H 1

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RASTCALL_H
	#include "rastcall.h"
#endif


#define STFONT 0
#define SPEEDO 1
#define HPJET 2
#define AMIFONT 3
#define TYPE1FONT 4

#ifdef VFONT_C
	#define RASType Raster
	#define BCOLvar Pixel bcolor
#else
	#define RASType void
	#define BCOLvar ...  	/* this is optional */
#endif

typedef enum text_mode
	{
	TM_MASK1 = 0,		/* 1's go to color, 0's are transparent. */
	TM_MASK2 = 1,		/* 1's go to color, 0's to bcolor. */
	TM_RENDER = 2,		/* Go draw with ink. */
	TM_OPAQUE = 3,		/* Go draw with opaque ink. */
	} Text_mode;

/* Structure to bundle together various types of fonts so system can
   use them uniformly */


typedef struct vfont
	{
	void *font;				/* pointer to font device data */
	VFUNC close_vfont;		/* unalloc self */

	/* draw text function */
	Errcode (*gftext)(RASType *r,
			struct vfont *f, unsigned char *s, int x, int y,
			Pixel col, Text_mode tmode, Pixel bcol);

	FUNC char_width;		/* pass in font and char string. Returns
							 * width of character imbedded in string 
							 * returns length as end of line char if 
							 * followed by a \0 includes spacing */
	EFUNC scale_font;		/* pass in point size desired. */
	FUNC in_font;			/* see if a character is in font */

	SHORT type;				/* Speedo?  Amiga? HPjet? */
	SHORT flags;			/* Bit defines. */
	SHORT widest_char;		/* width of widest imbedded char
							 * including spacing */
	SHORT widest_image;		/* width of widest imbedded char in font before
							 * spacing */
	SHORT widest_end;		/* widest char as terminal char in line */
	SHORT leading;			/* distance between lines beyond image_height */
	SHORT default_leading;  /* recommended line spacing beyond image_height */
	SHORT line_spacing;		/* current distance between lines */
	SHORT spacing;			/* Extra space between characters */
	SHORT tab_width;		/* pixel width of default tab */
	SHORT image_height;		/* height from tallest cap to lowest descender */
	SHORT end_space;		/* width of space at end of line */
	SHORT left_overlap;		/* Amount letter might overlap previous. */
	SHORT right_overlap;	/* Amount letter might overlap to the right */

	EFUNC change_unzag;		/* Change unzag. */
	SHORT pad[3];
	} Vfont;

struct font_hdr;
struct names;
struct wscreen;

/* defines for font flags. */
#define VFF_SCALEABLE 	0x0001
#define VFF_MONOSPACE 	0x0002 /* all chars and spaces the same width */
#define VFF_XINTERLEAVE 0x0004 /* characters in this font overlap in spacing */

extern char sixhi_font_name[];
extern Vfont *uvfont;

extern Errcode
font_req(char *font_path, char *wildcard, SHORT *ptop_name, SHORT *point_size,
		Vfont *pfont, struct wscreen *screen, SHORT *punzag_flag);

extern void init_sail_vfont(Vfont *vf);
extern void init_sixhi_vfont(Vfont *vf);
extern void init_st_vfont(Vfont *vfont, struct font_hdr *stf);

Errcode load_font(char *title, Vfont *font, SHORT height, SHORT unzag_flag);
Errcode fset_spacing(Vfont *f, SHORT spacing, SHORT leading);
Errcode fget_spacing(Vfont *f, SHORT *spacing, SHORT *leading);
Errcode fset_height(Vfont *f, SHORT height);
Errcode fset_unzag(Vfont *f, Boolean unzag);

extern void scan_init_vfont(Vfont *f);

int fspace_width(Vfont *f,char *s); /* special case of fchar_spacing()
									 * specificly for space characters
									 * and tabs.  A little faster in cases */

int fchar_spacing(Vfont *f,char *s); /* width of char while imbedded in text
									  * pays attention to what's after it 
									  * factors in spacing setting in font */

int fchar_width(Vfont *f,char *s); /* width of char while imbedded in text
									* pays attention to what's after it 
									* width without paying attention to 
									* font spacing setting. the minimum size
									* for imbedded text */

int fendchar_width(Vfont *v, char *s); /* width of char on end of line */

int vfont_interleave_extra(Vfont *f, char *s, int count);
long fnstring_width(Vfont *f,char *s,int n);
long fstring_width(Vfont *f,char *s);
int widest_char(Vfont *f);
int widest_name(Vfont *f, struct names *list);
int tallest_char(Vfont *f);
long widest_line(Vfont *f,char **lines, int tcount);
int font_cel_height(Vfont *f);
long fline_width(Vfont *f, char *s);
int font_ycent_oset(Vfont *f,SHORT height);
int font_xcent_oset(Vfont *f,char *s,SHORT width);
Boolean in_font(Vfont *f, int c);

extern void
blit_for_mode(int tmode,
		UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
		RASType *r, Coor rx, Coor ry, Ucoor w, Ucoor h,
		Pixel oncol, Pixel offcol);

Errcode gftext(RASType *rast,
			Vfont *f,
			char *s,
			int x,int y,
			Pixel color,Text_mode mode, BCOLvar);

#undef RASType
#undef BCOLvar

void close_vfont(Vfont *v);
Vfont *get_sys_font(void);

extern void free_menu_font(void);

/* what to expand tabs to */
#define TABEXP 4

extern unsigned char oem_to_ansi[];

extern void
bitmask_to_alpha_channel(Pixel *dest, int shrinker, UBYTE *bitplane,
		int w, int h, int bpr, int y_fraction);

#endif /* RASTEXT_H */
