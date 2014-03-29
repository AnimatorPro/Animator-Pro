#ifndef VPMENUS_H
#define VPMENUS_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct autoarg;
struct button;
struct menuhdr;
struct rectangle;

extern char *box_coor_str;
extern char *rub_circle_str;
extern char *rub_line_str;
extern char *rub_rect_str;
extern struct button sh1_brush_sel;

/* mainpul.c */
extern Boolean do_mainpull(struct menuhdr *mh);
extern Errcode init_poco_pull(struct menuhdr *mh, SHORT prev_id, SHORT root_id);

/* messages.c */
extern void top_textf(char *fmt, ...);
extern void soft_top_textf(char *key, ...);
extern Errcode cant_create(Errcode err, char *name);
extern void truncated(char *filename);
extern Boolean overwrite_old(char *name);
extern Boolean really_delete(char *name);

/* multimen.c */
extern Errcode multimenu(struct autoarg *aa);
extern void disable_multi_menu(void);
extern void enable_multi_menu(void);
extern void go_multi(void);

/* muparts.c */
extern void redraw_head1_ccolor(struct button *hanger);
extern void mb_toggle_zclear(struct button *b);
extern Errcode init_menu_parts(void);
extern void cleanup_menu_parts(void);

/* quickdat.c */
extern Errcode go_quick_menu(void);

/* sizemenu.c */
extern Errcode go_format_menu(struct rectangle *outsize);

/* vpaint.c */
extern void qload(void);
extern Boolean confirm_dirty_load(void);
extern void main_selit(struct menuhdr *mh, SHORT hitid);

/* vpsubs.c */
extern void mb_quickmenu_to_bottom(struct button *b);
extern void mb_move_quickmenu(struct button *b);

#endif
