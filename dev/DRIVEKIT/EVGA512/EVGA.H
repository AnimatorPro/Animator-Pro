
#ifndef EVGA_H
#define EVGA_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif

typedef struct ytable
	{
	USHORT bank;
	USHORT split_at;
	long address;
	} Ytable;

/* hardware specific part of evga raster */
typedef struct evga_hw {
	LONG driver_reserved[2];
	long bpr;
	void *ytable;
} Evga_hw;

/* Union of memory raster with hardware specific raster, so can do
 * blits to/from memory without making compiler neurotic about casts. */
typedef union evga_hw_bmap {
	Bmap bm;
	Evga_hw em;
} VmBm;

/* Raster structure (something we can draw on.) */
typedef struct evgarast {
	RASTHDR_FIELDS;
	VmBm hw;
} EvgaRast;

/* This definition - LibRast - lets the compiler use your hardware specific 
 * raster structure as the "Raster" parameter to device and raster library
 * function vectors without the compiler requiring lots and lots of
 * type-casts or complaining about type-mismatches. It must be defined
 * _before_ rastlib.h or vdevice.h are included. */
#define LibRast EvgaRast

#ifndef RASTLIB_H
	#include "rastlib.h"
#endif

#ifndef VDEVICE_H
	#include "vdevice.h"
#endif


#endif /* EVGA_H */
