#ifndef VERTICES_H
#define VERTICES_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/* 2d and 3d SHORT vertices */

typedef struct byte_xy {
	BYTE x,y;
} Byte_xy;

typedef struct short_xy {
	SHORT x,y;
} Short_xy;

typedef struct short_xyz {
	SHORT x,y,z;
} Short_xyz;

#endif /* VERTICES_H */
