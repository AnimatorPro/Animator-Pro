#ifndef PALCHUNK_H
#define PALCHUNK_H

#ifndef JFILE_H
	#include "jfile.h"
#endif

#ifndef VMAGICS_H
	#include "vmagics.h"
#endif

#ifndef CMAP_H
	#include "cmap.h"
#endif

#ifdef COMMENT

typedef struct pal_chunk {
	Fat_chunk id;  /* Type is for file use, version is type of cmap data. */
	Rgb3 ctab[1]; 
} Pal_chunk;

#endif /* COMMENT */

#define PAL_RGB256_VERS 0   /* only one type for now */

Errcode pj_write_palchunk(Jfile fd, Cmap *cmap, SHORT id_type);
Errcode pj_read_palchunk(Jfile fd, Fat_chunk *id,Cmap *cmap);
Errcode pj_col_load(char *name, Cmap *cmap);
Errcode pj_col_save(char *name, Cmap *cmap);

#endif /* PALCHUNK_H */

