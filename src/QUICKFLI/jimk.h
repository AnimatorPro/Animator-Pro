
typedef int WORD;
typedef unsigned UWORD;
typedef char BYTE;
typedef unsigned char UBYTE;

struct byte_regs 
	{
	unsigned char al, ah, bl, bh, cl, ch, dl, dh;
	unsigned int si, di, ds, es;
	};
struct word_regs
	{
	unsigned ax,bx,cx,dx;
	unsigned int si, di, ds, es;
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
extern void *norm_pointer();

extern unsigned jread(int f, void *buf, unsigned size);
extern jopen(char *title, int mode);
extern long jseek(int f, long offset, int mode);

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
