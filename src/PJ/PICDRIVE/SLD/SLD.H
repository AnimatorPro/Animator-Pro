#ifndef SLD_H
#define SLD_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

struct llpoint
	{
	struct llpoint *next;
	SHORT x, y, z;
	};
typedef struct llpoint LLpoint;

struct poly
	{
	SHORT pt_count;
	LLpoint *clipped_list;
	UBYTE reserved;
	UBYTE polymagic;
	};
typedef struct poly Poly;

#endif /* SLD_H */
