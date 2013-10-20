#ifndef JIMK_H
#define JIMK_H

#include "debug.h"

#define GCC_PACKED

#if defined(__GNUC__)
#undef GCC_PACKED
#define GCC_PACKED  __attribute__((packed))
#endif

typedef char BYTE;
typedef unsigned char UBYTE;
typedef short WORD;
typedef unsigned short UWORD;

#if defined(__TURBOC__)
typedef long LONG;
typedef unsigned long ULONG;
#else
typedef int LONG;
typedef unsigned int ULONG;
#endif

STATIC_ASSERT(jimk, sizeof( BYTE) == 1);
STATIC_ASSERT(jimk, sizeof(UBYTE) == 1);
STATIC_ASSERT(jimk, sizeof( WORD) == 2);
STATIC_ASSERT(jimk, sizeof(UWORD) == 2);
STATIC_ASSERT(jimk, sizeof( LONG) == 4);
STATIC_ASSERT(jimk, sizeof(ULONG) == 4);

struct byte_regs 
	{
	unsigned char al, ah, bl, bh, cl, ch, dl, dh;
	UWORD si, di, ds, es;
	};
struct word_regs
	{
	UWORD ax, bx, cx, dx;
	UWORD si, di, ds, es;
	};
union regs
	{
	struct byte_regs b;
	struct word_regs w;
	};

#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */



extern void *malloc();
#define askmem(size) malloc((unsigned)size)
#define freemem(p) free(p)

extern void *begmem(unsigned size);
extern long get80hz();

/* A couple of routines to deal with data spaces potentially greater than
   64K without resorting to a HUGE model.  In the 320x200 case where
   a single screen fits inside 64K these may not be necessary. */
extern long pt_to_long(), make_long();
extern void *long_to_pt();

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
