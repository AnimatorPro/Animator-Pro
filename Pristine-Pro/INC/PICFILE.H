#ifndef PICFILE_H
#define PICFILE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef JFILE_H
	#include "jfile.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef VMAGICS_H
	#include "vmagics.h"
#endif

/* Old style pic header, Animator 1.0 compatible */
typedef struct opic_header {
	USHORT type;  	/* type == OPIC_MAGIC */
	RECT_FIELDS; 	/* width, height, x, y */
	char d;  		/* pixel depth */
	char compress; 	/* unused */
	LONG csize;    	/* w * h */
	char reserved[16];
} Opic_header;



typedef struct pic_header {
	Chunk_id id;      	/* type is PIC_MAGIC */
	RECT_FIELDS;   	  	/* width; height; x; y; size and positioning info */
	LONG user_id;      	/* arbitrary id number for user */
	UBYTE depth;		/* pixel depth needed to represent pic */
	UBYTE reserved[45]; /* for a total of 64 bytes */
} Pic_header;

/* chunk types after header */

enum pic_chunks {
	PIC_CMAP = 0,		/* palette chunk 0-255 Fat_chunk + rgb bytes */
	PIC_BYTEPIXELS = 1,	/* uncompressed pixels in Bytemap format */
	PIC_BITPIXELS = 2,	/* uncompressed pixels in Bitmap format */
};


/* "pic" file io calls */

Errcode pj_read_pichead(Jfile f,Pic_header *pic);
Errcode pj_read_picbody(Jfile f,Pic_header *pic,Raster *cel, Cmap *cmap);

Errcode load_pic(char *name,Rcel *rcel,LONG check_id, Boolean load_colors);
Errcode save_pic(char *name,Rcel *screen,LONG id, Boolean save_colors);

#endif /* PICFILE_H */
