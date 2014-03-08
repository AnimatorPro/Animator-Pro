#ifndef PALCHUNK_H
#define PALCHUNK_H

#ifndef JFILE_H
	#include "jfile.h"
#endif

#ifndef VMAGICS_H
	#include "vmagics.h"
#endif

struct cmap;

#define PAL_RGB256_VERS 0   /* only one type for now */

extern Errcode pj_write_palchunk(Jfile fd, struct cmap *cmap, SHORT id_type);
extern Errcode pj_read_palchunk(Jfile fd, Fat_chunk *id, struct cmap *cmap);
extern Errcode pj_col_load(char *name, struct cmap *cmap);
extern Errcode pj_col_save(char *name, struct cmap *cmap);

#endif
