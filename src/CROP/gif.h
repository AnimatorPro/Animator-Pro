#ifndef GIF_H
#define GIF_H

#include "jimk.h"

struct GCC_PACKED gif_header
	{
	char giftype[6];
	WORD w, h;
	unsigned char colpix;	/* flags */
	unsigned char bgcolor;
	unsigned char reserved;
	};
STATIC_ASSERT(gif, sizeof(struct gif_header) == 13);

#define COLTAB	0x80
#define COLMASK 0x70
#define COLSHIFT 4
#define PIXMASK 7
#define COLPIXVGA13 (COLTAB | (5<<COLSHIFT) | 7)

struct GCC_PACKED gif_image
	{
	WORD x, y, w, h;
	unsigned char flags;
	};
STATIC_ASSERT(gif, sizeof(struct gif_image) == 9);

#define ITLV_BIT 0x40

/* Various error codes used by decoder
 * and my own routines...   It's okay
 * for you to define whatever you want,
 * as long as it's negative...  It will be
 * returned intact up the various subroutine
 * levels...
 */
#define OUT_OF_MEMORY -10
#define BAD_CODE_SIZE -20
#define READ_ERROR -1
#define WRITE_ERROR -2
#define OPEN_ERROR -3
#define CREATE_ERROR -4
#define TOO_HIGH	-5

#endif
