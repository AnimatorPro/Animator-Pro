#ifndef FLI_H
#define FLI_H

#ifndef JFILE_H
	#include "jfile.h"
#endif

#ifndef VMAGICS_H
	#include "vmagics.h"
#endif

struct cmap;
struct rcel;
struct rectangle;
struct rgb3;

/***** end file types ******/

#define MAXFRAMES (4*1000)	/* Max number of frames... */

typedef struct fli_id {
	LONG create_time;	 /* time file created */
	LONG create_user;	 /* user id of creator */
	LONG update_time;	 /* time of last update */
	LONG update_user;	 /* user id of last update */
} Fli_id;
STATIC_ASSERT(fli, sizeof(Fli_id) == 16);

/* fli top level chunk ids */

#define FCID_PREFIX  0xf100U	   /* Fli prefix chunk (pre frame) */
#define FCID_FRAME	 0xf1faU	   /* Fli frame chunk */

/* prefix chunk sub chunk ids the only condition is that they be unique
 * and that none equal -1 */

#define FP_FREE 		0	/* unused space */
#define FP_FLIPATH		1	/* fli path, only valid in a cel or flx file */
#define FP_VSETTINGS	2	/* vsettings chunk */
#define FP_CELDATA		3	/* cel information chunk */
#define FP_DIRECTORY	4	/* Cleve's directory chunk */

#ifdef BIGCOMMENT

	/* fields common to both the fli_head and flx_head */

	CHUNKID_FIELDS;\
	USHORT frame_count;\  /* number of frames in this fli */
	USHORT width;\
	USHORT height;\
	USHORT bits_a_pixel;\ /* depth of a pixel */
	SHORT flags;\
	LONG speed;\		 /* speed in milisec 1/1000 sec per frame */
	USHORT unused;\
	Fli_id id;			 /* creator and update id */
	USHORT aspect_dx;	 /* aspect ratio ie: dx X dy is a square */
	USHORT aspect_dy;

#endif /* BIGCOMMENT */

#ifdef FLI_1_0

typedef struct GCC_PACKED fhead_1_0 {
	CHUNKID_FIELDS;
	USHORT frame_count;
	USHORT width;
	USHORT height;
	USHORT bits_a_pixel;
	SHORT flags;
	SHORT jiffy_speed;
	UBYTE pad[110]; 	/* should be total of 128 bytes */
} Fhead_1_0;
STATIC_ASSERT(fli, sizeof(Fhead_1_0) == 128);

#endif /* FLI_1_0 */

#define FHEAD_COMMON \
	CHUNKID_FIELDS;\
	USHORT frame_count;\
	USHORT width;\
	USHORT height;\
	USHORT bits_a_pixel;\
	SHORT flags;\
	LONG speed;\
	USHORT unused;\
	Fli_id id;\
	USHORT aspect_dx;\
	USHORT aspect_dy;\
	UBYTE commonpad[38]  /* should be total of 80 bytes (48 for unique) */

/* size of common fields */
#define FLIH_COMMONSIZE (POSTOSET(Fli_head,commonpad))

typedef struct GCC_PACKED fli_head {
	FHEAD_COMMON;
	LONG frame1_oset;
	LONG frame2_oset;
	UBYTE padfill[40];
} Fli_head;
STATIC_ASSERT(fli, sizeof(Fli_head) == 128);

/* Fli_head flags values */

#define FLI_FINISHED 0x0001
#define FLI_LOOPED	 0x0002

typedef struct fli_frame {
	CHUNKID_FIELDS; /* type = FCID_FRAME */
	SHORT chunks;
	char pad[8];
} Fli_frame;
STATIC_ASSERT(fli, sizeof(Fli_frame) == 16);

/* fli frame sub chunk types and ids */

enum {
	FLI_COL = 0,
	FLI_WRUN = 1,
	FLI_WSKIP = 2,
	FLI_WSKIP2 = 3,
	FLI_COLOR256 = 4,
	FLI_WSKIPRUN = 5,
	FLI_BSKIP = 6,
	FLI_SS2 = 7,
	FLI_BSC = 8,
	FLI_SBSC = 9,
	FLI_SBSRSC = 10,
	FLI_COLOR = 11,
	FLI_LC = 12,
	FLI_COLOR_0 = 13, /* whole frame is color 0 */
	FLI_ICOLORS = 14,
	FLI_BRUN = 15,
	FLI_COPY = 16,
	FLI_SOLID = 17,
	FLI_PSTAMP = 18, /* "postage stamp" chunk */
};


typedef struct GCC_PACKED pstamp_chunk {
	CHUNKID_FIELDS;
	SHORT height;
	SHORT width;
	SHORT xlat_type;
	Chunk_id data; /* this is a fli chunk either copy or brun */
} Pstamp_chunk;
STATIC_ASSERT(fli, sizeof(Pstamp_chunk) == 18);

#define PSTAMP_NOXLAT	0
#define PSTAMP_SIXCUBE	1

#define PSTAMP_W 100
#define PSTAMP_H 63

/* pstamp data sub chunk types and ids */

enum {
	FPS_BRUN = FLI_BRUN,		/* note this is the same */
	FPS_COPY = FLI_COPY,		/* note the same */
	FPS_XLAT256 = FLI_PSTAMP,	/* translation table */
};

/*******************************/
/* flitypes for use with fli_create, and compression routines,
 * will create a file of either the old lowres
 * 320 by 200 fli or the newer variable resolution format. The type is an
 * external symbol to avoid linking with both compression types if they are
 * not needed these are also input to frame compression routines */

typedef struct complib *Flicomp;

struct anim_info;

extern const Flicomp
			pj_fli_comp_ani;	/* high res animator variable resolution fli */


/* a fli file object used with fli library routines */

typedef struct flifile {
	Fli_head hdr;  /* the fli hdr for the fli */
	XFILE *xf; /* file handle for this fli */
	Flicomp comp_type;	/* compression type if this fli is a created fli */
} Flifile;


/**** low level calls in fli library *****/

/* compression buffer allocation */

LONG pj_fli_cbuf_size(USHORT width,USHORT height, LONG num_colors);
Errcode pj_fli_alloc_cbuf(Fli_frame **pcbuf, USHORT width,USHORT height,
				   LONG num_colors);
LONG pj_fli_cel_cbuf_size(struct rcel *cel);
Errcode pj_fli_cel_alloc_cbuf(Fli_frame **pcbuf, struct rcel *cel);

/* Flifile header checking open and close */

extern Errcode
pj_fli_read_head(const char *title, Fli_head *flih,
		XFILE **pxf, enum XReadWriteMode mode);

Errcode pj_fli_info(char *path, struct anim_info *ainfo);
Errcode pj_fli_info_open(Flifile *flif, char *path, struct anim_info *ainfo);

extern Errcode
pj_fli_open(char *path, Flifile *flif, enum XReadWriteMode mode);

extern Errcode
squawk_open_flifile(char *path, Flifile *flif, enum XReadWriteMode mode);

Errcode pj_fli_create(char *path, Flifile *flif);

extern Errcode pj_i_create(char *path, Flifile *flif);
extern void pj_i_update_id(Flifile *flif);
extern Errcode pj_i_flush_head(Flifile *flif);

void pj_fli_close(Flifile *flif);

/* compressed record adding */

Errcode pj_i_add_frame1_rec(char *name, Flifile *flif, Fli_frame *frame);
Errcode pj_i_add_next_rec(char *name, Flifile *flif, Fli_frame *frame);
Errcode pj_i_add_ring_rec(char *name, Flifile *flif, Fli_frame *frame);

/* record compressing and adding */

extern Errcode
pj_fli_add_frame1(char *name, Flifile *flif, void *cbuf, struct rcel *frame1);

Errcode pj_fli_add_next(char *name, Flifile *flif, void *cbuf,
		struct rcel *last_screen, struct rcel *this_screen);
Errcode pj_fli_add_ring(char *name, Flifile *flif, void *cbuf,
		struct rcel *last_screen, struct rcel *first_screen);

/* empty record handleing */

Boolean pj_i_is_empty_rec(Fli_frame *frame);
void pj_i_get_empty_rec(Fli_frame *frame);
Errcode pj_i_add_empty(char *name,Flifile *flif);
Errcode pj_i_add_empty_ring(char *name,Flifile *flif);

/* compression */

/* frame compress control for fli_comp_rect() and fli_comp_cel() what
 * type of frame is being compressed */

#define COMP_FIRST_FRAME  0 /* compress a first frame record */
#define COMP_DELTA_FRAME  1 /* compress a delta frame record */
#define COMP_BLACK_FIRST  2 /* compress a first black frame record */

extern LONG
pj_fli_comp_rect(void *comp_buf,
		struct rcel *last_screen, struct rcel *this_screen,
		struct rectangle *rect, Boolean do_colors, SHORT frame_type,
		Flicomp comp_type);

extern LONG
pj_fli_comp_cel(void *comp_buf,
		struct rcel *last_screen, struct rcel *this_screen,
		SHORT frame_type, Flicomp comp_type);

extern LONG
pj_fli_comp_frame1(void *cbuf, struct rcel *this_screen, Flicomp comp_type);

extern Errcode
pj_write_one_frame_fli(char *name, Flifile *flif, struct rcel *screen);

extern Errcode jwrite_chunk(XFILE *xf, void *data, LONG size, SHORT type);

/* reading and decompression */

extern void pj_fcuncomp(const UBYTE *src, struct rgb3 *dst);
extern void pj_fcuncomp64(const UBYTE *buf, struct rgb3 *dst);

extern void
pj_fli_uncomp_rect(struct rcel *f, Fli_frame *frame, struct rectangle *rect,
		int colors);

extern void
pj_fli_uncomp_frame(struct rcel *screen, Fli_frame *frame, int colors);

Errcode pj_fli_seek_first(Flifile *flif);
Errcode pj_fli_seek_second(Flifile *flif);

extern Errcode fli_read_colors(Flifile *flif, struct cmap *cmap);

extern Errcode
pj_i_read_uncomp1(char *fname, Flifile *flif, struct rcel *fscreen,
		Fli_frame *ff, Boolean colors);

extern Errcode
pj_fli_read_uncomp(char *name, Flifile *flif, struct rcel *fscreen,
		Fli_frame *ff, int colors);

extern Errcode
pj_fli_read_first(char *name, Flifile *flif, struct rcel *fscreen,
		Boolean colors);

extern Errcode
pj_fli_read_next(char *name, Flifile *flif, struct rcel *fscreen,
		Boolean colors);

extern int fli_wrap_frame(Flifile *flif, int frame);

/* Postage stamp */

extern void pj_make_pstamp_xlat(struct rgb3 *ctab, UBYTE *xlat, int count);

extern void
pj_get_stampsize(SHORT maxw, SHORT maxh, SHORT sw, SHORT sh,
		SHORT *pw, SHORT *ph);

extern LONG
pj_build_rect_pstamp(struct rcel *screen, void *cbuf,
		SHORT x, SHORT y, USHORT width, USHORT height);

/*----------------------------------------------------------------------------
 * This proto is for a temporary kludge to eliminate flilib references to
 * the errline() function.	See comments in FLI\FLIERROR.C for details.
 *--------------------------------------------------------------------------*/

Errcode pj_fli_error_report(Errcode err, char *msg, char *filename);

extern LONG pj__fii_get_user_id(void);

/* fli.c */
extern Errcode
gb_unfli(struct rcel *screen, int ix, int wait, Fli_frame *frame);

extern Errcode flx_ringseek(struct rcel *screen, int curix, int ix);
extern Errcode unfli(struct rcel *f, int ix, int wait);
extern Errcode flisize_error(Errcode err, SHORT width, SHORT height);
extern Errcode resize_load_fli(char *flicname);
extern Errcode open_default_flx(void);
extern int wrap_frame(int frame);
extern void check_loop(void);
extern void advance_frame_ix(void);
extern void flip_range(void);
extern void loop_range(void);
extern void flip5(void);
extern Errcode fli_tseek(struct rcel *screen, int cur_ix, int new_ix);

extern Errcode
gb_fli_tseek(struct rcel *screen, int cur_ix, int new_ix,
		struct fli_frame *fbuf);

extern Errcode
gb_fli_abs_tseek(struct rcel *screen, int new_ix, struct fli_frame *fbuf);

extern Errcode fli_abs_tseek(struct rcel *screen, int new_ix);
extern void restore(void);

/* fli.c -- Minitime_data functions */
extern void first_frame(void *data);
extern void prev_frame(void *data);
extern void next_frame(void *data);
extern void mplayit(void *data);
extern void last_frame(void *data);
extern SHORT flx_get_frameix(void *data);
extern SHORT flx_get_framecount(void *data);
extern void flx_seek_frame(SHORT frame);
extern void flx_seek_frame_with_data(SHORT frame, void *data);

/* savefli.c */
extern void dirties(void);
extern void cleans(void);
extern Boolean need_scrub_frame(void);
extern Errcode scrub_frame_save_undo(void);
extern Errcode scrub_cur_frame(void);
extern Errcode sub_cur_frame(void);
extern Errcode sv_fli(char *name);
extern void qsave(void);
extern void qsave_backwards(void);
extern void qsave_segment(void);

#endif
