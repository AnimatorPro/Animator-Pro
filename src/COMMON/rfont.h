#ifndef RFONT_H
#define RFONT_H

#include "gemfont.h"

struct spacing_data
{
	WORD xoff;
	WORD new_width;
};

struct rast_font
{
	WORD *font_raster;
	WORD char_count;
	WORD start_char;
	struct spacing_data *spacing_array;
	WORD norm_char_width;
	WORD char_height;
	WORD words_in_line;
	WORD lines;
};

/* Function: systext
 *
 *  Hardcoded text routine for the system font.
 */
extern void
systext();

/* Function: systext_clip */
extern void systext_clip(int width, const char *str, int x, int y, int col);

/* Function: systext_keepcase */
extern void systext_keepcase(const char *s, int x, int y, int fg, int bg);

/* Function: gftext
 *
 *  Graphics text in any font, no special effects yet at least.
 */
extern void
gftext(Video_form *screen, struct font_hdr *f, const unsigned char *s,
		int x, int y, int color, Vector tblit, int bcolor);

/* Function: fchar_width
 *
 *  Find width of the first character in string.
 */
extern int fchar_width(struct font_hdr *f, const unsigned char *s);

/* Function: fnstring_width */
extern long fnstring_width(struct font_hdr *f, const unsigned char *s, int n);

/* Function: fstring_width
 *
 *  Find width of whole string.
 */
extern long fstring_width(struct font_hdr *f, const unsigned char *s);

/* Function: widest_char */
extern int widest_char(struct font_hdr *f);

/* Function: font_cel_height
 *
 *  How far to next line of the font.
 */
extern int font_cel_height(struct font_hdr *f);

#ifndef SLUFF

#define gtext(s, x, y, col) \
	systext(s, x, y, col, a1blit)

#define stext(s, x, y, col, col1) \
	systext(s, x, y, col, a2blit, col1)

#endif /* SLUFF */

#endif
