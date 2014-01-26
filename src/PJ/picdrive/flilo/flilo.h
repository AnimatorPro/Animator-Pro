#ifndef FLILO_H
#define FLILO_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef JFILE_H
	#include "jfile.h"
#endif

#ifndef PICDRIVE_H
	#include "picdrive.h"
#endif

#define CHUNKID_FIELDS \
	LONG size;\
	USHORT type

typedef struct chunk_id {
	CHUNKID_FIELDS;
} Chunk_id;


#define CBUF_SIZE ((320*200)+400+(COLORS*3)+256+16+sizeof(Raster))
#define COLORS 256
#define MAXFRAMES (4*1000)	/* Max number of frames... */

/* 1.0 low res Magic */
#define FLIH_MAGIC 0xaf11	

/* Frame Magic */
#define FCID_FRAME 0xf1fa

typedef struct fli_head {
	long size;
	USHORT type;  /* = FLIH_MAGIC or FLIX_MAGIC */
	USHORT frame_count;
	USHORT width;
	USHORT height;
	USHORT bits_a_pixel;
	SHORT flags;
	SHORT speed;
	char reserved[110];
} Fli_head;

#define FLI_FINISHED 1
#define FLI_LOOPED	2

typedef struct fli_frame {
	long size;
	USHORT type;		/* = 0xf1fa FLIF_MAGIC */
	SHORT chunks;
	char pad[8];
} Fli_frame;


#define FLI_COL 0
#define FLI_WRUN 1
#define FLI_WSKIP 2
#define FLI_WSKIP2 3
#define FLI_COL2 4
#define FLI_WSKIPRUN 5
#define FLI_BSKIP 6
#define FLI_BSKIPRUN 7
#define FLI_BSC 8
#define FLI_SBSC 9
#define FLI_SBSRSC 10
#define FLI_COLOR 11
#define FLI_LC	12
#define FLI_COLOR_0 13
#define FLI_ICOLORS 14
#define FLI_BRUN 15
#define FLI_COPY 16


typedef struct fli_chunk {
	long size;
	SHORT type;
} Fli_chunk;


#define EMPTY_DCOMP 8  /* sizeof of a FLI_SKIP chunk with no change */

/* a fli file object used with fli library routines */

typedef struct flifile {
	Image_file ifile;
	Fli_head hdr;  /* the fli hdr for the fli */
	Jfile fd;      /* file handle for this fli */
} Flifile;

struct raster;

extern void *
flow_brun_rect(struct raster *r, void *cbuf,
		SHORT x, SHORT y, USHORT width, USHORT height);

#endif /* FLILO_H */
