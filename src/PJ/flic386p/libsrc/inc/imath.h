
#ifndef IMATH_H
#define IMATH_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#define SIXTY4K ((1L<<16)-16)
#define BIG_SHORT ((1L<<15)-1)

/* 'normal' perspective */
#define GROUND_Z 512	

/* Used as value for 1 by 2.14 fixed point routines */
#define SCALE_ONE 	(1<<14)

/* 360 degrees according to my trig tables */
#define TWOPI 1024

int imax(int a, int b); 			/* maximum of a,b */
int iabs(int a);					/* integer absolute value */
int ilcm(int a, int b);				/* least common multiple */
void calc_sieve(char *sieve, int max);	/* make sieve of primes */
int sqr_root(long i);				/* square root */
int calc_distance(short x1,short y1,short x2,short y2);  /* 2D distance */
void partial_rot(SHORT theta, SHORT *xx, SHORT *yy);

/* Give our fixed point multiply routine a better name */
#define scale_mult(a,scale) itmult(a,scale)		/* 2.14 fixed point multiply */

#endif /* IMATH_H */
