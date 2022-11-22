#ifndef RECTANG_H
#define RECTANG_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef REXLIB_CODE

typedef struct short_minmax {
	SHORT min, max;
} Short_minmax;

/* clipping rectangle note this is in same order as Rectangle 
 * maxX is one beyond last pixel MaxY likewise should never be a different
 * data type size than a Rectangle */

typedef struct cliprect {
	SHORT x, y;
	SHORT MaxX, MaxY;
} Cliprect;

/* REXLIB_CODE */ #endif

/* Rectangle or Box used for all kinds of boxy things icons blits etc 
 * change this only under penalty of severe crash.  There are many instances
 * of (Rectangle *)&(struc->width) around. 
 *
 *	The order width,height,x,y is important in that if it is followed by
 *  MaxX and MaxY it becomes FullRect */


#define RECT_FIELDS \
	USHORT width, height;\
	SHORT x, y

typedef struct rectangle {
	RECT_FIELDS;
} Rectangle;

typedef struct srect {    /* signed rectangle */
	SHORT width, height;
	SHORT x, y;
} Srect;

#ifndef REXLIB_CODE

#define FRECT_FIELDS \
	USHORT width, height;\
	SHORT x, y;\
	SHORT MaxX, MaxY

#define FRECTSTART width

typedef struct fullrect {  /* really just a Cliprect merged with a Rectangle */
	FRECT_FIELDS;
} Fullrect;

#define RECTSTART width
#define CRECTSTART x 

#define copy_rectfields(src,dst) {*((Rectangle *)&((dst)->RECTSTART))\
 =*((Rectangle *)&((src)->RECTSTART));}

#define copy_crectfields(src,dst) {*((Cliprect *)&((dst)->CRECTSTART))\
 =*((Cliprect *)&((src)->CRECTSTART));}

/* rectangle and point clip things */

Boolean ptin_rect(Rectangle *r,SHORT x,SHORT y);
Boolean ptin_crect(Cliprect *r, SHORT x,SHORT y);
Boolean ptinside_rect(Rectangle *b, SHORT x, SHORT y, SHORT inside);

Boolean crects_overlap(Cliprect *a,Cliprect *b);
Boolean and_cliprects(Cliprect *a,Cliprect *b,Cliprect *out);
Boolean and_rects(Rectangle *a,Rectangle *b,Rectangle *out);
void swap_clip(Cliprect *clip);

void sclip_rect(Rectangle *r, Rectangle *b);
void bclip_dim(SHORT *pos,USHORT *len, SHORT minpos, SHORT maxlen);
void bclip_rect(Rectangle *r,Rectangle *b);
void bclip0xy_rect(Rectangle *r,Rectangle *b);

/* rectangle converters */

void rect_tocrect(Rectangle *r,Cliprect *cr);
void rect_tofrect(Rectangle *r, Fullrect *fr);

void crect_torect(Cliprect *cr, Rectangle *r);
void crect_tofrect(Cliprect *cr,Fullrect *fr);

void frame_tocrect(SHORT x0, SHORT y0, SHORT x1, SHORT y1, Cliprect *cr);
void frame_torect(SHORT x0, SHORT y0, SHORT x1, SHORT y1, Rectangle *r);
void frame_tofrect(SHORT x0, SHORT y0, SHORT x1, SHORT y1, Fullrect *fr);

extern int quad9(SHORT px, SHORT py, SHORT x, SHORT y, SHORT w, SHORT h);

/* clipcode returns the codes below or 0 */
int clipcode_crects(Cliprect *a,Cliprect *b);
#define CODELEFT 0x01
#define CODERIGHT 0x02
#define CODETOP 0x04
#define CODEBOTTOM 0x08
#define CODEALL 0x0f
#define CODEOBSCURES 0x10

/* REXLIB_CODE */ #endif

#endif /* RECTANG_H */
