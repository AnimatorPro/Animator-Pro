#ifndef TEXTEDIT_H
#define TEXTEDIT_H 

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RECTANG_H
	#include "rectang.h"
#endif

struct menuhdr;
struct raster;
struct vfont;

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
	struct vfont *font;
	struct raster *raster;
	void (*undraw_rect)(struct raster *r, void *data,
		int x, int y, int width, int height);
	dotout_func undraw_dot;
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
	struct menuhdr *pull;
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

#define DTSIZE 16002        /* default size for text buffer */

/* textedit.c */
extern void free_text_file(Text_file *gf);
extern Boolean edit_text_file(Text_file *gf);
extern Errcode get_rub_twin(Text_file *gf, Boolean cutout);

/* textfile.c */
extern Errcode load_text_file(Text_file *gf, char *name);
extern Errcode save_text_file(Text_file *gf);
extern Errcode load_titles(char *title);

#endif
