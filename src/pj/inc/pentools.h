#ifndef PENTOOLS_H
#define PENTOOLS_H

#ifndef OPTIONS_H
#include "options.h"
#endif

struct button;
struct cursorhdr;
struct pos_p;
struct rcel;
struct sep_p;
struct wndo;

enum pentool_ids {
	NULL_PTOOL = -1,  /* we don't want any other tool to have this id */
	TEXT_PTOOL = 0,
	STREAK_PTOOL,
	STARF_PTOOL,
	SPRAY_PTOOL,
	CURVE_PTOOL,
	SPIRAL_PTOOL,
	SHAPEF_PTOOL,
	SEP_PTOOL,
	RPOLYF_PTOOL,
	POLYF_PTOOL,
	PETLF_PTOOL,
	OVALF_PTOOL,
	MOVE_PTOOL,
	LINE_PTOOL,
	GEL_PTOOL,
	FLOOD_PTOOL,
	FILL_PTOOL,
	EDGE_PTOOL,
	DRIZ_PTOOL,
	DRAW_PTOOL,
	CIRCLE_PTOOL,
	BOX_PTOOL,
	TWEEN_PTOOL,		/* unique to tween subsystem */
	COPY_PTOOL,
	TEST_PTOOL = 100,
};

typedef struct pentool {
	Option_tool ot;
	Errcode (*doptool)(struct pentool *pt, struct wndo *w);
	struct cursorhdr *cursor;   /* cursor for tool */
	Errcode (*on_install)(struct pentool *pt);
	void (*on_remove)(struct pentool *pt);
} Pentool;

#define PTOOLINIT0(nx,na,t,id,hlp,opt,cur,tl,cl) \
	{{nx,na,t,id,hlp,opt,cl},tl,cur,NULL,NULL}

#define PTOOLINIT1(nx,na,t,id,hlp,opt,cl,tl,cur,oi,or) \
	{{nx,na,t,id,hlp,opt,cl},tl,cur,oi,or}

extern Pentool null_pentool;
extern Pentool tween_pen_tool;
extern Pentool sep_ptool_opt;
extern struct button pen_opts_sel;
extern struct button box_group_sel;
extern struct button curve_group_sel;
extern struct button fill2c_group_sel;
extern struct button freepoly_group_sel;
extern struct button move_group_sel;
extern struct button om_osped_group_sel;
extern struct button om_points_group_sel;
extern struct button om_sratio_group_sel;
extern struct button sep_group_sel;
extern struct button text_group_sel;

extern Errcode box_tool(Pentool *pt, struct wndo *w);
extern Errcode circle_tool(Pentool *pt, struct wndo *w);
extern Errcode copy_tool(Pentool *pt, struct wndo *w);
extern Errcode draw_tool(Pentool *pt, struct wndo *w);
extern Errcode drizl_tool(Pentool *pt, struct wndo *w);
extern Errcode edge_tool(Pentool *pt, struct wndo *w);
extern Errcode fill_tool(Pentool *pt, struct wndo *w);
extern Errcode flood_tool(Pentool *pt, struct wndo *w);
extern Errcode gel_tool(Pentool *pt, struct wndo *w);
extern Errcode line_tool(Pentool *pt, struct wndo *w);
extern Errcode move_tool(Pentool *pt, struct wndo *w);
extern Errcode sep_tool(Pentool *pt, struct wndo *w);
extern Errcode spray_tool(Pentool *pt, struct wndo *w);
extern Errcode streak_tool(Pentool *pt, struct wndo *w);
extern Errcode text_tool(Pentool *pt, struct wndo *w);

/* pencel.c */
extern Errcode alloc_pencel(struct rcel **pcel);
extern void swap_pencels(struct rcel *s, struct rcel *d);
extern struct rcel *clone_pencel(struct rcel *s);

/* penopts.c */
extern Errcode init_ptools(void);
extern void cleanup_ptools(void);
extern void attatch_tools(void);

/* pentools.c */
extern Boolean tti_input(void);
extern Boolean pti_input(void);
extern int do_pen_tool(void *w);
extern void do_pentool_once(Pentool *ptool);
extern void end_line_undo(void);
extern Errcode start_line_undo(void);
extern void save_line_undo(Coor y);
extern void save_lines_undo(Ucoor start, int count);

extern Errcode dtool_input(struct pos_p *p, void *dummy, SHORT mode);

extern Errcode
dtool_loop(
		Errcode (*get_posp)(struct pos_p *, void *, SHORT), void *idata,
		SHORT mode);

extern Errcode
spray_loop(
		Errcode (*get_posp)(struct pos_p *, void *, SHORT), void *idata,
		Boolean redoing);

/* flood.c */
extern Errcode fill(USHORT x, USHORT y);
extern Errcode flood(USHORT x, USHORT y, Pixel endcolor);

/* gel.c */
extern Errcode
gel_tool_loop(
		Errcode (*get_posp)(struct pos_p *, void *, SHORT), void *idata);

extern void see_gel_brush(struct button *b);

/* hitext.c */
extern void qplace_titles(void);
extern void qpwtitles(int paste);
extern void qedit_titles(void);
extern void ttool(int paste);
extern Errcode load_and_paste_text(char *name);

/* sep.c */
extern int in_cnums(UBYTE color, UBYTE *table, int count);
extern Errcode do_sep_redo(struct sep_p *sep);
extern void separate(void);
extern int edge1(Pixel ecolor);

/* penwndo.c */
extern void free_undof(void);
extern void movefli_tool(void);
extern Errcode move_penwndo(void);
extern Errcode set_penwndo_size(SHORT width, SHORT height);
extern void set_penwndo_position(void);

/* vfeelme.c */
extern void toggle_pen(struct button *m);

#endif
