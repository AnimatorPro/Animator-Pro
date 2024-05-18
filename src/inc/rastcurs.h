#ifndef RASTCURS_H
#define RASTCURS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef INPUT_H
	#include "input.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif

struct rcel;
struct short_xy;
struct tcolxldat;

#define DFLT_CURS_HT 19
#define DFLT_CURS_WID 19

#define CURS_MC0	251   /* first cursor color */
#define CURS_CCOLOR	250   /* cursor "ccolor" */

typedef struct cursorsave {
	Bytemap r;
	SHORT w,h; /* width and height of saved area */
} Cursorsave;

typedef struct cursorcel {
	RAST_FIELDS;
} Cursorcel;

typedef struct rastcursor {
	Cursorhdr hdr;
	Cursorcel *cel;     /* the image cel */
	Cursorsave *save;	/* save area set by rastcursor code not present in
						 * default cursor */
} Rastcursor;

extern Cursorhdr pentool_cursor;
extern Cursorhdr zoom_pencel_cursor;

extern Rastcursor box_cursor;
extern Rastcursor edge_cursor;
extern Rastcursor fill_cursor;
extern Rastcursor hand_cursor;
extern Rastcursor menu_cursor;
extern Rastcursor move_tool_cursor;
extern Rastcursor pen_cursor;
extern Rastcursor pick_cursor;
extern Rastcursor plain_ptool_cursor;
extern Rastcursor sep_cursor;
extern Rastcursor shape_cursor;
extern Rastcursor spray_cursor;
extern Rastcursor star_cursor;
extern Rastcursor text_cursor;

/* cursor.c */
extern Cursorhdr *set_pen_cursor(Cursorhdr *ch);
extern Errcode init_cursors(void);
extern void cleanup_cursors(void);
extern Errcode save_cursor(char *title, struct rcel *rc, struct short_xy *hot);

/* pjhigh/cursxlat.c */
extern void set_cursor_ccolor(Pixel *pccolor);
extern struct tcolxldat *get_cursor_xlat(void);

/* pjhigh/rastcurs.c */
extern void show_rastcursor(Cursorhdr *rc);
extern void hide_rastcursor(Cursorhdr *rc);
extern void move_rastcursor(Cursorhdr *rc);

/* pjhigh/defltcur.c */
extern Rastcursor *get_default_cursor(void);

#endif
