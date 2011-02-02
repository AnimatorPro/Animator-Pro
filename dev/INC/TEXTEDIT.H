#ifndef TEXTEDIT_H
#define TEXTEDIT_H 

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RECTANG_H
	#include "rectang.h"
#endif

#ifndef WORDWRAP_H
	#include "wordwrap.h"
#endif

#ifndef RASTEXT_H
	#include "rastext.h"
#endif

#ifndef  MENUS_H
	#include "menus.h"
#endif


typedef struct linedata {
	char *cstart;       /* start of line last time 'round */
	ULONG crcsum;     /* crcsum on wordwrap line */
	SHORT xstart;       /* x offset into twin of first char */
	SHORT width;        /* width of line in pixels */
} Linedata;

typedef struct text_file
	{
	/* persistant stuff (should be saved between sessions) */
	char *text_buf;
	long text_size;
	long text_alloc;
	char *text_name;
	Rectangle twin;
	long block_start;
	long block_end;
	SHORT is_movable;	/* can we move this window? */
	long tcursor_p;
	SHORT text_yoff;
	SHORT justify_mode;
	Pixel ccolor;
	Pixel ucolor;
	Vfont *font;
	Raster *raster;
	void (*undraw_rect)(Raster *r, void *data, 
		int x, int y, int width, int height);
	void (*undraw_dot)(int x, int y, Raster *r);
	void *undraw_data;
	/* scratch stuff used only during a text edit */
	SHORT text_cursor_color;
	SHORT text_lines_visible;
	SHORT line_height;  /* distance between lines */
	SHORT pixelx, pixely, pixelw;	/* text cursor pixel location */
	char *wstart;	/* upper left character in window */
	char *lwstart;	/* line above top of window... */
	char *lstart;	/* start of line of text cursor is in */
	char *llstart;	/* start of line previous to text cursor */
	char *nlstart;	/* start of line after text cursor */
	SHORT twypos;		/* line of text window cursor is in */
	SHORT twxpos;		/* character in line cursor is in */
	Menuhdr *pull;
	void (*pull_sel)(struct text_file *gf, int menu_ix, int sel_ix);
	Linedata *ldat;      /* line data buffer */
	UBYTE overwrite;	/* overwrite mode */
	UBYTE cursor_up;    /* cursor is present */
	SHORT cursor_hgt;   /* height of cursor */
	SHORT tw_maxx;      /* one beyond end of window */
	SHORT prev_xpos;	/* previous x position to try for when going up or 
						 * down screen */
	void (*help_function)(struct text_file *gf);	/* Called on F1 */
	UBYTE is_changed;	/* Gets set if text altered. */
	UBYTE read_only;	/* Set me if in read-only mode. */
	UBYTE fill[18];
	} Text_file;

extern struct text_file *gtf;		/* titling/text tool editor */
extern struct text_file *ptf;		/* poco programming editor */

#define MAXTEXTSIZE 32000        /* max size for test buffer */
#define DTSIZE 16002        /* default size for text buffer */

#endif /* TEXTEDIT_H */
