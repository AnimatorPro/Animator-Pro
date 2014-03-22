#ifndef CELMENU_H
#define CELMENU_H

#ifndef FLICEL_H
#include "flicel.h"
#endif

#ifndef INPUT_H
#include "input.h"
#endif

#ifndef MENUS_H
#include "menus.h"
#endif

struct flicel;
struct pentool;
struct raster;
struct rcel;

/* local stuff for cel menu */
typedef struct celmu_cb {
	struct raster *crast1;  /* backup raster for paint tool */
	struct raster *crast2;  /* backup raster for paint tool */
	LONG olay_buf_size;		/* buffer size for making overlay */
	LONG finish_buf_size;	/* buffer size for finishing overlay */
	SHORT undo_corrupted;   /* undo buffer is corrupted */
	SHORT num_overlays;     /* number of overlays installed */
	LONG olaymem;			/* total memory consumed by overlays */
	Fcelpos lastpos;        /* last overlay position or last position
							 * used for undo in position tools */
	SHORT no_draw_tools;    /* menu was called not allowing draw tools */
	SHORT dummy_olays;		/* overlays have been freed and there is only
							 * a path and flags left */
	SHORT outta_mem;		/* paint tool has run out! */
	struct rcel *paste_undo;/* the cel paste operation undo buffer */
	Menuhdr tpull;			/* The pulldown structures */
	void *marqi_save;		/* buffer with saved marqi background */
	void *marqi_save_buf;	/* allocated buffer for saving */
	int marqmod;			/* marqi mod for creepy crawlys */
	Waitask mwt;			/* marqi creep task */
	ULONG marqitime;        /* time of last marqi */
} Celmu_cb;

enum cel_ptool_ids {
	CELPT_MOVE,
	CELPT_ROTATE,
	CELPT_SCALE,
	CELPT_PASTE,
	CELPT_PAINT,
	CELPT_TCOLOR,
};

extern Menuhdr cel_menu;
extern Celmu_cb *cmcb;
extern Button cmu_common_sels;
extern Button cmu_pa_group_sel;

/* celmenu.c */
extern void cmu_marqi_cel(void);
extern void cmu_unmarqi_cel(void);
extern Errcode init_marqi_ctool(struct pentool *pt);
extern void exit_marqi_ctool(struct pentool *pt);
extern void cm_erase_toolcel(void);
extern void cm_restore_toolcel(void);
extern Errcode reset_celmenu(int toolid, Boolean startup);
extern void enable_toolcel_redraw(void);
extern void disable_toolcel_redraw(void);
extern void go_cel_menu(void);
extern void go_nodraw_cel_menu(void);

/* celpaste.c */
extern void exit_paint_ctool(struct pentool *pt);
extern Errcode init_paint_ctool(struct pentool *pt);
extern Errcode cel_paint_ptfunc(struct pentool *pt, Wndo *w);

extern void cmu_free_paste_undo(void);
extern void exit_paste_ctool(struct pentool *pt);
extern Errcode init_paste_ctool(struct pentool *pt);
extern Errcode cel_paste_ptfunc(struct pentool *pt, Wndo *w);

/* celpull.c */
extern Boolean do_celpull(Menuhdr *mh);
extern void cm_selit(Menuhdr *mh, SHORT hitid);
extern void save_celpos_undo(void);
extern void cel_cancel_undo(void);

/* celtrans.c */
extern Boolean isin_fcel(struct flicel *fcel, SHORT x, SHORT y);
extern void vstretch_cel(int toolmode);
extern void vrotate_cel(int toolmode);
extern Errcode inc_thecel(void);
extern Errcode mp_thecel(int paste, int toolmode);
extern void move_the_cel(void);
extern void paste_the_cel(void);

/* vpaint.c */
extern void toggle_cel_opt(int mode);

#endif
