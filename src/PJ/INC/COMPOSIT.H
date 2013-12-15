#ifndef COMPOSIT_H
#define COMPOSIT_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef FLICEL_H
	#include "flicel.h"
#endif


enum {
	FIT_TOA = 0,
	FIT_TOB = 1,
	FIT_BLEND = 2, /* note: no 1 bit */
};

#define INVERT_CFITMODE(cm) ((cm)^1)  /* inverts cfit mode ie: a is b,b is a */

enum {
	COMP_CUT = 0,
	COMP_MASK,
	COMP_DISSOLVE,
	COMP_BOXIL,
	COMP_CIRCLE,
	COMP_HWEDGE,
	COMP_VERTW,
	COMP_BOXLTOP,
	COMP_BOXRTOP,
	COMP_BOX,
	COMP_VWEDGE,
	COMP_HORIZW,
	COMP_BOXRBOT,
	COMP_BOXLBOT,
	COMP_VENETIAN,
	COMP_LOUVER,
	COMP_DIAGTOP,
	COMP_DIAGBOT,
	COMP_DIAMOND,
};

typedef struct compo_cb {
/* Items to fill in before calling render_composite. */
	Flipath *tflxpath;  /* saved tempflx flipath */
	Flipath *maskpath;  /* saved flipath for mask cel */
	Flicel *fcela;		/* cel for tempflx used during render */
	Flicel *fcelb;		/* cel for fli b */
	Flicel *mask_cel;   /* cel for mask */

/* items calculated by render composite */

	Flicel *start_cel; /* cela items refer to this */
	Flicel *end_cel;   /* celb items refer to this */
	SHORT cela_start;     /* start of cela */
	SHORT cela_frames;    /* frames before transition of cela */ 
	SHORT celb_start;     /* start of celb (first of transition) */
	SHORT celb_frames;    /* frames after transition of celb */
	SHORT cela_transtart; /* first transition frame of cela */
	SHORT celb_tailstart; /* first non-transition frame of celb */
	SHORT transition_frames; /* number of transition frames */
	SHORT total_frames;   /* total frames in entire composite result */
	SHORT flx_ix;
	SHORT cfit_type; /* adjusted for start-end order selected */
	int clearc;		 /* clear color for cfitting -1 or 0 */
	Boolean preview_mode; /* true if in preview mode */
	int preview_frame; /* frame index for abort verify */
	char *tempfli_name;

} Compocb;

extern Compocb ccb;

Errcode render_composite(Boolean preview);
Errcode draw_slatmask(Boolean *pvertical,USHORT size);
void zoom_boxil_mask(Raster *boxil_mask,Raster *dest,Coor hsize,Coor vsize);

#endif /* COMPOSIT_H */
