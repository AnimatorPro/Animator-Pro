#ifndef POLY_H
#define POLY_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif
#ifndef VMAGICS_H
	#include "vmagics.h"
#endif
#ifndef VERTICES_H
	#include "vertices.h"
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

#define WP_RPOLY 0
#define WP_STAR 1
#define WP_PETAL 2
#define WP_SPIRAL 3
#define WP_ELLIPSE 4

Errcode filled_polygon();

LLpoint *poly_add_point(void);
LLpoint *new_poly_point();
LLpoint *closest_point();
LLpoint *poly_last_point(Poly *p);

void pp_find_next_prev();

Errcode clone_ppoints();
Errcode update_poly();
Errcode load_a_poly();
Errcode poly_to_vertices();
int calc_zpoly();

void dotty_disp_poly();
void poly_to_3d();
void poly_ave_3d();
void otoobig();

/* Some routines for filling shapes. */
void fill_add_shape(Poly *poly, UBYTE *on_off_buf
, 	SHORT bpr, SHORT xoff, SHORT yoff);
Errcode fill_concave(Poly *poly, EFUNC hline, void *hldat);
Errcode fill_on_off(SHORT bpr, SHORT width, SHORT height, 
	SHORT xoff, SHORT yoff, UBYTE *imagept, EFUNC hline, void *hldat);
#endif /* POLY_H */
