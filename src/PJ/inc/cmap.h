#ifndef CMAP_H
#define CMAP_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

struct button;
struct menuhdr;
struct wscreen;

#define COLORS 256
#define RGB_MAX 256

typedef struct rgb3 {
	UBYTE r,g,b;
} Rgb3;
STATIC_ASSERT(cmap, sizeof(Rgb3) == 3);

typedef struct cmap {
	LONG num_colors;
	Rgb3 ctab[COLORS];
} Cmap;

extern Cmap *pj_default_cmap;
extern struct button pal_bun_sel;
extern struct button pal_cco_sel;
extern struct button pal_pal_sel;
extern struct button pal_spe_sel;
extern struct menuhdr palette_menu;

extern Errcode pj_cmap_alloc(Cmap **pcmap, LONG num_colors);
extern void pj_cmap_free(Cmap *cmap);
extern void pj_get_default_cmap(Cmap *cmap);

extern ULONG cmap_crcsum(Cmap *cmap);
extern Boolean cmaps_same(Cmap *s1, Cmap *s2);
extern Cmap *clone_cmap(Cmap *toclone);
extern void pj_cmap_load(void *raster, Cmap *cmap);
extern void pj_cmap_copy(Cmap *s, Cmap *d);
extern int compromise_cmap(Cmap *s1, Cmap *s2, Cmap *d);
extern void pj_shift_cmap(const UBYTE *src, UBYTE *dst, unsigned int n);
extern void swap_cmaps(Cmap *a, Cmap *b);

extern Boolean in_ctable(Rgb3 *rgb, Rgb3 *ctab, int count);
extern void get_color_rgb(USHORT cnum, Cmap *cmap, Rgb3 *rgb);
extern void set_color_rgb(Rgb3 *rgb, USHORT cnum, Cmap *cmap);
extern void stuff_cmap(Cmap *cmap, Rgb3 *color);

extern int color_dif(const Rgb3 *c1, const Rgb3 *c2);
extern int closestc(const Rgb3 *rgb, const Rgb3 *cmap, int count);

extern int
closestc_excl(Rgb3 *rgb, Rgb3 *ctab, int ccount, UBYTE *ignore, int icount);

extern void true_blend(Rgb3 *c1, Rgb3 *c2, UBYTE percent, Rgb3 *d);

extern LONG _h_lsrgb_value(LONG n1, LONG n2, SHORT hue);
extern void rgb_to_hls(SHORT r, SHORT g, SHORT b, SHORT *h, SHORT *l, SHORT *s);
extern void hls_to_rgb(SHORT *r, SHORT *g, SHORT *b, SHORT h, SHORT l, SHORT s);

extern void pack_ctable(Rgb3 *source, LONG scount, Rgb3 *dest, int dcount);

/* cfit.c */
extern void fitting_ctable(Rgb3 *scm, Rgb3 *dcm, UBYTE *cnums);
extern void nz_fitting_ctable(Rgb3 *scm, Rgb3 *dcm, UBYTE *cnums);
extern void get_cmap_blend(int bscale, Cmap *cmapa, Cmap *cmapb, Cmap *dcmap);
extern void make_one_color_ctable(Pixel *ctable, SHORT tcolor);

/* vpsubs.c */
extern void see_cmap(void);
extern int interp_range(int c1, int c2, int i, int divi);

#endif
