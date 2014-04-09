
typedef int WORD;
typedef unsigned UWORD;
typedef unsigned char UBYTE;
typedef char BYTE;
typedef unsigned char *PLANEPTR;

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


struct name_list
	{
	struct name_list *next;
	char *name;
	};
typedef struct name_list Names, Name_list;

extern void *malloc();
#define askmem(size) malloc((unsigned)size)
#define freemem(p) free(p)

extern void *make_ptr();
extern void *norm_pointer(), *enorm_pointer();
extern void *begmem(unsigned size);
extern char *clone_string();
extern long get80hz();
extern long pt_to_long(), make_long();
extern void *long_to_pt();

extern long jreadwrite(int f, void *buf, long size, int ah);
#define jread(f,b,size) jreadwrite(f,b,(long)(size),0x3f)
#define jwrite(f,b,size) jreadwrite(f,b,(long)(size),0x40)

extern jcreate(char *title), jopen(char *title, int mode);
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
