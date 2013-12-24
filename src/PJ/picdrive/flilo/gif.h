#ifndef GIF_H
#define GIF_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef STDIO_H
	#include <stdio.h>
#endif

#ifndef PICDRIVE_H
	#include "picdrive.h"
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

typedef struct gif_image_file {
	Image_file hdr;
	FILE *file;
	Anim_info ainfo; /* info created with or opened with */
} Gif_file;


#endif /* GIF_H */
