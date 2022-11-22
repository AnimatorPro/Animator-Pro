#ifndef BRUSH_H
#define BRUSH_H

#ifndef PROCBLIT_H
#include "procblit.h"
#endif

#ifndef VERTICES_H
	#include "vertices.h"
#endif

struct button;

#define BRUSH_MAX_WIDTH 32
#define BRUSH_MAX_HEIGHT 32

/* a raster brush for drawing */

struct circleb_data {
	SHORT size;
};

struct squareb_data {
	SHORT size;
};

struct lineb_data {
	SHORT size;
	SHORT angle;
	Short_xy endoff;
};

enum {
	NO_BRUSH = 0,
	CIRCLE_BRUSH = 1,
	SQUARE_BRUSH = 2,
	LINE_BRUSH = 3,
};

typedef struct rast_brush {
	Bytemap *rast; /* raster containing brush image for cursor or render 
				* x and y are used to store cursor coordinates in cursor.c */
	SHORT type;
	USHORT width, height; /* width and height of image in brush from 0,0 */
	Short_xy cent;
	Tcolxldat tcxl; /* translation for raster colors */
	Pixel xlat_ccolor; /* ccolor of last translation */
	union brush_data {
		struct circleb_data circ;
		struct squareb_data square;
		struct lineb_data line;
	} b;
} Rbrush;

extern struct button gel_brush_group;
extern struct button pen_brush_group;

/* brush.c */
extern void save_ubrush(Rbrush *rb, void *src, Coor x, Coor y);
extern void rest_ubrush(Rbrush *rb, void *dst);
extern void cleanup_brushes(void);
extern Errcode init_brushes(void);

extern void
draw_line_brush(void *rast, Short_xy *cent, Pixel color, int size, int angle);

extern Errcode set_brush_type(int type);
extern int get_brush_size(void);
extern void set_brush_size(int size);
extern void blit_brush(Rbrush *rb, void *dest, Coor x, Coor y);
extern void zoom_blit_brush(Rbrush *rb, Coor x, Coor y);
extern void save_undo_brush(SHORT y);
extern void see_pen(struct button *b);

/* gel.c */
extern void see_gel_brush(struct button *b);

/* selbrush.c */
extern Errcode nest_alloc_brush_texts(void);
extern void nest_free_brush_texts(void);
extern void set_pbrush(struct button *b);

#endif
