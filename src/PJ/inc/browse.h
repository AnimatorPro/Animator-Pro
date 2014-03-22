#ifndef BROWSE_H
#define BROWSE_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct fli_frame;
struct raster;
struct rectangle;

/* browse.c */
extern void go_browse(void);
extern Errcode go_browse_cels(void);

/* pstamp.c */
extern Boolean pj_frame_has_pstamp(struct fli_frame *frame);

extern Errcode
postage_stamp(struct raster *r, char *name,
		SHORT x, SHORT y, USHORT width, USHORT height,
		struct rectangle *actual);

#endif
