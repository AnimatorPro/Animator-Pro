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

extern Celmu_cb *cmcb;

enum cel_ptool_ids {
	CELPT_MOVE,
	CELPT_ROTATE,
	CELPT_SCALE,
	CELPT_PASTE,
	CELPT_PAINT,
	CELPT_TCOLOR,
};

extern Errcode init_marqi_ctool(struct pentool *pt);
extern void exit_marqi_ctool(struct pentool *pt);

#endif
