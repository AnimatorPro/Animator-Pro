#ifndef FLX_H
#define FLX_H

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif

#ifndef FLI_H
	#include "fli.h"
#endif

#ifndef RECTANG_H
#include "rectang.h"
#endif

#ifndef VERTICES_H
	#include "vertices.h"
#endif

struct flipath;

typedef struct GCC_PACKED flx_head {
	FHEAD_COMMON;
	LONG frames_in_table; /* size of index */
	LONG index_oset;	/* offset to index */
	LONG path_oset;     /* offset to flipath record chunk */

	char padding[36];
} Flx_head;
STATIC_ASSERT(flx, sizeof(Flx_head) == sizeof(Fli_head));

/* struct flx MUST be the same size or smaller than struct fli_frame
   for the 'add frames to sequence' routines to work. */

typedef struct flx {
	LONG foff;
	LONG fsize;
} Flx;

typedef struct flx_overlay {
	struct flx_overlay *next;
	UBYTE flags;        /* flags for render state and overlay type */
	UBYTE unused;
	Pixel ccolor;       /* ccolor when rendered */
	Pixel ink0;			/* ink 0 for this frame */
	SHORT cframe;		/* cel frame for this overlay */
	Short_xy cpos;    	/* cel position for this overlay */
	Rectangle pos;      /* compressed rectangle position */
	Fli_frame overlay;  /* compressed overlay record */
} Flx_overlay;

/* render state flags for overlay struct */

#define FOVL_FLIF	0x01    /* fli frame present */
#define FOVL_CEL	0x02    /* Cel blitted at cpos */

#define FOVL_UNDER	0x10 	/* render under */
#define FOVL_ZCLEAR	0x20 	/* tcolor on */
#define FOVL_CFIT	0x40 		/* auto fit */
#define FOVL_ONECOL	((UBYTE)0x80) 	/* cel one color option */

/* note this struct is the same as the Flifile through the fd (file handle)
 * so code can be smaller */

typedef struct flxfile {
	Flx_head hdr;  /* the fli hdr for the flx plus flx fields */
	Jfile fd;      /* file handle for this flx */

	Flicomp comp_type; /* compression type for this file always set to 
						* fli_comp_ani */

	Flx *idx;      /* frame index */
	Flx_overlay **overlays; /* array of overlays to add to flx frames */
	SHORT overlays_in_table;
	struct flipath *path; /* path of file loaded into tempflx */
} Flxfile;

extern Flxfile flix;
LONG write_tflx();

/* make sure fd and comp_type fields are in the same position as in a flifile */
STATIC_ASSERT(flx, OFFSET(Flifile, fd) == OFFSET(Flxfile, fd));
STATIC_ASSERT(flx, OFFSET(Flifile, comp_type) == OFFSET(Flxfile, comp_type));


/***** Flxfile functions ****/

#define update_flx_id(flx) pj_i_update_id((Flifile *)&((flx)->hdr))
Errcode create_flxfile(char *path, Flxfile *flx);
LONG flx_data_offset(Flxfile *flx);
Errcode read_flx_frame(Flxfile *flx,Fli_frame *frame, int ix);
Errcode gb_unfli_flx_frame(Flxfile *flx, struct rcel *screen, int ix,
						   int wait,Fli_frame *frame);
Errcode gb_flx_ringseek(Flxfile *flx, struct rcel *screen, int curix, int ix,
						Fli_frame *frame);

/**** functions for serially writing to a new empty flx */

Errcode write_first_flxchunk(char *name, Flxfile *flxf, Fli_frame *frame);
Errcode write_next_flxchunk(char *name, Flxfile *flxf, Fli_frame *frame);
Errcode write_ring_flxchunk(char *name, Flxfile *flxf, Fli_frame *frame);

extern Errcode
write_first_flxframe(char *name, Flxfile *flxf, void *cbuf,
		struct rcel *frame1);

extern Errcode
write_next_flxframe(char *name, Flxfile *flxf, void *cbuf,
		struct rcel *last_screen, struct rcel *this_screen);

extern Errcode
write_ring_flxframe(char *name, Flxfile *flxf, void *cbuf,
		struct rcel *last_screen, struct rcel *first_screen);

extern Errcode
write_first_flxblack(char *name, Flxfile *flxf, struct rcel *screen);

Errcode write_next_flxempty(char *name,Flxfile *flxf,int num_emptys);
Errcode write_ring_flxempty(char *name,Flxfile *flxf);

/* flx overlay functions */

void free_flx_overlays(Flxfile *flx);
Errcode alloc_flx_olaytab(Flxfile *flx, int tablesize);
void unfli_flx_overlay(Flxfile *flx, struct rcel *screen, int frame_ix);
Errcode push_flx_overlays();
Errcode pop_flx_overlays();

Errcode add_flx_olayrec(Short_xy *cpos, SHORT cframe,
				  	    Rectangle *fpos, Fli_frame *rec, int frame_ix);

/* path maintenance */

Errcode update_flx_path(Flxfile *flx, Fli_id *flid, char *fliname);
Errcode read_flx_path(Flxfile *flx, struct flipath *fp);

#define FLX_DEFAULT_SPEED 71

#endif
