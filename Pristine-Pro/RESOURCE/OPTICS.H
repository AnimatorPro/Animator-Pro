#ifndef OPTICS_H
#define OPTICS_H


typedef struct xyzPoint
/* Three dimensional point */
	{
	double x,y,z;			/* pixel coordinates */
	} XyzPoint;


typedef struct optPos
/* An element of an optics move.  One of these generated for each 
   Continue-Move menu press or OptContinue() poco call. */
	{
	XyzPoint move;			/* offset for straight move */
	XyzPoint spin_center;	/* center point of spin in pixels */
	XyzPoint spin_axis;		/* a line from 0,0,0 to here defines spin axis*/
	XyzPoint spin_angle;	/* spins about 3 axis.  In degrees */
	XyzPoint size_center;	/* center point of scaling in pixels */
	long xp, xq; 			/* xscaling factor.  newx = oldx*xp/xq */
	long yp, yq;			/* yscaling factor.  newy = oldy*yp/yq */
	long bp, bq;			/* both scale.  Applied after x and y scale */
	} OptPos;


typedef struct optState
/* Contains all the motion information for an optics transformation */
	{
	int pos_count;			/* Number of OptPos's */
	OptPos *pos;			/* Info on all the optics sliders. */
	int path_count;			/* Points in the optics path (0 if no path) */
	int *xpath;				/* X coordinates of path */
	int *ypath;				/* Y coordinates of path */
	char path_type;			/* one of PATH_ defines below */
	Boolean path_closed;	/* is movement path closed? */
	Boolean outlined;		/* is element outlined? */
	char element;			/* one of EL_ defines below */
	} OptState;

/* values for OptState->path_type */
#define PATH_SPLINE		0
#define PATH_POLY		1
#define PATH_SAMPLED	2
#define PATH_CLOCKED	3

/* values for OptState->element */
#define EL_SCREEN	0
#define EL_THECEL	1
#define EL_POLY 	2
#define EL_SPLINE	3
#define EL_TWEEN	4

#endif /* OPTICS_H */
