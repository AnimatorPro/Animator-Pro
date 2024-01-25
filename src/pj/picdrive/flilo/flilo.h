#ifndef FLILO_H
#define FLILO_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef PICDRIVE_H
	#include "picdrive.h"
#endif

#ifndef XFILE_H
#include "xfile.h"
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
	int32_t size;
	USHORT type;  /* = FLIH_MAGIC or FLIX_MAGIC */
	USHORT frame_count;
	USHORT width;
	USHORT height;
	USHORT bits_a_pixel;
	SHORT flags;
	SHORT speed;
	char reserved[110];
} Fli_head;
STATIC_ASSERT(flilo, sizeof(Fli_head) == 128);

#define FLI_FINISHED 1
#define FLI_LOOPED	2

typedef struct fli_frame {
	int32_t size;
	USHORT type;		/* = 0xf1fa FLIF_MAGIC */
	SHORT chunks;
	char pad[8];
} Fli_frame;
STATIC_ASSERT(flilo, sizeof(Fli_frame) == 16);

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

typedef struct GCC_PACKED fli_chunk {
	int32_t size;
	SHORT type;
} Fli_chunk;
STATIC_ASSERT(flilo, sizeof(Fli_chunk) == 6);

#define EMPTY_DCOMP 8  /* sizeof of a FLI_SKIP chunk with no change */

/* a fli file object used with fli library routines */

typedef struct flifile {
	Image_file ifile;
	Fli_head hdr;  /* the fli hdr for the fli */
	XFILE *xf;     /* file handle for this fli */
} Flifile;

struct raster;
struct rcel;

extern void *
flow_brun_rect(struct raster *r, void *cbuf,
		SHORT x, SHORT y, USHORT width, USHORT height);

extern LONG
flow_comp_cel(void *comp_buf,
		struct rcel *last_screen, struct rcel *this_screen, SHORT type);

#endif
