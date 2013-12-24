/*  VESA.H
*/



/******************************************************************************
*									      *
*		   Copyright (C) 1991 by Autodesk, Inc. 		      *
*									      *
*	Permission to use, copy, modify, and distribute this software and     *
*	its documentation for the purpose of creating applications for	      *
*	AutoCAD, is hereby granted in accordance with the terms of the	      *
*	License Agreement accompanying this product.			      *
*									      *
*	Autodesk makes no warranties, express or implied, as to the	      *
*	correctness of this code or any derivative works which incorporate    *
*	it.  Autodesk provides the code on an ''as-is'' basis and             *
*	explicitly disclaims any liability, express or implied, for	      *
*	errors, omissions, and other problems in the code, including	      *
*	consequential and incidental damages.				      *
*									      *
*	Use, duplication, or disclosure by the U.S.  Government is	      *
*	subject to restrictions set forth in FAR 52.227-19 (Commercial	      *
*	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)      *
*	(1) (ii) (Rights in Technical Data and Computer Software, as	      *
*	applicable.							      *
*									      *
******************************************************************************/

/*
    1/7/91  - jdb - put into ADI stream
    3/18/91 - jdb - added VALID_GRAPHICS_MODE definition to check for color,
		    graphics, extended info, and supported mode in the
		    mode attributes byte.
*/


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
	struct vdevice *dev;  /* pointer back to null device. */
} Null_hw;

typedef union null_n_bmap {
	Bmap bm;
	Null_hw nm;
} NuBm;

typedef struct VRaster {
	RASTHDR_FIELDS;
	Bmap bm;
} VRaster;

/* Make sure that driver's hardware specific raster is the right size.
 * (Following code will force a compiler syntax error if it's not.) */
struct _rast_error_check_ {
	char xx[sizeof(VRaster) == sizeof(Raster)];
};

/* This definition - LibRast - lets the compiler use your hardware specific
 * raster structure as the "Raster" parameter to device and raster library
 * function vectors without the compiler requiring lots and lots of
 * type-casts or complaining about type-mismatches. It must be defined
 * _before_ rastlib.h or vdevice.h are included. */

#define Raster VRaster

#ifndef RASTLIB_H
	#include "rastlib.h"
#endif

#ifndef VDEVICE_H
	#include "vdevice.h"
#endif

#define VESA_1_0	    (1<<16) /* VESA Version 1.0 which can't tell you
				       total memory on board */
#define VESA_1_1	    ((1<<16)+1) /* VESA Version 1.1 which can tell you
				       total memory on board */

#define BYTES_IN_64K	    65536   /* number of bytes in 64K */

#define MAXVESAMODES	    100     /* maximum number of video modes */

				    /* struct VModeInfo's modeattr bits */
#define MODE_SUPPORTED	    1
#define EXTENDED_INFO	    2
#define BIOS_OUTPUT	    4
#define COLOR_MODE	    8
#define GRAPHICS_MODE	    0x10
#define VALID_GRAPHICS_MODE 0x1b    /* supported, extended, color, graphics */

				    /* struct VModeInfo's memorymodel bits */
#define TEXT_MODE	    0
#define CGA_GRAPHICS	    1
#define HERCULES_GRAPHICS   2
#define FOUR_PLANE_PLANAR   3
#define PACKED_PIXEL	    4
#define NON_CHAIN4_256COLOR 5


/*  VESA Version 1.0 does NOT have 'TotalMemory' */
/*  but  Version 1.1 does! */
struct VESAInfo {
    char    VESASignature[4];	    /* 'VESA' */
    short   VESAVersion;	    /* VESA version number */
    long    OEMStringPtr;	    /* ptr to OEM string */
    long    Capabilities;	    /* cap. of the video environment */
    long    VideoModPtr;	    /* ptr to supported Super VGA modes */
    short   TotalMemory;	    /* # of 64K mem on board */
    char    filler[242];
};


struct VModeInfo {
    /*	mandatory information */
    short   modeattr;		    /* bits defined above */
    char    winAattr;
    char    winBattr;
    short   wingran;
    short   winsize;
    short   winAseg;
    short   winBseg;
    long    winfuncptr;
    short   pitch;
    /*	optional information -- if modeattr bit EXTENDED_INFO is set */
    short   xsize;
    short   ysize;
    char    xcharsize;
    char    ycharsize;
    char    numplanes;
    char    bitsperpixel;
    char    numbanks;
    char    memorymodel;	    /* bits defined above */
    char    banksize;
    char    numimagespages;
};


extern short		VESAModes[MAXVESAMODES];
extern struct VESAInfo	VESAInfoBlock;
extern struct VModeInfo VESAModeBlock;
extern char		VESAOEMString[81];

/*  hardware.asm
*/
void	wait_vsync(VRaster *r);
void	set_colors(VRaster *r, LONG start, LONG len, void *table);

/*  rect.asm
*/
void	set_rast(VRaster *r, Pixel color);
void	set_rect(VRaster *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);
void	set_hline(VRaster *r, Pixel color, Coor x, Coor y, Ucoor w);
void	set_vline(VRaster *r, Pixel color, Coor x, Coor y, Ucoor h);

/*  dot.asm
*/
void	put_dot(VRaster *r, Pixel color, Coor x, Coor y);
void	cput_dot(VRaster *r, Pixel color, Coor x, Coor y);
Pixel	get_dot(VRaster *r, Coor x, Coor y);
Pixel	cget_dot(VRaster *r, Coor x, Coor y);

/*  uncomp.asm
*/
void	uncc64(VRaster *r, void *cbuf);
void	uncc256(VRaster *r, void *cbuf);

/*  xor.asm
*/
void	xor_rect(VRaster *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);

/*  rectpix.asm
*/
void	put_rectpix(VRaster *r, void *buf, Coor x, Coor y, Ucoor w, Ucoor h);
void	put_hseg(VRaster *r, void *buf, Ucoor x, Ucoor y, Ucoor w);
void	put_vseg(VRaster *r, void *buf, Ucoor x, Ucoor y, Ucoor h);
void	get_rectpix(VRaster *r, void *buf, Coor x, Coor y, Ucoor w, Ucoor h);
void	get_hseg(VRaster *r, void *buf, Ucoor x, Ucoor y, Ucoor w);
void	get_vseg(VRaster *r, void *buf, Ucoor x, Ucoor y, Ucoor h);


#endif /* NULL_H */
