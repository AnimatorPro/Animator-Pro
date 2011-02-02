
#ifndef REDO_H
#define REDO_H

#ifndef VERTICES_H
#include "vertices.h"
#endif /*VERTICES_H*/

#define REDO_NONE 0
#define REDO_BOX 1
#define REDO_CIRCLE 2
#define REDO_TEXT 3
#define REDO_POLY 4
#define REDO_FILL 5
#define REDO_FLOOD 6
#define REDO_EDGE 7
#define REDO_LINE 8
#define REDO_SPIRAL 9
#define REDO_MOVE 10
#define REDO_DRAW 11
#define REDO_GEL 12
#define REDO_SPRAY 13
#define REDO_SEP 14

/* draw tool subtypes */
#define DT_STREAK 0
#define DT_DRAW 1
#define DT_DRIZZLE 2
#define DT_GEL 3
#define DT_SPRAY 4



/* Placeholder for REDO_NONE */
typedef struct none_p
	{
	UBYTE dummy;
	} None_p;

typedef struct circle_p
	{
	Short_xy center;
	SHORT diam;
	} Circle_p;

typedef struct poly_p
	{
	char curve;
	} Poly_p;

typedef struct move_p
	{
	Rectangle orig;
	Short_xy new;
	Boolean clear_move_out;
	} Move_p;

typedef struct sep_p
	{
	Rectangle rect;
	short ccount;
	UBYTE *ctable;
	} Sep_p;

/* This one is just a placeholder to alloc biggest redo structure. */
typedef struct big_p
	{
	UBYTE big[16];
	} Big_p;

typedef union redo_all_p
	{
	None_p none_p;
	Rectangle rect_p;
	Circle_p circle_p;
	Poly_p poly_p;
	Move_p move_p;
	Short_xy line_p[2];
	Short_xy fill_p;
	Short_xy flood_p[2];
	Short_xy edge_p;
	Sep_p sep_p;
	Big_p big_p;
	SHORT draw_p;
	} Redo_all_p;

struct _redo_err_check_ {
	char xx[sizeof(Redo_all_p) == sizeof(Big_p)];
};

typedef struct redo_rec
	{
	SHORT type;
	Redo_all_p p;
	} Redo_rec;

/* Individual element of save file for redo_draw */
typedef struct pos_p
	{
	Coor x, y;
	UBYTE pressure;
	} Pos_p;

/* item to save for each spray blob mass */

typedef struct spray_redo {
	SHORT count; /* count of blobs for previous point */
} Spray_redo;

Boolean get_spray_redo(Spray_redo *sr);

#endif /* REDO_H */

