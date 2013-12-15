/*****************************************************************************
 * hgsc.h - header file for the hgsc driver
 ****************************************************************************/

#ifndef HGSC_H
#define HGSC_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif

#ifndef Array_els
  #define Array_els(a) (sizeof((a))/sizeof((a)[0]))
#endif

typedef struct {
	short	hwregs[14];
	short	width;
	short	height;
	short	pdepth;
	short	pitch;
	short	yshfmul;
} HWModedata;

typedef struct hgscrast {
	RASTHDR_FIELDS;
	void   *grclib;
	long	filler[3];
} Hrast;

/* Make sure that driver's hardware specific raster is the right size.
 * (Following code will force a compiler syntax error if it's not.) */

struct _rast_error_check_ {
	char xx[sizeof(Hrast) == sizeof(Raster)];
};

/* This definition - LibRast - lets the compiler use your hardware specific
 * raster structure as the "Raster" parameter to device and raster library
 * function vectors without the compiler requiring lots and lots of
 * type-casts or complaining about type-mismatches. It must be defined
 * _before_ rastlib.h or vdevice.h are included. */

#define LibRast Hrast

#ifndef RASTLIB_H
	#include "rastlib.h"
#endif

#ifndef VDEVICE_H
	#include "vdevice.h"
#endif

/*
 * graphics mode numbers as understood by the hgs_setmode() function...
 */

#define TM_VGA			 -1
#define TM_640x480x8	  0
#define TM_640x480x16	  1
#define TM_512x480		  2
#define TM_800x600		  3
#define TM_1024x768i	  4
#define TM_1024x768n	  5

/*
 * global function prototypes...
 */

/* in raster.c */

extern Rastlib *hgsc_get_rlib(Vdevice *dev, int mode, Hrast *r);

/* in hgsmodep.asm */

extern void  hgs_setmode(int mode, HWModedata **pmd);
extern void  hgs_setpalette(Hrast *r, int index, int count, void *rgbs);
extern void  hgs_wait_vsync(Hrast *r);
extern void  hgs_bus16_on(void);
extern void  hgs_bus16_off(void);

/* in hgsdots.asm */

extern void  hgs_putdot(Hrast *r, Pixel color, Coor x, Coor y);
extern void  hgs_cputdot(Hrast *r, Pixel color, Coor x, Coor y);
extern Pixel hgs_getdot(Hrast *r, Coor x, Coor y);
extern Pixel hgs_cgetdot(Hrast *r, Coor x, Coor y);

/* in hgssegs.asm */

extern void  hgs_get_hseg(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor w);
extern void  hgs_put_hseg(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor w);
extern void  hgs_get_vseg(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor h);
extern void  hgs_put_vseg(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor h);

/* in hgslines.asm */

extern void  hgs_set_hline(Hrast *r, Pixel color, Coor x, Coor y, Ucoor w);
extern void  hgs_set_vline(Hrast *r, Pixel color, Coor x, Coor y, Ucoor h);

/* in hgsrects.asm */

extern void  hgs_put_rectpix(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor w, Ucoor h);
extern void  hgs_get_rectpix(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor w, Ucoor h);
extern void  hgs_setrect(Hrast *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);
extern void  hgs_xorrect(Hrast *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);
extern void  hgs_setrast(Hrast *r, Pixel color);

/* in hgsdcomp.asm */

extern void  hgs_unss2();

#endif /* HGSC_H */
