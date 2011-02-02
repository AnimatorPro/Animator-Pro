#ifndef POLYGON_H
#define POLYGON_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

struct llpoint
	{
	struct llpoint *next;
	SHORT x, y;
	USHORT flags;
	};
typedef struct llpoint LLpoint;

struct poly
	{
	SHORT pt_count;
	LLpoint *clipped_list;
	LLpoint *tail;
	};
typedef struct poly Poly;


typedef struct pointlist {
	LLpoint *head;
} Pointlist;

Errcode fill_poly_inside(Poly *pl, EFUNC hline, void *hldat);

#endif /* POLYGON_H */
