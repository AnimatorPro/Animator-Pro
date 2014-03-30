#ifndef WNDOBODY_H
#define WNDOBODY_H

#ifndef LINKLIST_H
#include "linklist.h"
#endif

struct wscreen;

typedef struct wndobody {
	/* This is another representation of width and height for clipping
	 * note this is immediately after the RastHdr that makes a
	 * ClipRect out of RastHdr RECT_FIELDS and the wndobody !!
	 */
	SHORT xmax, ymax;

	/* Node for installation in window screen window list node.next
	 * points to window behind node.prev points to window in front.
	 */
	Dlnode node;

	/* The screen window is attached to. */
	struct wscreen *screen;
} Wndobody;

#endif
