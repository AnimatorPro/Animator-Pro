#ifndef BROADCAS_H
#define BROADCAS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef LINKLIST_H
	#include "linklist.h"
#endif

/* stuff for refreshing menus on user palette changes */

typedef struct redraw_node {
	Dlnode node;
	void (*doit)(void *data,USHORT why);
	void *data;
	USHORT why;
} Redraw_node;

void add_color_redraw(Redraw_node *rn);
void rem_color_redraw(Redraw_node *rn);
void do_color_redraw(USHORT why);
void set_color_redraw(USHORT why);
void add_rmode_redraw(Redraw_node *rn);
void rem_rmode_redraw(Redraw_node *rn);
void do_rmode_redraw(USHORT why);

/* "why" flags for color redraws */

#define NEW_CCOLOR 0x0001
#define NEW_INK0   0x0002
#define NEW_CMAP   0x0004
#define NEW_INK1   0x0008
#define NEW_CCYCLE 0x0010
#define NEW_CEL_TCOLOR 0x0020
#define NEW_MINIPAL_INK 0x0040


/* flags for do_rmode_redraw() and rmode nodes */

#define RSTAT_ZCLEAR	0x0001
#define RSTAT_UNDER		0x0002
#define RSTAT_ONECOL	0x0004
#define RSTAT_MASK		0x0008
#define RSTAT_CFIT		0x0010

#endif /* BROADCAS_H */
