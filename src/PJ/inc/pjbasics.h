#ifndef PJBASICS_H
#define PJBASICS_H


/*** this include file contains basic stuff that is common to all pj 
 * 	 applications that use graphics menus the fileing system and menus */

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif

#ifndef MEMORY_H
	#include "memory.h"
#endif

#ifndef INPUT_H
	#include "input.h"
#endif 

#ifndef FILEPATH_H
	#include "filepath.h"
#endif

#ifndef WNDO_H
	#include "wndo.h"
#endif

#ifndef MENUS_H
	#include "menus.h"
#endif

#ifndef VDEVICE_H
	#include "vdevice.h"
#endif

struct screen_mode;

typedef struct vbasics_cb {
	SHORT ivmode;	    /* initial Video mode program started from */
	char init_drawer[PATH_SIZE];	/* directory program started from */
	Vdevice *vd;    	/* video driver for main display */
	Vdevice *ram_vd;	/* video driver for ram cels */
	Rcel *cel_a;        /* two displayable cels for screen */
	Rcel *cel_b;
	Wscreen *screen;	/* the main window screen */
	Short_xy scrcent;   /* pre calc'd center of screen */
	Rcel *pencel;		/* the drawing cel This is an Rcel* for uniformity
						 * with other Rcels even though it is actually
						 * a Wndo* It is put here and will be set to the 
						 * screen window by default unless altered */

#define PENWNDO ((Wndo *)(vb.pencel)) /* for when we need it as a Wndo */

	char *config_name;  /* config file name */
	char *vdriver_name; /* video driver name from command line args */
	SHORT vdriver_mode; /* video driver mode from command line args */
} Vbcb; 

/* defines for access to menu colors for screen */

#define sblack (vb.screen->mc_colors[MC_BLACK])
#define sgrey (vb.screen->mc_colors[MC_GREY])
#define swhite (vb.screen->mc_colors[MC_WHITE])
#define sbright (vb.screen->mc_colors[MC_BRIGHT])
#define sred (vb.screen->mc_colors[MC_RED])

/***** globals *****/

extern Vbcb vb;  				/* in pjhigh.lib(startup.c) */
extern char resource_dir[PATH_SIZE];
extern Errcode builtin_err;     /* in pocolib(pocoface.c) */

/***** initializer functions ******/

#ifdef SCRNINIT_CODE
	#define OPTdata void*
#else
	#define OPTdata ...
#endif

Errcode open_pj_startup_screen(Errcode (*init_with_screen)(void *iwdat),
					OPTdata);

Errcode init_screen(struct screen_mode *smode, struct screen_mode *altmode,
					Errcode (*init_with_screen)(void *iwdat),OPTdata );

Errcode go_resize_screen(Errcode (*reinit)(void *dat),
						 void (*close_reinit)(void *dat), OPTdata );

#undef OPTdata

void get_default_cmap(Cmap *cm);
void clip_penwinrect(Rectangle *r);
Errcode set_pencel_size(SHORT width, SHORT height, SHORT xcen, SHORT ycen);
extern void fliborder_off(void);
extern void fliborder_on(void);
extern void erase_flibord(void);
extern void draw_flibord(void);

extern void hide_mp(void);
extern void show_mp(void);

ULONG jiffies_to_millisec(USHORT jiffies);
USHORT millisec_to_jiffies(ULONG millis);

#endif /* PJBASICS_H Leave at end of file */
