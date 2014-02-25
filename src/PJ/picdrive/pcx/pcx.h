#ifndef PCX_H
#define PCX_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef PICDRIVE_H
	#include "picdrive.h"
#endif

#include "xfile.h"

typedef struct pcx_header
	{
	UBYTE magic, version, encode, bitpx;
	SHORT x1,y1,x2,y2;
	SHORT cardw, cardh;
	UBYTE palette[48];
	UBYTE vmode, nplanes;
	SHORT bpl;	/* bytes per line of piccie */
	UBYTE pad[60];
	} Pcx_header;
STATIC_ASSERT(pcx, sizeof(Pcx_header) == 128);

typedef struct pcx_image_file {
	Image_file hdr;
	XFILE *file;
	Anim_info ainfo; /* info created with or opened with */
} Pcx_file;


#endif /* PCX_H */
