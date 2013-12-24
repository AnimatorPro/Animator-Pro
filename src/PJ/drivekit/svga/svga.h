/*****************************************************************************
 * SVGA.H - Header for SVGA device driver.
 ****************************************************************************/

#ifndef STDTYPES_H
	#include <stdtypes.h>
#endif

#ifndef ERRCODES_H
	#include <errcodes.h>
#endif

#ifndef RASTER_H
	#include <raster.h>
#endif

#ifndef DRVCOMN_H
	#include <drvcomn.h>
#endif

typedef struct svgarast {
	RASTHDR_FIELDS;
	struct
	 rastlib *grclib;
	long	 filler[3];
} SVGARast;

/* Make sure that driver's hardware specific raster is the right size.
 * (Following code will force a compiler syntax error if it's not.) */
struct _rast_error_check_ {
	char xx[sizeof(SVGARast) == sizeof(Raster)];
	};

/* This definition - LibRast - lets the compiler use your hardware specific
 * raster structure as the "Raster" parameter to device and raster library
 * function vectors without the compiler requiring lots and lots of
 * type-casts or complaining about type-mismatches. It must be defined
 * _before_ rastlib.h or vdevice.h are included. */

#define Raster SVGARast

#ifndef RASTLIB_H
	#include <rastlib.h>
#endif

#ifndef VDEVICE_H
	#include <vdevice.h>
#endif

/*----------------------------------------------------------------------------
 * protos for things that live in svgaintf.asm...
 *--------------------------------------------------------------------------*/

extern int	pj_svga_init(SMInfo *sminf_array);
extern void pj_svga_cleanup(void);
extern int	pj_svga_setmode(SMInfo *sminf);
extern void pj_svga_clrmode(long old_mode_data);
extern char *pj_svga_getname(void);
