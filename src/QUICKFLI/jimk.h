#ifndef JIMK_H
#define JIMK_H

#include "jimk0.h"

#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */



extern void *malloc();
#define askmem(size) malloc((unsigned)size)
#define freemem(p) free(p)

extern void *begmem(unsigned size);

#define XMAX 320
#define BPR 320
#define YMAX 200
#define WIDTH 320
#define HEIGHT 200
#define DEPTH 8
#define COLORS 256
#define SCREEN_SIZE (BPR*HEIGHT)
#define VGA_SCREEN ((void *)0xa0000000)

struct video_form
	{
	WORD x, y;	/* upper left corner in screen coordinates */
	UWORD w, h;	/* width, height */
	UWORD bpr;	/* bytes per row of image p */
	UBYTE *p;
	UBYTE *cmap;
	};
typedef struct video_form Video_form;
extern struct video_form vf;	/* structure for VGA screen */

#endif
