#ifndef TWEEN_H
#define TWEEN_H

#ifndef LINKLIST_H
	#include "linklist.h"
#endif
#ifndef POLY_H
	#include "poly.h"
#endif

struct button;
struct menuhdr;

#define TWEEN_ONCE 0
#define TWEEN_LOOP 1

typedef struct tween_file_header
	{
	USHORT magic;			/* == TWEEN_MAGIC */
	USHORT version;
	long tcount;			/* == 2 for now */
	char reserved[8];
	long link_count;
	} Tween_file_header;

typedef struct tween_link
	{
	Dlnode node;
	long start,end;
	} Tween_link;

typedef struct tween_state
	{
	Poly p0, p1;
	Dlheader links;
	} Tween_state;

/******** Stuff to pass to do_auto() for rendering a tween. */
typedef struct tween1_data
	{
	Tween_state *ts;
	Boolean is_spline;
	} Tween1_data;

/******** structures internal to tween system **********/
typedef struct tw_tlist
	{
	Dlheader list;
	Short_xyz *spts;
	int scount;
	Short_xyz *dpts;
	int dcount;
	Short_xyz *ipts;
	int icount;
	} Tw_tlist;

typedef struct tw_thread
	{
	Dlnode node;
	Short_xyz *source;
	int scount;
	Short_xyz *dest;
	int dcount;
	Short_xyz *inter;
	int icount;
	int dinc;		/* == 1 or -1 */
	} Tw_thread;

extern struct menuhdr twe_menu;

/* tween.c */
extern Boolean got_tween(void);
extern void twe_go_tool(struct button *b);
extern void tween_menu(Boolean renderable);

/* tweendat.c */
extern Errcode load_tween_panel_strings(void **ss);

/* tweenhi.c */
extern void
a_wireframe_tween(Tween_state *tween, int frames, int speed,
		Pixel dit_color, Pixel dash_color, Boolean closed, int play_mode);

extern Errcode save_tween(char *name, Tween_state *ts);
extern Errcode load_tween(char *name, Tween_state *ts);
extern Errcode test_load_tween(char *name);
extern void render_a_tween(Tween_state *ts);
extern Errcode tween_trail_frame(Tween_state *ts, int steps);

/* tweenlo.c */
extern void init_tween_state(Tween_state *ts);
extern void trash_tween_state(Tween_state *ts);
extern Boolean tween_has_data(Tween_state *ts);
extern void tween_state_swap_ends(Tween_state *ts);
extern int tween_cmp_link(Tween_link *a, Tween_link *b);

extern Errcode
tween_add_a_link(Tween_state *ts, int startix, int endix, Boolean closed,
		Tween_link **pnewl);

extern Errcode
calc_path_pos(Poly *poly, Short_xyz *delta_array, int scale, Boolean closed);

extern void trash_tw_list(Tw_tlist *twl);
extern Errcode ts_to_tw_list(Tween_state *vin, Boolean closed, Tw_tlist *tout);

extern void
calc_tween_points(Tw_tlist *tl, Boolean closed, int scale,
		Short_xyz **ppts, int *pcount);

#endif
