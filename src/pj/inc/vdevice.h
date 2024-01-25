#ifndef VDEVICE_H
#define VDEVICE_H


#ifndef VDEV_INTERNALS	/* ---What the outside world sees ----- */

	typedef void *Vdevice;

#else  /* ----- Video device driver internal header file ------ */

#ifndef RASTER_H
	#include "raster.h"
#endif

#ifndef REXLIB_H
	#include "rexlib.h"
#endif

#ifndef LibRast
/* Make it so Device drivers can define Raster to be one of their local
 * types. */
	#define LibRast Raster
#endif

#define VDEV_VERSION 0

/* Video device rex library control block */

typedef struct vdevice {
	Rexlib hdr; 			/* rexlib header see rexlib.h
							 * type is REX_VDRIVER version is VDEV_VERSION
							 * id_string must be NULL */

	USHORT first_rtype; 	/* first raster type allocated to this driver
								set by PJ before open */

	USHORT num_rtypes;		/* maximum number of raster types produced by this
							 * driver all raster type must be >= first_rtype
							 * and < (first_rtype + num_rtypes) Initialized
							 * in driver code Needed by host for driver open
							 * must be at least one. */

	USHORT mode_count;		/* Number of major graphics modes. and number of
							 * mode infos available, Initialized by driver */

	USHORT dev_lib_count;	/* # of functions in device library
								- Initialized by driver*/

	struct vdevice_lib *lib;	/* function library  - set by driver*/

	struct rastlib *grclib; /* generic raster library set by host */

	USHORT rast_lib_count;	/* # of functions in raster library
								- Initialized by driver*/
	SHORT reserved[15];
} Vdevice;

struct local_vdevice {
	struct local_vdevice *next;
	struct vdevice		 *device;
	};

#ifndef VDEVINFO_H
#include "vdevinfo.h"
#endif

typedef struct vdevice_lib {
/* Is our hardware attatched? may or may not be called before any
 * other calls. */
	Errcode (*detect)(Vdevice *vd);
/* Get major modes supported by device. */
	Errcode (*get_modes)(Vdevice *vd, USHORT mode, Vmode_info *pvm);
/* Get extended text description of mode. This may be a softmenu key
 * for autodesk managed drivers */
	char * (*mode_text)(Vdevice *vd, USHORT mode);
/* For a set width return max height can support */
	int (*set_max_height)(Vdevice *vd, Vmode_info *vm);
/* Open up primary screen. */
	Errcode (*open_graphics)(Vdevice *vd, LibRast *r,
							 LONG width, LONG height, USHORT mode);
/* Back to mode extant when open_graphics() called and de-allocate
 * resources used by open_graphics() */
	Errcode (*close_graphics)(Vdevice *vd);
/* Try to get secondary screen from card.  May be a non-displayable
   screen (as in the undo-buffer), or displayable if the card supports
   double-buffering.  */
	Errcode (*open_cel)(Vdevice *vd, LibRast *r,
						LONG width, LONG height, USHORT pixel_depth,
						UBYTE displayable);

/* Display a different screen (used mostly when double-buffering) */
	Errcode (*show_rast)(Vdevice *vd, LibRast *r);
} Vdevice_lib;


#ifndef REXLIB_CODE

Errcode pj__vdr_initload_open(Errcode (*loadit)(Vdevice **pvd, char *name),
						   Vdevice **pvd, char *name);

/*	REXLIB_CODE */ #endif

/* VDEV_INTERNALS */ #endif

#endif /* VDEVICE_H */
