#ifndef INKAID_H
#define INKAID_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct button;
struct ink;
struct rcel;
struct rgb3;

/* structure hung onto ink for run-time support of inks */
typedef struct aa_ink_data {
	Pixel ccolor;
	Pixel tcolor;
	struct rcel *screen;    /* drawing screen */
	struct rcel *undo;      /* undo screen */
	struct rcel *cel;       /* cel */
	struct rcel *alt;       /* alt screen */
	SHORT ccount;           /* # of colors in system */
	SHORT rmax, gmax, bmax; /* max values of rgb components */
	void (*true_blend)(Rgb3 *dest, Rgb3 *source,
			UBYTE percent, struct rgb3 *result);
	int (*closestc)(const struct rgb3 *true_color, const struct rgb3 *cmap,
			int ccount);
	Errcode (*make_bhash)(struct ink *ink);
	void (*free_bhash)(struct ink *ink);
	int (*bclosest_col)(const struct rgb3 *rgb, int count, SHORT dither);
} Aa_ink_data;

/* structure passed to ink on program entry so it can configure itself... */
typedef struct ink_groups {
	struct button *dither_group;    /* pointer to dither button */
	struct button *strength_group;  /* pointer to strength slider button */
	struct button *dstrength_group; /* pointer to dither & ink strength */
} Ink_groups;

extern Aa_ink_data ink_aid;

#endif
