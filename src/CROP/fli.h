#ifndef FLI_H
#define FLI_H

#include "jimk.h"

#define MAXFRAMES (4*1000)	/* Max number of frames... */

/* Normal Magic */
#define FLIH_MAGIC 0xaf11	

/* Indexed Magic */
#define FLIX_MAGIC (0x971b+4)

#define SETTINGS_MAGIC (FLIX_MAGIC^0xffff)

/* Frame Magic */
#define FLIF_MAGIC 0xf1fa

struct GCC_PACKED fli_head
	{
	LONG size;
	UWORD type;  /* = FLIH_MAGIC or FLIX_MAGIC */
	UWORD frame_count;
	UWORD width;
	UWORD height;
	UWORD bits_a_pixel;
	WORD flags;
	WORD speed;
	LONG next_head;
	LONG frames_in_table;
	WORD file;
	LONG frame1_off;
	LONG strokes;	/* how many paint strokes etc. made. */
	LONG session; /* stokes since file's been loaded. */
	char reserved[88];
	};
STATIC_ASSERT(fli, sizeof(struct fli_head) == 128);

#define FLI_FINISHED 1
#define FLI_LOOPED	2

struct fli_frame
	{
	LONG size;
	UWORD type;		/* = 0xf1fa FLIF_MAGIC */
	WORD chunks;
	char pad[8];
	};
STATIC_ASSERT(fli, sizeof(struct fli_frame) == 16);

#define FLI_COL 0
#define FLI_WRUN 1
#define FLI_WSKIP 2
#define FLI_WSKIP2 3
#define FLI_COL2 4
#define FLI_WSKIPRUN 5
#define FLI_BSKIP 6
#define FLI_BSKIPRUN 7
#define FLI_BSC 8
#define FLI_SBSC 9
#define FLI_SBSRSC 10
#define FLI_COLOR 11
#define FLI_LC	12
#define FLI_BLACK 13
#define FLI_ICOLORS 14
#define FLI_BRUN 15
#define FLI_COPY 16


struct GCC_PACKED fli_chunk
	{
	LONG size;
	WORD type;
	};
STATIC_ASSERT(fli, sizeof(struct fli_chunk) == 6);

#define EMPTY_DCOMP 8  /* sizeof of a FLI_SKIP chunk with no change */

/* This structure MUST be the same size or smaller than struct fli_frame
   for the 'add frames to sequence' routines to work. */
struct flx
	{
	LONG foff;
	LONG fsize;
	};
STATIC_ASSERT(fli, sizeof(struct flx) == 8);
typedef struct flx Flx;

extern Flx *cur_flx;
extern unsigned long maxflx;
extern struct fli_head fhead;

extern int tflx;	/* handle for temp file */
#ifndef SLUFF
extern long find_free_tflx(long size);
#endif /* SLUFF */
extern long fli_comp_frame();
extern long fli_comp1();
extern long write_tflx();

#define FLX_OFFSETS ((long)sizeof(struct fli_head) + sizeof(vs) )

#ifdef SLUFFED
struct vedit
	{
	struct video_form v;
	char *name;
	UWORD type;
	WORD file;
	UWORD frame_count;
	WORD speed;
	long frames_in_table;
	struct flx *offsets;
	char docmap;
	char pad;
	};
typedef struct vedit Vedit;
extern Vedit *alloc_vedit();

struct vtrack
	{
	struct vedit *ve;
	WORD screeny;
	};
typedef struct vtrack Vtrack;
extern Vtrack vtracks[3];
#endif /* SLUFFED */

#define VGA_MAGIC 0x0100
struct vga_header
	{
	WORD type;	/* == VGA_MAGIC */
	char unknown[10];
	WORD w,h,d;
	};

#define PIC_MAGIC 0x9119
struct pic_header
	{
	UWORD type;
	WORD w,h,x,y;
	char d;
	char compress;
	long csize;
	char reserved[16];
	};

#define PIC_UNC  0
#define PIC_BRUN 1

#endif
