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

struct rectangle;

#define WP_RPOLY 0
#define WP_STAR 1
#define WP_PETAL 2
#define WP_SPIRAL 3
#define WP_ELLIPSE 4

extern void
poly_cline_with_render_dot(SHORT x1, SHORT y1, SHORT x2, SHORT y2, void *data);

extern void
hollow_polygon(Poly *poly,
		line_func lineout, void *ldat, Boolean closed);

extern Errcode
filled_polygon(Poly *poly,
		hline_func hline, void *hldat,
		line_func line, void *ldat);

extern LLpoint *poly_add_point(void);
extern LLpoint *new_poly_point(Poly *p);
extern LLpoint *closest_point(Poly *p, int x, int y, long *dsquared);
extern LLpoint *poly_last_point(Poly *p);

extern void
pp_find_next_prev(Poly *poly, LLpoint *point, LLpoint **next, LLpoint **prev);

extern Errcode clone_ppoints(Poly *s, Poly *d);
extern Errcode update_poly(Poly *s, Poly *d);
extern Errcode load_a_poly(char *name, Poly *poly);

extern Errcode
poly_to_vertices(Poly *poly, Boolean closed, Short_xyz **pvertices);

extern int
calc_zpoly(Short_xyz *s, Short_xy *d, int count, int x, int y, int z);

extern void
dotty_disp_poly(Poly *p, Boolean closed, Pixel dit_color, Pixel dot_color);

extern void poly_to_3d(Poly *sp, Short_xyz *d);
extern void poly_ave_3d(Poly *p, Short_xyz *v);

/* Some routines for filling shapes. */
extern void find_pminmax(Poly *poly, struct rectangle *r);
void fill_add_shape(Poly *poly, UBYTE *on_off_buf
, 	SHORT bpr, SHORT xoff, SHORT yoff);
extern Errcode fill_poly_inside(Poly *pl, hline_func hline, void *hldat);
extern Errcode fill_concave(Poly *poly, hline_func hline, void *hldat);
Errcode fill_on_off(SHORT bpr, SHORT width, SHORT height, 
	SHORT xoff, SHORT yoff, UBYTE *imagept, hline_func hline, void *hldat);

#endif /* POLY_H */
