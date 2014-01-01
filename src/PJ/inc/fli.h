#ifndef FLI_H
#define FLI_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef JFILE_H
	#include "jfile.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef VMAGICS_H
	#include "vmagics.h"
#endif

/***** end file types ******/

#define MAXFRAMES (4*1000)	/* Max number of frames... */

typedef struct fli_id {
	LONG create_time;	 /* time file created */
	LONG create_user;	 /* user id of creator */
	LONG update_time;	 /* time of last update */
	LONG update_user;	 /* user id of last update */
} Fli_id;

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

typedef struct fhead_1_0 {
	CHUNKID_FIELDS;
	USHORT frame_count;
	USHORT width;
	USHORT height;
	USHORT bits_a_pixel;
	SHORT flags;
	SHORT jiffy_speed;
	UBYTE pad[110]; 	/* should be total of 128 bytes */
} Fhead_1_0;

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

typedef struct fli_head {
	FHEAD_COMMON;
	LONG frame1_oset;
	LONG frame2_oset;
	UBYTE padfill[40];
} Fli_head;

struct _fli_h_error_check_ {
	char x0[sizeof(Fli_head) == 128];
#ifdef FLI_1_0
	char x1[sizeof(Fhead_1_0) == 128];
#endif /* FLI_1_0 */
};

/* Fli_head flags values */

#define FLI_FINISHED 0x0001
#define FLI_LOOPED	 0x0002

typedef struct fli_frame {
	CHUNKID_FIELDS; /* type = FCID_FRAME */
	SHORT chunks;
	char pad[8];
} Fli_frame;

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


typedef struct pstamp_chunk {
	CHUNKID_FIELDS;
	SHORT height;
	SHORT width;
	SHORT xlat_type;
	Chunk_id data; /* this is a fli chunk either copy or brun */
} Pstamp_chunk;

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

extern const Flicomp
			pj_fli_comp_aa, 	/* low res animator 320 X 200 fli */
			pj_fli_comp_ani;	/* high res animator variable resolution fli */


/* a fli file object used with fli library routines */

typedef struct flifile {
	Fli_head hdr;  /* the fli hdr for the fli */
	Jfile fd;	   /* file handle for this fli */
	Flicomp comp_type;	/* compression type if this fli is a created fli */
} Flifile;


/**** low level calls in fli library *****/

/* compression buffer allocation */

LONG pj_fli_cbuf_size(USHORT width,USHORT height, LONG num_colors);
Errcode pj_fli_alloc_cbuf(Fli_frame **pcbuf, USHORT width,USHORT height,
				   LONG num_colors);
LONG pj_fli_cel_cbuf_size(Rcel *cel);
Errcode pj_fli_cel_alloc_cbuf(Fli_frame **pcbuf, Rcel *cel);

/* Flifile header checking open and close */

Errcode pj_fli_read_head(char *title, Fli_head *flih, Jfile *pfd,int jmode);
Errcode pj_fli_is_valid(char *path);

Errcode pj_fli_open(char *path, Flifile *flif, int jmode);

Errcode pj_fli_create(char *path, Flifile *flif);
Errcode pj_fli_create_aa(char *path, Flifile *flif);

void pj_fli_close(Flifile *flif);

/* compressed record adding */

Errcode pj_i_add_frame1_rec(char *name, Flifile *flif, Fli_frame *frame);
Errcode pj_i_add_next_rec(char *name, Flifile *flif, Fli_frame *frame);
Errcode pj_i_add_ring_rec(char *name, Flifile *flif, Fli_frame *frame);

/* record compressing and adding */

Errcode pj_fli_add_frame1(char *name, Flifile *flif, void *cbuf, Rcel *frame1);
Errcode pj_fli_add_next(char *name, Flifile *flif, void *cbuf,
					 Rcel *last_screen, Rcel *this_screen);
Errcode pj_fli_add_ring(char *name, Flifile *flif, void *cbuf,
					 Rcel *last_screen, Rcel *first_screen);
Errcode pj_fli_finish(char *name, Flifile *flif, void *cbuf,
				   Rcel *last_screen, Rcel *work_screen);

/* empty record handleing */

Boolean pj_i_is_empty_rec(Fli_frame *frame);
Errcode pj_i_add_black1(char *name,Flifile *flif,Rcel *screen);
void pj_i_get_empty_rec(Fli_frame *frame);
Errcode pj_i_add_empty(char *name,Flifile *flif);
Errcode pj_i_add_empty_ring(char *name,Flifile *flif);

/* compression */

/* frame compress control for fli_comp_rect() and fli_comp_cel() what
 * type of frame is being compressed */

#define COMP_FIRST_FRAME  0 /* compress a first frame record */
#define COMP_DELTA_FRAME  1 /* compress a delta frame record */
#define COMP_BLACK_FIRST  2 /* compress a first black frame record */

LONG pj_fli_comp_rect(void *comp_buf, Rcel *last_screen, Rcel *this_screen,
				   Rectangle *rect, Boolean do_colors, SHORT frame_type,
				   Flicomp comp_type);

LONG pj_fli_comp_cel(void *comp_buf, Rcel *last_screen,
				  Rcel *this_screen, SHORT frame_type, Flicomp comp_type);

LONG pj_fli_comp_frame1(void *cbuf,Rcel *this_screen, Flicomp comp_type);

/* reading and decompression */

Errcode pj_fli_seek_first(Flifile *flif);
Errcode pj_fli_seek_second(Flifile *flif);

Errcode pj_fli_read_uncomp(char *name, Flifile *flif, Rcel *fscreen,
						Fli_frame *ff, int colors);

Errcode pj_fli_read_first(char *name, Flifile *flif, Rcel *fscreen,
					   Boolean colors );
Errcode pj_fli_read_next(char *name, Flifile *flif, Rcel *fscreen,
					  Boolean colors );

extern void pj_fcuncomp(const UBYTE *src, Rgb3 *dst);
extern void pj_fcuncomp64(const UBYTE *src, Rgb3 *dst);

/*----------------------------------------------------------------------------
 * This proto is for a temporary kludge to eliminate flilib references to
 * the errline() function.	See comments in FLI\FLIERROR.C for details.
 *--------------------------------------------------------------------------*/

Errcode pj_fli_error_report(Errcode err, char *msg, char *filename);

#endif /* FLI_H */
