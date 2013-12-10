#ifndef POLY_H
#define POLY_H

#include "jimk.h"

struct llpoint
	{
	struct llpoint *next;
	WORD x, y, z;
	};
typedef struct llpoint LLpoint;

#define POLYMAGIC 0x99
#define SIZEOF_POLY 8

struct poly
	{
	WORD pt_count;
	LLpoint *clipped_list;
	char closed;
	UBYTE polymagic;
	};
typedef struct poly Poly;

#if defined(__TURBOC__)
STATIC_ASSERT(poly, sizeof(struct poly) == SIZEOF_POLY);
#endif /* __TURBOC__ */

#define WP_RPOLY 0
#define WP_STAR 1
#define WP_PETAL 2
#define WP_SPIRAL 3
#define WP_ELLIPSE 4

#endif
