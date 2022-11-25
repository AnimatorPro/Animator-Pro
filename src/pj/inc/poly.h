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

#ifndef XFILE_H
#include "xfile.h"
#endif

struct cliprect;
struct marqihdr;
struct pentool;
struct rectangle;
struct wndo;

typedef struct llpoint {
	struct llpoint *next;
	SHORT x, y, z;
} LLpoint;

typedef struct poly {
	SHORT pt_count;
	LLpoint *clipped_list;
	UBYTE reserved;
	UBYTE polymagic;
} Poly;

#define WP_RPOLY 0
#define WP_STAR 1
#define WP_PETAL 2
#define WP_SPIRAL 3
#define WP_ELLIPSE 4

/* draw points as polygon or curve? */
extern char curveflag;

/* Use path or curve tension cont. bias? */
extern int is_path;

extern Poly working_poly;

extern Errcode curve_tool(struct pentool *pt, struct wndo *w);
extern Errcode ovalf_tool(struct pentool *pt, struct wndo *w);
extern Errcode petlf_tool(struct pentool *pt, struct wndo *w);
extern Errcode polyf_tool(struct pentool *pt, struct wndo *w);
extern Errcode rpolyf_tool(struct pentool *pt, struct wndo *w);
extern Errcode shapef_tool(struct pentool *pt, struct wndo *w);
extern Errcode spiral_tool(struct pentool *pt, struct wndo *w);
extern Errcode starf_tool(struct pentool *pt, struct wndo *w);

/* ellipse.c */
extern Errcode
oval_loop(Poly *poly, Pixel color, int *pxrad, int *pyrad, int *ptheta);

/* poly.c */
extern void reverse_poly(Poly *p);

extern Errcode
poly_to_vertices(Poly *poly, Boolean closed, Short_xyz **pvertices);

extern void poly_ave_3d(Poly *p, Short_xyz *v);

extern int
calc_zpoly(Short_xyz *s, Short_xy *d, int count, int x, int y, int z);

extern void poly_to_3d(Poly *sp, Short_xyz *d);
extern void poly_bounds(Poly *p, struct cliprect *cr);

/* polytool.c */
extern void
poly_cline_with_render_dot(SHORT x1, SHORT y1, SHORT x2, SHORT y2, void *data);

extern void marqi_polydots(struct marqihdr *mh, Poly *poly);
extern void undo_polydots(struct marqihdr *mh, Poly *poly);
extern void marqi_poly(struct marqihdr *mh, Poly *p, Boolean closed);
extern void undo_poly(struct marqihdr *mh, Poly *p, Boolean closed);
extern void free_polypoints(Poly *poly);
extern LLpoint *poly_last_point(Poly *p);
extern Errcode render_poly(Poly *wply, Boolean filled, Boolean closed);
extern void poly_grad_dims(Poly *p, Boolean filled);
extern Errcode render_fill_poly(Poly *p);
extern Errcode finish_polyt(Boolean filled, Boolean closed);
extern Errcode maybe_finish_polyt(Boolean filled, Boolean closed);
extern LLpoint *new_poly_point(Poly *poly);
extern LLpoint *start_polyt(Poly *p);

extern void
make_poly_loop(Poly *poly, Boolean curved, Boolean closed,
		LLpoint *this, int color);

extern Errcode make_poly(Poly *p, Boolean closed);

extern int
make_sp_wpoly(Poly *poly, int x0, int y0, int rad, int theta,
		int points, int star, int sratio);

extern Errcode
polystar_loop(Poly *poly, int star, int dot_color, int dash_color,
		int x0, int y0, int *ptheta, int *prad);

extern Errcode check_poly_file(char *filename);
extern int ld_poly(XFILE *f, Poly *poly);
extern Errcode load_a_poly(char *name, Poly *poly);
extern Errcode s_poly(XFILE *f, Poly *poly);
extern int save_poly(char *name, Poly *poly);
extern LLpoint *closest_point(Poly *p, int x, int y, long *dsquared);

extern void
pp_find_next_prev(Poly *poly, LLpoint *point, LLpoint **next, LLpoint **prev);

extern void linkup_poly(Poly *p);
extern Errcode update_poly(Poly *s, Poly *d);

extern void
dotty_disp_poly(Poly *p, Boolean closed, Pixel dit_color, Pixel dot_color);

extern Errcode load_and_paste_poly(char *name);
extern Errcode edit_poly_file(char *name, char curve);
extern void edit_poly(void);
extern void edit_curve(void);
extern void offset_poly(Poly *poly, int x, int y, int z);

/* spline.c */
extern int
some_spline(Poly *poly,
		dotout_func dotout, void *dotdat,
		void (*vecout)(SHORT, SHORT, SHORT, SHORT, dotout_func, void *),
		int closed, int ir);

extern int
partial_spline(Poly *poly,
		dotout_func dotout, void *dotdat,
		void (*vecout)(SHORT, SHORT, SHORT, SHORT, dotout_func, void *),
		int closed, int ir, int moving_point_ix, int invert_seg);

extern int make_sp_poly(Poly *poly, Poly *dpoly, int closed, int ir);
extern Errcode filled_spline(Poly *poly);
extern Errcode hollow_spline(Poly *poly, Boolean closed);

/* gfx/pg*.c */
extern void find_pminmax(Poly *poly, struct rectangle *r);

extern void
fill_add_shape(Poly *poly, UBYTE *on_off_buf,
		SHORT bpr, SHORT xoff, SHORT yoff);

extern Errcode
fill_concave(Poly *poly,
		hline_func hline, void *hldat);

extern Errcode
fill_on_off(SHORT bpr, SHORT width, SHORT height,
		SHORT xoff, SHORT yoff, UBYTE *imagept,
		hline_func hline, void *hldat);

extern Errcode
fill_poly_inside(Poly *pl,
		hline_func hline, void *hldat);

extern void
hollow_polygon(Poly *poly,
		line_func lineout, void *ldat, Boolean closed);

extern Errcode
filled_polygon(Poly *poly,
		hline_func hline, void *hldat,
		line_func line, void *ldat);

#endif
