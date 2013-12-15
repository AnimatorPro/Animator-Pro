#ifndef TWEEN_H
#define TWEEN_H

#ifndef LINKLIST_H
	#include "linklist.h"
#endif
#ifndef POLY_H
	#include "poly.h"
#endif

void tween_menu();
Boolean in_tween_menu();

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

void init_tween_state(Tween_state *s);
void trash_tween_state(Tween_state *state);
Boolean tween_has_data(Tween_state *ts);
Errcode tween_add_a_link(Tween_state *ts, int startix, int endix
, Boolean closed, Tween_link **pnewl);
int tween_cmp_link(Tween_link *a, Tween_link *b);
void tween_state_swap_ends(Tween_state *ts);

Errcode load_tween(char *name, Tween_state *ts);
Errcode save_tween(char *name, Tween_state *ts);
void a_wireframe_tween(Tween_state *tween,
	int frames, int speed, 
	Pixel dit_color, Pixel dash_color, Boolean closed,
	int play_mode);
void render_a_tween(Tween_state *ts);
Errcode tween_trail_frame(Tween_state *ts, int steps);


void sample_vertex();

Errcode calc_path_pos();


/******** Stuff to pass to do_auto() for rendering a tween. */
typedef struct tween1_data
	{
	Tween_state *ts;
	Boolean is_spline;
	} Tween1_data;
Errcode tween1(Tween1_data *twd, int ix, int  intween, int  scale);

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
void init_tw_list(Tw_tlist *twl);
void trash_tw_list(Tw_tlist *twl);

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

Errcode ts_to_tw_list();
void trash_tw_list();
void calc_tween_points(Tw_tlist *tl, Boolean closed, int scale, 
	Short_xyz **ppts, int *pcount);

#endif /* TWEEN_H */
