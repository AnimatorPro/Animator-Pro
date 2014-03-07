#ifndef A3D_H
#define A3D_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#define ADO_SPIN 0
#define ADO_SIZE 1
#define ADO_MOVE 2
#define ADO_PATH 3
#define SPIN_CENTER 0
#define SPIN_AXIS 1
#define SPIN_TURNS 2
#define PATH_SPLINE 0
#define PATH_POLY 1
#define PATH_SAMPLED 2
#define PATH_CLOCKED 3

#define OPS_SCREEN	0
#define OPS_THECEL	1
#define OPS_POLY 	2
#define OPS_SPLINE	3
#define OPS_TWEEN   4

extern void a3d_check_el(Boolean *no_poly, Boolean *no_tween);

#endif
