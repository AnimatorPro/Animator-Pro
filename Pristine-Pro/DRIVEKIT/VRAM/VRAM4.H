#ifndef VRAM_H
#define VRAM_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif

typedef struct ytable {
	USHORT bank;
	USHORT split_at;
	long address;
} Ytable;

typedef struct vram_hw {
	USHORT	filler1;
	USHORT	rastnum;
	USHORT	xoff;
	USHORT	yoff;
	long 	bpr;
	void 	*ytable;
} Vram_hw;

typedef union vram_hw_bmap {
	Bmap bm;
	Vram_hw vm;
} VmBm;

typedef struct vramrast {
	RASTHDR_FIELDS;
	VmBm hw;
} VramRast;

/* This definition - LibRast - lets the compiler use your hardware specific 
 * raster structure as the "Raster" parameter to device and raster library
 * function vectors without the compiler requiring lots and lots of
 * type-casts or complaining about type-mismatches. It must be defined
 * _before_ rastlib.h or vdevice.h are included. */
#define LibRast VramRast

/* Make sure that driver's hardware specific raster is the right size.
 * (Following code will force a compiler syntax error if it's not.) */
struct _rast_error_check_ {
	char xx[sizeof(LibRast) == sizeof(Raster)];
};

#ifndef RASTLIB_H
	#include "rastlib.h"
#endif

#ifndef VDEVICE_H
	#include "vdevice.h"
#endif


#endif /* VRAM_H */
