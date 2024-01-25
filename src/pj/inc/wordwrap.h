#ifndef WORDWRAP_H
#define WORDWRAP_H

#ifndef RASTEXT_H
	#include "rastext.h"
#endif

/* justification types */

#define JUST_LEFT	0
#define JUST_RIGHT	1
#define JUST_CENTER	2
#define JUST_FILL	3

#ifdef WORDWRAP_INTERNALS
	#define RASType Raster
#else
	#define RASType void
#endif

extern int
just_charstart(Vfont *font, int x, int lwidth,
		char *linebuf, int charpos, int just);

int justify_line(RASType *r, Vfont *font, char *linebuf, int x,int y,int w,
				 Pixel color, Text_mode tmode, Pixel color2,int just, SHORT *cposx,
				 int cpos_idx);

void wwtext(RASType *screen, Vfont *f, char *s,
			int x,int y,int w,int h,
			int skiplines, int justify,
			Pixel color,Text_mode tmode,Pixel color2);

#undef RASType

int wwnext_line(Vfont *f, char **ps, const int w, char *buf, int sub_cr);

int wwcount_lines(Vfont *f,char *s,int w, SHORT *maxwid);

#ifdef WORDWRAP_INTERNALS

enum ww_why {
	WW_FILLED = 1,  /* Line completely filled, goto next */
	WW_HARDCR,		/* Hard return */
	WW_EOTEXT = 10, /* End of text */
	WW_EOWORD,      /* End of word and/or trailing spaces */
	WW_LONGWORD,	/* WRAP, a word exceeded width */
};

typedef struct wwdata {
	Vfont *font;
    char *lstart;	/* next char in input text stream */
	char *out;		/* buffer to put results in */
	int wid; 		/* wrap window width */
	int tab_width;  /* size of a tab in pixels */
	int sub_cr;		/* what to output carraige returns with in output lines */

	/* returned values */

	int line_left;  /* Ammount of line left */
	int why;        /* why function returned */

	/* internal constant initialized items */

	int max_wrap_test;

	/* internal items to wordrap_line() */

	int wrap_test; 	/* Precalc test variable for word wrap test */
	int ww;			/* Width of imbedded word */
	int endw; 		/* Width to last pixel of word (word on end of line) */
} Wwdata;

#endif

#endif /* WORDWRAP_H */
