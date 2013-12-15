
#ifndef INKAID_H
#define INKAID_H

#ifndef RCEL_H
	#include "rcel.h"
#endif

/* structure hung onto ink for run-time support of inks */
typedef struct aa_ink_data {
	Pixel ccolor;
	Pixel tcolor;
	Rcel *screen;		/* drawing screen */
	Rcel *undo;			/* undo screen */
	Rcel *cel;			/* cel */
	Rcel *alt;			/* alt screen */
	SHORT ccount;		/* # of colors in system */
	SHORT rmax,gmax,bmax;	/* max values of rgb components */
	void (*true_blend)(Rgb3 *dest, Rgb3 *source, UBYTE percent, Rgb3 *result);
	int (*closestc)(Rgb3 *true_color, Rgb3 *cmap, int ccount);
	Errcode (*make_bhash)(struct ink *ink);
	void (*free_bhash)(struct ink *ink);
	int (*bclosest_col)(Rgb3 *rgb,int count,SHORT dither);
} Aa_ink_data;
extern Aa_ink_data ink_aid;

/* structure passed to ink on program entry so it can configure itself... */
typedef struct ink_groups {
struct button *dither_group;	/* pointer to dither button */
struct button *strength_group;	/* pointer to strength slider button */
struct button *dstrength_group; /* pointer to dither & ink strength */
} Ink_groups;

#endif /* INKAID_H */
