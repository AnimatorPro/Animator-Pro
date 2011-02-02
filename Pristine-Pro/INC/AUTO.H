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

typedef struct autoarg {
	EFUNC avec; /* called as (*avec)(avecdat,int ix,int intween,int scale,
								     Autoarg *this) */
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
Errcode noask_do_auto(Autoarg *aa, int mode); /* does not call multi menu
											   * just does it */

Errcode do_auto(Autoarg *aa);
Errcode noask_do_auto_time_mode(Autoarg *aa);
Errcode noask_do_auto(Autoarg *aa, int frame_mode);

Errcode go_autodraw(EFUNC avec, void *avecdat, USHORT flags);

/* found in vs.time mode */

enum automodes {
	DOAUTO_FRAME =	0,
	DOAUTO_SEGMENT = 1,
	DOAUTO_ALL = 2,
};

#endif /* AUTO_H */
