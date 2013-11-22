
#ifndef POLY_H
#define POLY_H

struct llpoint
	{
	struct llpoint *next;
	WORD x, y, z;
	};
typedef struct llpoint LLpoint;

#define POLYMAGIC 0x99

struct poly
	{
	int pt_count;
	LLpoint *clipped_list;
	char closed;
	UBYTE polymagic;
	};
typedef struct poly Poly;

#define WP_RPOLY 0
#define WP_STAR 1
#define WP_PETAL 2
#define WP_SPIRAL 3
#define WP_ELLIPSE 4


#endif POLY_H

