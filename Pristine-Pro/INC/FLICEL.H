#ifndef FLICEL_H
#define FLICEL_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif

#ifndef A3D_H
	#include "a3d.h"
#endif

#ifndef FLI_H
	#include "fli.h"
#endif

#ifndef RASTRANS_H
	#include "rastrans.h"
#endif

#ifndef FLIPATH_H
	#include "flipath.h"
#endif

/* the celdata fli chunk */

	/* cel position description */
#define CDAT_POS_START cent

#define get_fcelpos(fc,cp) (*(cp) = *((Fcelpos *)&((fc)->cd.CDAT_POS_START)))
#define put_fcelpos(fc,cp) (*((Fcelpos *)&((fc)->cd.CDAT_POS_START)) = *(cp))

#define CDAT_POS_FIELDS \
 Short_xy cent;\
 Short_xy stretch;\
 Short_xyz rotang

typedef struct fcelpos {
	CDAT_POS_FIELDS;
} Fcelpos;

#ifdef COMMENT

	Short_xy cent;	  /* center of cel */
	Short_xy stretch; /* ammount added to cel width and height,
					   * for width, if <= -width cel is
					   * flipped and width is ammount < -width */
	Short_xyz rotang; /* angle of rotation 0 to 5760 == 0 to 360 degrees
					   * for x y and z */
#endif

#define FCEL_TWOPI (360*16)  /* 5760 may seem like a weird number but it's
								evenly divisible by 360, 128,48,32,16,8,4,2,
								18,9,12,6,3,
								80,60,40,30,20,10,5, */

#define CELDATA_VERS 0

typedef struct celdata {
	Fat_chunk id;	  /* type = FC_CELDATA */
	CDAT_POS_FIELDS;
	SHORT cur_frame;	  /* current frame in cel fli */
	LONG next_frame_oset; /* offset to next (after current) frame in fli */

	SHORT tcolor;		  /* pre translation tcolor for this cel */

	/* total size 66 bytes */
	UBYTE cdpad[64 - POSTOSET(struct celdata,next_frame_oset)];
} Celdata;

typedef struct celcfit {
	ULONG src_cksum;	/* checkcum of last cel cmap */
	ULONG dst_cksum;	/* checkcum of last destination cmap */
	SHORT ccolor;		/* ccolor of last cfit */
	SHORT tcolor;		/* tcolor of last cfit make */
	SHORT ink0; 		/* ink0 of last cfit make */
	USHORT flags;		/* some flags */
	Pixel ctable[COLORS]; /* color translation table */
} Celcfit;

#define CCFIT_NULL	0x0001	/* cfit was made and no fitting required */

typedef struct flicel {
	Rcel *rc;		/* optional image cel needed for seeking etc. */
	Celdata cd; 	/* where position lives: celdata chunk for this cel */
	Flifile flif;	/* fli file for this cel when open */
	char *tpath;	/* allocated temp file path */
	Flipath *cpath; /* name and id of cel fli file */
	SHORT frame_loaded; /* frame currently loaded in cel <0 = unloaded */
	ULONG flags;		/* some flags */
	ULONG pos_cksum;	/* last refresh position crcsum */
	Xformspec xf;		/* transform spec for cel contains bounding poly and
						 * tcolor, and minmax data kept updated by move
						 * functions */
	Celcfit *cfit;		/* cfit table to use for this cel */
	struct flicel_lib *lib;  /* function library for this flicel */
} Flicel;

#define FCEL_XFORMED	0x0001	/* cel is transformed */
#define FCEL_DOCFIT 	0x0002	/* cfit table is set by make_celcfit */
#define FCEL_OWNS_CFIT	0x0004	/* cfit table is "owned" by cel and freeable */
#define FCEL_OWNS_RAST	0x0008	/* raster is "owned" by cel and freeable */
#define FCEL_RAMONLY	0x0010	/* cel is not linked to files, it is in ram
								 * only, only used for one frame cels */

#ifdef BIGCOMMENT
/***** structure of a celinfo.tmp file for temp cels ******/

this file may be either a fli or a file pointing to a fli, If it is a fli
it will have a celdata as the first chunk in the prefix chunk
(FCID_CELDATA is the id of this chunk) If the file only points to a fli file
It will have a dummy fli header as the first field and a prefix chunk as in the
fli case.  This is so the virtual celinfo.tmp file will have the same offset
to the to the celdata as the fli case. For convenience. also we may use
the fli header and have the current frame of the cel stored as a 1 frame fli
in the virtual case if seek time to the frame is too long to get to the frame.
on the fli pointed to. Whether a celinfo.tmp is THE cel or points to another
fli is determined by the existence or lack of a CELPATH chunk after the
CELDATA chunk. If it is present, the celinfo.tmp file "points" to the fli file
in the celpath chunk.

struct celinfo_file {
	Fli_head hdr;	   /* fli header */
	Chunk_id pfchunk; /* prefix chunk header */
	Celdata cdata;	   /* celdata chunk */
	Flipath cpath;	   /* optional celpath chunk (if a "virtual" cel) */
	/* Frames ? ... */
};

#endif /* BIGCOMMENT */

#define CELDATA_OFFSET (sizeof(Fli_head) + sizeof(Chunk_id))

void noask_delete_the_cel(void);

Errcode alloc_fcel(Flicel **pfc);
void free_fcel(Flicel **pfc);
void free_fcel_raster(Flicel *fc);
Errcode alloc_fcel_raster(Flicel *fc);
load_temp_fcel(char *tempname,Flicel **fc);
save_fcel_temp(Flicel *fc);
Errcode load_fli_fcel(char *flipath,char *tempname,char *celfli_name,
					  Flicel **pfc);

void move_flicel(Flicel *vc);

Boolean make_flicelpoly(Flicel *cel);

Errcode make_cfcel(Flicel *vc);
void free_cfcel(Flicel *vc);
Errcode load_cel(char *name);
Boolean need_render_cfit(Cmap *scmap);
Boolean make_render_cfit(Cmap *scmap, Celcfit *cfit, SHORT tcolor);
Errcode draw_flicel(Flicel *fc,int drawmode,int cfitmode);

/* draw flicel options */
#define DRAW_DELTA	0
#define DRAW_FIRST	1
#define DRAW_RENDER 2

/* draw flicel cfit options */
#define NO_CFIT 	0
#define OLD_CFIT	1
#define FORCE_CFIT	2
#define NEW_CFIT	4

void set_flicel_tcolor(Flicel *fc,Pixel tcolor);
Boolean refresh_flicel_pos(Flicel *cel);
Boolean maybe_ref_flicel_pos(Flicel *cel);
void set_flicel_center(Flicel *fc,SHORT x, SHORT y);
void rotate_flicel(Flicel *fc,Short_xyz *drotate);
void translate_flicel(Flicel *fc, SHORT dx, SHORT dy);

Errcode gb_seek_fcel_frame(Flicel *fc, SHORT frame,
						   Fli_frame *cbuf,Boolean force_read);
Errcode gb_abseek_fcel_frame(Flicel *fc, SHORT frame,Fli_frame *cbuf);

Errcode seek_fcel_frame(Flicel *fc, SHORT frame);


#endif /* FLICEL_H */
