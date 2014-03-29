#ifndef TIMEMENU_H
#define TIMEMENU_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct button;
struct menuhdr;

extern struct menuhdr tmu_menu;
extern struct button tseg_a_sel;
extern struct button tseg_s_sel;
extern struct button tseg_f_sel;
extern struct button tseg_group_sel;
extern struct button tseg_slider_sel;
extern struct button timeslider_sel;
extern struct button minitime_sel;

/* saveseg.c */
extern void go_save_segment(void);

/* time.c */
extern void qinsert_frames(void);
extern Errcode delete_some(int x);
extern void qdelete_frames(void);
extern void qmake_frames(void);
extern void set_frame_count(int x);
extern Errcode check_max_frames(int count);
extern Errcode insert_frames(int count, int where);

/* timemenu.c */
extern void insert_a_frame(void);
extern void kill_a_frame(void);
extern void set_total_frames(void);
extern void redraw_range_buttons(void);
extern void do_time_menu(void);
extern void go_time_menu(void *data);

/* tseg.c */
extern void redraw_tseg(struct button *b);
extern void change_time_mode(struct button *m);

/* vpsubs.c */
extern void init_seq(void);
extern void kill_seq(void);

#endif
