#ifndef CMAP_H
#define CMAP_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#define COLORS 256
#define RGB_MAX 256

typedef struct rgb3 {
	UBYTE r,g,b;
} Rgb3;

typedef struct colormap {
	LONG num_colors;
	Rgb3 ctab[COLORS];
} Cmap;

Errcode pj_cmap_alloc(Cmap **pcmap, LONG num_colors);
void pj_cmap_free(Cmap *cmap);
void pj_get_default_cmap(Cmap *cmap);

extern int color_dif(const Rgb3 *c1, const Rgb3 *c2);
extern void true_blend(Rgb3 *c1, Rgb3 *c2, UBYTE percent, Rgb3 *d);

#endif /* CMAP_H */
