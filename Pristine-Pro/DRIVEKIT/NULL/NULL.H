/* null.h - header file for the null driver */
#ifndef NULL_H
#define NULL_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif

typedef struct null_hw {
	long gets;			/* the number of times get_dot is called */
	long puts;			/* the number of times put_dot is called */
	long colors;		/* the number of times set_colors is called */
	struct vdevice *dev;  /* pointer back to null device. */
} Null_hw;

typedef union null_n_bmap {
	Bmap bm;
	Null_hw nm;
} NuBm;

typedef struct nullrast {
	RASTHDR_FIELDS;
	NuBm hw;
} NullRast;

/* Make sure that driver's hardware specific raster is the right size.
 * (Following code will force a compiler syntax error if it's not.) */
struct _rast_error_check_ {
	char xx[sizeof(NullRast) == sizeof(Raster)];
};

/* This definition - LibRast - lets the compiler use your hardware specific 
 * raster structure as the "Raster" parameter to device and raster library
 * function vectors without the compiler requiring lots and lots of
 * type-casts or complaining about type-mismatches. It must be defined
 * _before_ rastlib.h or vdevice.h are included. */
#define LibRast NullRast

#ifndef RASTLIB_H
	#include "rastlib.h"
#endif

#ifndef VDEVICE_H
	#include "vdevice.h"
#endif

#endif /* NULL_H */
