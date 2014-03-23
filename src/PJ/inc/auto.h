#ifndef AUTO_H
#define AUTO_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef CMAP_H
	#include "cmap.h"
#endif

#ifndef FLICEL_H
	#include "flicel.h"
#endif

struct autoarg;

typedef Errcode (*autoarg_func)
	(void *data, int ix, int intween, int scale, struct autoarg *aa);

typedef struct autoarg {
	autoarg_func avec;
	void *avecdat;
	USHORT flags;

/* items set or used internally */
	UBYTE in_preview; /* in preview mode */
	Rgb3 auto_rgb;
	Rgb3 auto_8rgb;
	SHORT frames_in_seq;  /* total frames in sequence */
	SHORT cur_frame;  /* current frame in sequence 0 == first */
	SHORT doing_overlays;
	Fcelpos cpos;  /* cel position for restore on abort */
	SHORT cframe;    /* cel frame for restore on abort */
	SHORT celjmode;
} Autoarg;

#define AUTO_READONLY	0x0001  /* doesn't need to save screen only reads 
								 * also installs "from" labels on multi menu */
#define AUTO_USESCEL	0x0002	/* uses the cel */
#define AUTO_UNZOOM		0x0004  /* unzoom when doing do_auto */
#define AUTO_HIDEMP		0x0008  /* hide menus doing do_auto */
#define	AUTO_PUSHINKS	0x0010  /* push items not needed by inks */
#define	AUTO_PUSHMOST	0x0020  /* push "most" */
#define	AUTO_PUSHCEL	0x0040  /* push cel */
#define AUTO_NOCELSEEK	0x0080  /* do not do auto cel seeking */
#define AUTO_USES_UNDO  0x0100  /* set if undo is altered by avec */
#define AUTO_PREVIEW_ONLY 0x0200 /* multi menu is only to do preview,
									render is only to return Success */

/* some external variables that find_seg_range() sets up for us to
 * tell us how many frames are in the time segment etc.
 */
extern SHORT tr_r1;
extern SHORT tr_r2;
extern SHORT tr_rdir;
extern SHORT tr_tix;
extern SHORT tr_frames;

/* auto.c */
extern Boolean auto_abort_verify(void *aa);
extern void pmhmpauto(autoarg_func what, void *data);
extern void hmpauto(autoarg_func what, void *data);
extern Errcode uzauto(autoarg_func what, void *data);
extern void clear_pic(void);
extern void auto_blue_nums(void);
extern int auto_trails(void);
extern void greys_only(void);
extern void auto_engrave(void);
extern void auto_dither(void);
extern void auto_put(void);
extern void auto_set(void);
extern void crop_video(void);
extern void auto_shrink(void);
extern void quantize(void);
extern void auto_expand(void);
extern void auto_setup(Autoarg *aa);
extern Errcode auto_restores(Autoarg *aa, Errcode err);
extern Errcode noask_do_auto(Autoarg *aa, int frame_mode);
extern Errcode noask_do_auto_time_mode(Autoarg *aa);
extern void clip_tseg(void);
extern Errcode auto_merge_overlays(void);
extern void find_seg_range(void);
extern void find_range(void);
extern int calc_time_scale(int ix, int intween);
extern Errcode auto_apply(Autoarg *aa, int ix, int intween);
extern Errcode dopreview(Autoarg *aa);
extern Errcode do_autodraw(autoarg_func avec, void *avecdat);
extern Errcode go_autodraw(autoarg_func avec, void *avecdat, USHORT flags);
extern Errcode do_auto(Autoarg *aa);

/* autoseg.c */
extern Errcode dseg(Autoarg *aa);

/* found in vs.time mode */

enum automodes {
	DOAUTO_FRAME =	0,
	DOAUTO_SEGMENT = 1,
	DOAUTO_ALL = 2,
};

#endif
