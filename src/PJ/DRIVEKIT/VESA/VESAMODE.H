/*****************************************************************************
 * VESA.H - Header file for C-language driver routines.
 ****************************************************************************/

/******************************************************************************
*
*		   Copyright (C) 1991 by Autodesk, Inc.
*
*	Permission to use, copy, modify, and distribute this software and
*	its documentation for the purpose of creating applications for
*	AutoCAD, is hereby granted in accordance with the terms of the
*	License Agreement accompanying this product.
*
*	Autodesk makes no warrantees, express or implied, as to the
*	correctness of this code or any derivative works which incorporate
*	it.  Autodesk provides the code on an ''as-is'' basis and
*	explicitly disclaims any liability, express or implied, for
*	errors, omissions, and other problems in the code, including
*	consequential and incidental damages.
*
*	Use, duplication, or disclosure by the U.S.  Government is
*	subject to restrictions set forth in FAR 52.227-19 (Commercial
*	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)
*	(1) (ii) (Rights in Technical Data and Computer Software, as
*	applicable.
*
******************************************************************************/

/*
	01/07/91  - jdb - put into ADI stream.
	03/27/91  - ian - major rewrite of entire driver.
	04/17/91  - jdb - def'd Raster to VesaRast before rastlib.h inclusion
			- made necessary protos below match rastlib.h
			- cast all 'Bytemap *' to 'Raster *' to match rastlib.h
			- (above was necessary for High C strict type-casting)
*/

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef DRVCOMN_H
	#include "drvcomn.h"
#endif

/*----------------------------------------------------------------------------
 * stuff specific to the VESA interface...
 *
 *	these things are defined by the VESA standard.
 *--------------------------------------------------------------------------*/

#define VESA_1_0		0x0100		/* VESA Version 1.0 which can't tell you
									   total memory on board */
#define VESA_1_1		0x0101		/* VESA Version 1.1 which can tell you
									   total memory on board */

#define BYTES_IN_64K		65536L	/* number of bytes in 64K */

#define MAXpj_vesa_MODES		100 /* maximum number of video modes */

					/* struct VModeInfo's modeattr bits */
#define MODE_SUPPORTED		0x01
#define EXTENDED_INFO		0x02
#define BIOS_OUTPUT 	0x04
#define COLOR_MODE			0x08
#define GRAPHICS_MODE		0x10
					/* struct VModeInfo's memorymodel values */
#define TEXT_MODE			0
#define CGA_GRAPHICS		1
#define HERCULES_GRAPHICS	2
#define FOUR_PLANE_PLANAR	3
#define PACKED_PIXEL		4
#define NON_CHAIN4_256COLOR 5


/*	VESA Version 1.0 does NOT have 'TotalMemory' */
/*	but  Version 1.1 does! */

typedef struct vesa_bios_info {
	char	VESASignature[4];	/* 'VESA' */
	USHORT	VESAVersion;		/* VESA version number */
	long	OEMStringPtr;		/* ptr to OEM string */
	long	Capabilities;		/* cap. of the video environment */
	long	VideoModPtr;		/* ptr to supported Super VGA modes */
	USHORT	TotalMemory;		/* # of 64K mem on board */
	UBYTE	filler[242];
	} VBEInfo;


typedef struct vmodeinfo {
	/*	mandatory information */
	USHORT	modeattr;			/* bits defined above */
	UBYTE	winAattr;
	UBYTE	winBattr;
	USHORT	wingran;
	USHORT	winsize;
	USHORT	winAseg;
	USHORT	winBseg;
	long	winfuncptr;
	USHORT	pitch;
	/*	optional information -- if modeattr bit EXTENDED_INFO is set */
	USHORT	xsize;
	USHORT	ysize;
	UBYTE	xcharsize;
	UBYTE	ycharsize;
	UBYTE	numplanes;
	UBYTE	bitsperpixel;
	UBYTE	numbanks;
	UBYTE	memorymodel;		/* bits defined above */
	UBYTE	banksize;
	UBYTE	numimagespages;
	} VModeInfo;

/*****************************************************************************
 * protos for things in vesaintf.asm that must be visible to C code...
 ****************************************************************************/

extern Errcode	 pj_vesa_detect(void);
extern Errcode	 pj_vesa_setmode(SMInfo *pmode);
extern void 	 pj_vesa_clrmode(long old_mode_data);
extern void 	 pj_vesa_free_dosbuf(void);
extern VBEInfo	 *pj_vesa_get_bios_info(void);
extern VModeInfo *pj_vesa_get_mode_info(int mode);

/* end of VESA.H */
