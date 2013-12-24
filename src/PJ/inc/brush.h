#ifndef BRUSH_H
#define BRUSH_H

#ifndef RASTCALL_H
	#include "rastcall.h"
#endif

#ifndef VERTICES_H
	#include "vertices.h"
#endif

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
		struct circleb_data cd;
		struct squareb_data sd;
		struct lineb_data ld;
	} d;
} Rbrush;

#define bcirc d.cd 
#define bsquare d.sd 
#define bline d.ld 

#endif /* BRUSH_H */
