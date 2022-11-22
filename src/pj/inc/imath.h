
#ifndef IMATH_H
#define IMATH_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

struct ado_setting;

#define SIXTY4K ((1L<<16)-16)
#define BIG_SHORT ((1L<<15)-1)

/* 'normal' perspective */
#define GROUND_Z 512	

/* Used as value for 1 by 2.14 fixed point routines */
#define SCALE_ONE 	(1<<14)

/* 360 degrees according to my trig tables */
#define TWOPI 1024

int ilcm(int a, int b);				/* least common multiple */
int sqr_root(long i);				/* square root */
int calc_distance(short x1,short y1,short x2,short y2);  /* 2D distance */
void partial_rot(SHORT theta, SHORT *xx, SHORT *yy);

extern int intabs(int a);
extern int intmax(int a, int b);
extern int intmin(int a, int b);

extern int itmult(SHORT trig, SHORT x);
extern void polar(short theta, short rad, short *xy);

extern int isin(short t);
extern int icos(short t);
extern int isincos(int angle, int *cos);
extern SHORT arctan(int x, int y);
extern int arcnorm(int t);

extern int rscale_by(int x, int p, int q);
extern int sscale_by(int x, int p, int q);
extern int pj_uscale_by(USHORT x, USHORT p, USHORT q);

extern void find_conjugates(struct ado_setting *function);

#endif /* IMATH_H */
