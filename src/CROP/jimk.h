

#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */

#define WORD int
#define UWORD unsigned int
#define BYTE char
#define UBYTE unsigned char
#define LONG long
#define ULONG unsigned long


extern void *list_el();
#ifndef SLUFF
extern long jreadwrite(int f, void *buf, long size, int ah);
#define jread(f,b,size) jreadwrite(f,b,(long)(size),0x3f)
#define jwrite(f,b,size) jreadwrite(f,b,(long)(size),0x40)

extern jcreate(char *title), jopen(char *title, int mode);
extern long jseek(int f, long offset, int mode);
/* seek modes */

#define SEEK_START	0
#define SEEK_REL	1
#define SEEK_END	2



#endif /* SLUFF */

extern void *make_ptr();
extern void *norm_pointer(), *enorm_pointer();
extern void *lbegmem(), *begmem(), *lbegcmem();
extern void *paskmem();
extern char *clone_string();
extern char *get_filename();
extern long get80hz();
extern void *askmem(), *laskmem();
extern long pt_to_long(), make_long();
extern void *long_to_pt();
extern unsigned mem_free, largest_frag();

extern void *sort_list();
extern void *remove_el();

#define free_string(pt) freemem(pt)
#define string_width(s) (strlen(s)*CH_WIDTH)
#define zero_structure(s, size) stuff_words(0, s, ((unsigned)(size))>>1)
#define zero_words(s,size) stuff_words(0, s, (unsigned)(size))

typedef int (*Vector)();	/* AKA pointer to a function */

typedef unsigned char *PLANEPTR;

extern long comp_size;


#define Array_els(arr) (sizeof(arr)/sizeof(arr[0]))
#define Raster_block(w, h) ((w)*(h))
#define Raster_line(w) (w)
#define Mask_line(w) (((w)+7)>>3)
#define Mask_block(w, h)	(Mask_line(w)*(h))


#define XMAX 320
#define BPR 320
#define YMAX 200
#define WIDTH 320
#define HEIGHT 200
#define DEPTH 8
#define COLORS 256
#define SCREEN_SIZE (BPR*HEIGHT)
#define VGA_SCREEN ((void *)0xa0000000)
#define CH_WIDTH 6
#define CH_HEIGHT 8

extern UBYTE lmasks[], rmasks[], bmasks[];

struct name_list
	{
	struct name_list *next;
	char *name;
	};
typedef struct name_list Name_list;
typedef struct name_list Names;
extern Name_list *name_in_list();

struct thread2_list
	{
	struct thread2_list *next;
	char *name;
	struct thread2_list *next2;
	};
typedef struct thread2_list Thread2_list;


struct BitMap 
	{
	UWORD BytesPerRow;
	UWORD Rows;
	UBYTE Flags;
	UBYTE Depth;
	UWORD pad;
	PLANEPTR Planes[8];
	};

/*  These are globals that contain the input state */
extern long itime, oitime;
extern char menu_ix, sel_ix;
extern char usemacro,defmacro;
extern WORD mouse_button, omouse_button;  /* button state and last state */
extern WORD uzx, uzy;	/* unzoomed xy mouse position */
extern WORD grid_x, grid_y;		/* xy position after zoom & grid snap */
extern WORD lastx, lasty;	/* last mouse_x, mouse_y */
extern WORD luzx, luzy; /* last different mouse position */
extern WORD mouse_moved;	/* new mouse input? */
extern WORD key_hit;			/* 1 if keyboard hit, 0 otherwise */
extern WORD key_in;				/* What the extended character code 
								   for when key_hit = 1 */
extern WORD mouse_on;	/* is cursor updated by input routines? */
extern WORD x_0,y_0,x_1,y_1;
extern int recordall;
extern char clickonly, realtimemac; /* how much input to record... */
extern PLANEPTR brushcursor;

/* magic number for configuration */
#define VCFGMAGIC 0x2891

/* size of histerisis buffer */
#define HSZ 4	


/* These are macros on the mouse buttons */
#define PDN (mouse_button & 0x1)
#define RDN (mouse_button & 0x2)
#define EDN (mouse_button & 0x3)
#define PJSTDN ( (mouse_button & 0x1) && !(omouse_button & 0x1) )
#define RJSTDN ( (mouse_button & 0x2) && !(omouse_button & 0x2) )

extern WORD firstx, firsty;

/* Arrow key definitions */
#define PAGEUP  0x4900
#define PAGEDN  0x5100
#define ENDKEY  0x4f00
#define HOMEKEY 0x4700
#define DELKEY  0x5300
#define LARROW	0x4b00
#define RARROW	0x4d00
#define UARROW	0x4800
#define DARROW	0x5000


extern int brush_ix, *brushes[];

#define absval(x) ((x) >= 0 ? (x) : -(x) )

extern PLANEPTR white_cursor, black_cursor;


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
struct video_form
	{
	WORD x, y;	/* upper left corner in screen coordinates */
	UWORD w, h;	/* width, height */
	UWORD bpr;	/* bytes per row of image p */
	PLANEPTR p;
	UBYTE *cmap;
	UWORD ix;	/* where in sequence we are */
	};
typedef struct video_form Video_form;
extern Video_form vf,uf;
extern Video_form *zoom_form, *render_form, *cursor_form, *alt_form;
extern Video_form *alloc_big_screen(),*alloc_screen(), *clone_screen();

extern PLANEPTR mask_plane;
#define MASK_SIZE (Mask_line(XMAX)*YMAX)

struct vcel
	{
	WORD x, y;	/* upper left corner in screen coordinates */
	UWORD w, h;	/* width, height */
	UWORD bpr;	/* bytes per row of image p */
	PLANEPTR p;
	UBYTE *cmap;
	UWORD ix;	/* where in sequence we are */
	};
typedef struct vcel Vcel;
extern Vcel *cel;
extern Vcel *alloc_cel(), *alloc_bcel(), *clone_cel();


struct blitblock 
	{
	WORD width, height;
	WORD sx, sy, *spt, sbpr;
	WORD dx, dy, *dpt, dbpr;
	WORD color;
	};
/* graphics macros */
extern a1blit(),a2blit(),blit8(),tblit8();	/* lo level PC gfx funx */
#ifndef SLUFF
#define colblock(color, x, y, x2, y2)  \
	cblock(vf.p,x,y,(x2)-(x)+1, (y2)-(y)+1, color)
#define colrop(color, x, y, width, height) \
	cblock(vf.p,x,y,(width+1),(height+1),color)
#define xorrop(color, x, y, width, height) \
	xorblock(vf.p,x,y,(width),(height),color)
#define cdraw_brush(brush, x, y, color) \
	a1blit(16, 16, 0, 0, brush, 2, (x)-8, (y)-8, vf.p, vf.bpr, color)
#define draw_brush(brush, x, y, color) \
	a1blit(16, 16, 0, 0, brush, 2, x, y, vf.p, vf.bpr, color)
#define hline(y, x0, x1, color) \
	chli(vf.p, x0, y, (x1)-(x0)+1, color)
#define vline(x, y0, y1, color) \
	cvli(vf.p, x, y0, (y1)-(y0)+1, color)
#define color_hslice(y, height, color) \
	cblock(vf.p,0,y,vf.w,height,color)
#define putdot(x, y, color) cdot(vf.p, x, y, color)
#define getdot(x,y)	getd(vf.p,x,y)
#define gtext(s, x, y, color) systext(s,x,y,color,a1blit)
#define stext(s, x, y, color,color1) systext(s,x,y,color,a2blit,color1)
#define marqi_frame(x0,y0,x1,y1) \
 some_frame(x0,y0,x1,y1,marqidot, &marqidata)
#define undo_frame(x0,y0,x1,y1) \
 some_frame(x0,y0,x1,y1,copydot, uf.p)
#endif /* SLUFF */

extern UBYTE sys_cmap[], init_cmap[];

struct marqidata
	{
	UBYTE mod;
	UBYTE c0,c1;
	}; 
extern struct marqidata marqidata;
extern copydot(), marqidot(), sdot();

/* macro to undraw a stand_by() */
#define stood_by() undraw_tbox()

struct cgrid
	{
	WORD divx, divy;
	WORD startc, dc;
	};

struct point
	{
	int x,y;
	};
typedef struct point Point;

struct vertex
	{
	int x,y,z;
	};
typedef struct vertex Vertex;

extern char loaded_screen;	/* another flag kludge */

#define SIXTY4K ((1L<<16)-16)

/* 'normal' perspective */
#define GROUND_Z 512	
#define SCALE_ONE 	(1<<14)

struct ado_setting
	{
	struct ado_setting *next;
	Vertex spin_center;
	Vertex spin_axis;
	Vertex spin_theta;
	WORD itheta1, itheta2;	/* filled in by program */
	Vertex size_center;
	WORD  xp, xq, yp, yq, bp, bq;
	Vertex move;
	};



#define MMD_PAINT 0
#define MMD_TITLE 1
#define MMD_EDIT 2

#define CBUF_SIZE (64000L+3*COLORS+256+16+sizeof(struct video_form) )

/* names of all our temp files */
extern char tflxname[], new_tflx_name[], tmacro_name[], alt_name[], cel_name[],
	screen_name[], text_name[], cclip_name[], poly_name[], mask_name[],
	crestore_name[], another_name[], optics_name[],
	poly1_name[], poly2_name[],
	poly_name[], default_name[], ppoly_name[], bscreen_name[];

extern char dirty_file;	/* need to resave file? */
extern char dirty_frame;	/* need to recompress frame? */

extern UBYTE pure_white[], pure_black[], pure_red[], pure_green[], pure_blue[];
#define copy_cmap(s,d) copy_structure(s,d,COLORS*3)

/* Some variable to maintain start and stop of time range we're
   currently concerned with. */
extern int tr_r1,tr_r2;
extern int tr_rdir;
extern int tr_tix;
extern int tr_frames;

/* variables to clip area we render to */
extern int render_xmin, render_ymin, render_xmax, render_ymax;

/* some day hope these will be changed so alway can be alloced */
#define paskmem askmem
#define pfreemem freemem

extern UBYTE *dot_pens[];

extern char menus_up;

/* stuff for my buffered io */
#define BSIZE 2048
struct bfile
	{
	int fd;
	int left;
	UBYTE *buf;
	UBYTE *filept;
	long fpos;
	int writable;
	};
typedef struct bfile Bfile;

#define TWOPI 1024

struct config
	{
	int	cmagic;				/* Verify it's right file type magic number */
	char scratch_drive;		/* Index of drive for temp files */
	char noint;				/* Should we install clock interrupt? */
	char font_drawer[71];	/* Where to find the fonts */
	char dev_type;			/* 0 = mouse, 1 = tablet */
	int comm_port;			/* which comm port for a serial tablet */
	char pucky4;			/* summa swap mouse buttons? */
	char pad[5];			/* all zeroes currently */
	};
extern struct config vconfg;


#define VP_SETTING 0x9499
struct bundle
	{
	int bun_count;
	UBYTE bundle[256];
	};

extern UBYTE *cluster_bundle();

#define START_SIXCUBE 0

#define MC_BLACK 0
#define MC_GREY 1
#define MC_WHITE 2
#define MC_BRIGHT 3
#define MC_RED 4

extern WORD mc_colors[];

#define sblack (mc_colors[MC_BLACK])
#define sgrey (mc_colors[MC_GREY])
#define swhite (mc_colors[MC_WHITE])
#define sbright (mc_colors[MC_BRIGHT])
#define sred (mc_colors[MC_RED])

#define BOX_TOOL 0
#define CIRCLE_TOOL 1
#define CURVE_TOOL 2
#define DRAW_TOOL 3
#define DRIZ_TOOL 4
#define FILL_TOOL 5
#define FILLTO_TOOL 6
#define GEL_TOOL 7
#define LINE_TOOL 8
#define MOVE_TOOL 9
#define OVAL_TOOL 10
#define PETAL_TOOL 11
#define POLY_TOOL 12
#define RPOLY_TOOL 13
#define SEP_TOOL 14
#define SHAPE_TOOL 15
#define SPIRAL_TOOL 16
#define SPRAY_TOOL 17
#define STAR_TOOL 18
#define STREAK_TOOL 19
#define TEXT_TOOL 20



struct vsettings 
	{
	long size;
	WORD type;	/* == VP_SETTINGS */
	WORD frame_ix;
	WORD ccolor;
	WORD zero_clear;
	WORD tool_ix;
	WORD pen_width;
	WORD large_pen;
	unsigned char inks[8];
	WORD rmyoff;
	struct cgrid crange;
	WORD draw_mode;
	WORD wierd_draw_mode;
	unsigned char tool_slots[6];
	WORD zoomx, zoomy, zoomw, zoomh, zoomscale, zoom_mode;
	WORD tint_percent;
	WORD menu_mode;
	WORD font_ix;
	WORD twin_x, twin_y, twin_w, twin_h;
	char drawer[71];
	char file[81];
	UBYTE dm_slots[6];
	char fonts[1][10];
	WORD text_yoff;
	WORD tcursor_x, tcursor_y;
	WORD fscroller_top, oscroller_top, dm_scroller_top;
	WORD star_points, star_ratio;
	WORD make_mask, use_mask;
	WORD gridx,gridy,gridw,gridh;
	UBYTE use_grid;
	UBYTE dthresh;	
	WORD air_speed, air_spread;
	WORD dither;
	WORD qdx, qdy;	/* quantize grid coordinates */
	WORD rgx,rgy,rgr;	/* radial gradient things */
	WORD fillp;		/* fill polygons */
	WORD color2;	/* outline filled polygons? */
	WORD mkx,mky;	/* marked point */
	WORD multi;		/* do it to many frames? */
	WORD closed_curve;	/* connect 1'st/last */
	WORD transition_frames;
	WORD start_seg, stop_seg;
	WORD browse_type;
	WORD browse_action;
	WORD fit_colors;
	WORD sep_rgb;
	WORD sep_threshold;
	WORD ado_tween;
	WORD ado_ease;
	WORD ado_pong;
	WORD ado_mode;
	WORD ado_spin;
	WORD ado_size;
	WORD ado_path;
	WORD ado_mouse;
	WORD ado_turn;
	WORD ado_szmouse;
	struct ado_setting move3;
	WORD ado_first;
	WORD ado_reverse;
	WORD ado_complete;
	WORD ado_source;
	WORD ado_outline;
	WORD sp_tens;
	WORD sp_cont;
	WORD sp_bias;
	WORD time_mode;		/* to frame/segment/all */
	unsigned char mcideals[6][3];
	WORD sep_box;
	WORD marks[4];
	WORD starttr[4];
	WORD stoptr[4];
	WORD bframe_ix;
	WORD hls;
	WORD pal_to;
	WORD use_bun;	/* which bundle to use */
	struct bundle buns[2];
	WORD cclose;	/* how close a color (%) to go into bundle ? */
	WORD ctint;		/* WHat percentage to tint in palette remap tint */
	WORD cycle_draw;	
	WORD cdraw_ix;
	WORD tit_just;
	WORD tit_scroll;
	WORD tit_move;
	WORD pal_fit;
	WORD pa_tens;
	WORD pa_cont;
	WORD pa_bias;
	WORD pa_closed;
	WORD ado_ease_out;
	WORD cblend;
	WORD zoom4;
	UBYTE inkstrengths[32];
	char big_pad[44];
	};
typedef struct vsettings Vsettings;
extern Vsettings vs, default_vs;

extern long dfree(int device);

extern char devices[26];
extern int dev_count;
