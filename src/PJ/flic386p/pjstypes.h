/*****************************************************************************
 * PJSTYPES_H - Standard pj datatypes.
 *
 *	Items defined here are extracted from pj's internal STDTYPES.H file,
 *	and several other pj internal header files.
 ****************************************************************************/

#ifndef PJSTYPES_H
#define PJSTYPES_H

/*----------------------------------------------------------------------------
 * set up some typical C things...
 *--------------------------------------------------------------------------*/

#include <stddef.h> 	/* system standard definitions */
#include <limits.h> 	/* compiler dependent data type sizes */

#ifndef NULL
	#define NULL ((void *)0)
#endif

#ifndef TRUE
	#define TRUE	1
	#define FALSE	0
#endif

#ifndef Success
	#define Success 0
#else
	#if Success != 0
		#error Fatal: You have defined "Success" to a non-zero value!
	#endif
#endif

/*----------------------------------------------------------------------------
 * pj's atomic types.
 *	The mixed case names are special pj types - change at your own risk!
 *--------------------------------------------------------------------------*/

typedef unsigned char	Pixel;	 /* type used in pixel buffers by get_hseg etc */
typedef unsigned long	Ucoor;	 /* A pixel width/height probably */
typedef long			Coor;	 /* Most likely pixel x/y offset */

typedef int 			Boolean; /* your typical true/false type */
typedef int 			Errcode; /* >= 0 if ok, < 0 otherwise see pjecodes.h */

/*----------------------------------------------------------------------------
 * for Watcom C only, the following pragma names an alias for calling
 * and return value standards to implement the -3s/HighC standard.	We
 * declare (in other header files) all our documented entry points with the
 * FLICLIB3S name, which allows our functions to be called from a program
 * compiled using the (normal) -3r Watcom standard of parms-in-regs.
 *--------------------------------------------------------------------------*/

#ifdef __WATCOMC__
	#pragma aux FLICLIB3S	"*"                      \
							parm caller []			 \
							value no8087			 \
							modify [eax ecx edx gs];
#endif

#endif /* PJSTYPES_H */
