#ifndef GIF_H
#define GIF_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

struct gif_header
	{
	signed char giftype[6];
	SHORT w,h;
	unsigned char colpix;	/* flags */
	unsigned char bgcolor;
	unsigned char reserved;
	};

#define COLTAB	0x80
#define COLMASK 0x70
#define COLSHIFT 4
#define PIXMASK 7
#define COLPIXVGA13 (COLTAB | (5<<COLSHIFT) | 7)

struct gif_image
	{
	SHORT x,y,w,h;
	unsigned char flags;
	};
#define ITLV_BIT 0x40

/* Various error codes used by decoder
 * and my own routines...   It's okay
 * for you to define whatever you want,
 * as long as it's negative...  It will be
 * returned intact up the various subroutine
 * levels...
 */

#endif /* GIF_H */
