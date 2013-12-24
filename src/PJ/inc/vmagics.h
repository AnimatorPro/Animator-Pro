#ifndef VMAGICS_H
#define VMAGICS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/* file type magics these should all be unique !! */

/* magics found as first short word in file */

#define A3D_MAGIC 	    0x1A3FU
#define VCFG_MAGIC 		0x2894U	/* magic number for configuration */
#define OPIC_MAGIC      0x9119U /* animator 1.0 picture files */
#define TWEEN_MAGIC		0x1995U

/* magics found in "Chunk_id" as 3rd short word in file */

#define PIC_MAGIC 		0x9500U     /* animator temp file picture format */
#define VSETFILE_MAGIC  0x9701U  	/* settings file magic */
#define FLIX_MAGIC      0x9721U     /* Indexed temp flx Magic */
#define FLIH_MAGIC      0xAF11U     /* low res animator FLI Magic */
#define FLIHR_MAGIC 	0xAF12U     /* Hi res FLI magic */
#define FOVL_MAGIC 		0xAF1FU     /* overlay temp file magic */
#define CMAP_MAGIC		0xB123U     /* color map file */
#define MAC_MAGIC 		0xB200U		/* macro record file */


#define INVALID_CHUNK_TYPE (-1) /* the type -1 is reserved for the parser */

#define CHUNKID_FIELDS \
	LONG size;\
	USHORT type

typedef struct chunk_id {
	CHUNKID_FIELDS;
} Chunk_id;

typedef struct fat_chunk {
	CHUNKID_FIELDS;
	USHORT version;
} Fat_chunk;


/* This oddball is a byte at location 7 of a poly file */
#define POLYMAGIC 0x99

#endif /* VMAGICS_H */
