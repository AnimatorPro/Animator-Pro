/* Fixpoint.h - a crude but effective set of basic fixed point routines.
 * The point is defines as FIXPOINT_SHIFT - currently 8, allowing
 * a 24.8 representation - which is good for spliny things that are
 * ultimately drawn on the screen. */

#ifndef FIXPOINT_H
#define FIXPOINT_H

typedef long fixpoint;

#define FIXPOINT_SHIFT 8
#define FIXPOINT_ONE (1<<8)

	/* Plus */
#define FP(a,b) ((a)+(b))
	/* Minus */
#define FM(a,b) ((a)-(b))
	/* Times */
#define FT(a,b) (((a)*(b))>>FIXPOINT_SHIFT)
	/* Divide */
/* #define FD(a,b) (((a)<<FIXPOINT_SHIFT)/(b)) */
#define FD(a,b) fixpoint_div(a,b)

fixpoint fixpoint_div(fixpoint a, fixpoint b);

	/* Convert from int to fixpoint */
#define FVAL(a) ((long)(a)<<FIXPOINT_SHIFT)
	/* Convert from fixpoint to int. */
#define FINT(a) ((int)((a)>>FIXPOINT_SHIFT))

#ifdef LATER
/* Use these if want to revert to floating point. */
typedef float fixpoint;
#define FP(a,b) ((a)+(b))
#define FM(a,b) ((a)-(b))
#define FT(a,b) ((a)*(b))
#define FD(a,b) ((a)/(b))
#define FVAL(a) ((float)(a))
#define FINT(a) ((int)(a))
#endif /* LATER */


#endif /* FIXPOINT_H */
