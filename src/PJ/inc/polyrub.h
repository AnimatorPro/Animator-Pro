#ifndef POLYRUB_H
#define POLYRUB_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct marqihdr;
struct poly;

typedef struct mpl_data {
	int pcount;
	Poly **polys;
	Pixel *dit_colors;
	Pixel *dot_colors;
} Mpl_data;

typedef struct mpl_2p {
	Mpl_data mpl;
	Poly *polys[2];
	Pixel dit_colors[2];
	Pixel dot_colors[2];
} Mpl_2p;

extern void
rub_poly_points(Poly *p, Boolean closed,
		Pixel dit_color, Pixel dot_color, Pixel pt_color, Pixel pt1_color);

extern void move_poly_points(struct poly *poly, Boolean closed);
extern Errcode move_polys_loop(Mpl_data *data, int *pdx, int *pdy);
extern Errcode rub_move_poly(Poly *poly, Pixel color, int *pdx, int *pdy);
extern Errcode rub_size_polys(Mpl_data *mpl, int *pp, int *pq);

extern Errcode
rub_keep_star_type(struct poly *p, int startype, int color,
		int *ptheta, int *prad, int *prad2);

extern Errcode get_rub_shape(struct poly *poly, Pixel ocolor, Pixel offcolor);

#endif
