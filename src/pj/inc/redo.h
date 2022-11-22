
#ifndef REDO_H
#define REDO_H

#ifndef VERTICES_H
#include "vertices.h"
#endif /*VERTICES_H*/

struct button;

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

typedef union redo_all_p
	{
	UBYTE none_p; /* Placeholder for REDO_NONE. */
	Rectangle rect_p;
	Circle_p circle_p;
	Poly_p poly_p;
	Move_p move_p;
	Short_xy line_p[2];
	Short_xy fill_p;
	Short_xy flood_p[2];
	Short_xy edge_p;
	Sep_p sep_p;
	SHORT draw_p;
	} Redo_all_p;

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

/* redo.c */
extern Errcode start_save_redo_points(void);
extern void end_save_redo_points(void);
extern Errcode save_redo_point(Pos_p *p);
extern Errcode save_spray_redo(Spray_redo *sr);
extern Boolean get_spray_redo(Spray_redo *sr);
extern Errcode save_redo_draw(int mode);
extern Errcode save_redo_gel(void);
extern Errcode save_redo_spray(void);
extern Errcode save_redo_sep(Sep_p *sep);
extern void do_auto_redo(Boolean edit);
extern Errcode save_redo_box(Rectangle *r);
extern Errcode save_redo_circle(Circle_p *cp);
extern Errcode save_redo_text(void);
extern Errcode save_redo_poly(char curve);
extern Errcode save_redo_fill(Short_xy *p);
extern Errcode save_redo_flood(Short_xy p[2]);
extern Errcode save_redo_edge(Short_xy *p);
extern Errcode save_redo_line(Short_xy *xys);
extern Errcode save_redo_spiral(void);
extern Errcode save_redo_move(Move_p *m);
extern void clear_redo(void);

/* quickdat.c */
extern void see_undo(struct button *b);
extern void see_redo(struct button *b);

/* vpsubs.c */
extern void undo_dot(SHORT x, SHORT y, void *data);
extern void undo_rect(Coor x, Coor y, Coor w, Coor h);
extern void save_undo_rect(Coor x, Coor y, Coor w, Coor h);
extern void zoom_undo_rect(Coor x, Coor y, Coor w, Coor h);
extern void save_undo(void);
extern void zoom_unundo(void);
extern void swap_undo(void);
extern void menu_doundo(void);
extern void menu_doredo(void);

#endif
