#ifndef PALCHUNK_H
#define PALCHUNK_H

#ifndef VMAGICS_H
	#include "vmagics.h"
#endif

#ifndef XFILE_H
#include "xfile.h"
#endif

struct cmap;

#define PAL_RGB256_VERS 0   /* only one type for now */

extern Errcode pj_write_palchunk(XFILE *xf, struct cmap *cmap, SHORT id_type);
extern Errcode pj_read_palchunk(XFILE *xf, Fat_chunk *id, struct cmap *cmap);
extern Errcode pj_col_load(const char *name, struct cmap *cmap);
extern Errcode pj_col_save(const char *name, struct cmap *cmap);

#endif
