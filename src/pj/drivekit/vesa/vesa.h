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

/* The #define Raster below lets the compiler use your hardware specific
 * raster structure as the "Raster" parameter to device and raster library
 * function vectors without the compiler requiring lots and lots of
 * type-casts or complaining about type-mismatches. It must be defined
 * _before_ rastlib.h or vdevice.h are included. */

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif

struct rastlib; /* just enough to use in forward-ref grclib declaration... */

typedef struct vesarast {
	RASTHDR_FIELDS;
	struct
	 rastlib *grclib;
	long	 filler[3];
} VesaRast;

#define Raster VesaRast

#ifndef RASTLIB_H
	#include "rastlib.h"
#endif

#ifndef VDEVICE_H
	#include "vdevice.h"
#endif

/* end of VESA.H */
