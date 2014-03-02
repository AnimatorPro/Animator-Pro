#ifndef RASTCURS_H
#define RASTCURS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef MENUS_H
	#include "menus.h"
#endif

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

typedef struct curslist {
	Rastcursor *curs;
	char *name;
} Curslist;

/* cursor show hide and move functions for raster cursors */

void set_cursor_ccolor(Pixel *pccolor);
void show_rastcursor(Rastcursor *rc);
void hide_rastcursor(Cursorhdr *rc);
void move_rastcursor(Cursorhdr *rc);
Tcolxldat *get_cursor_xlat(void);
Rastcursor *get_default_cursor(void);

#endif /* RASTCURS_H Leave at end of file */
