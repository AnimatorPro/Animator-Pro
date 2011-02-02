
#ifndef POLYRUB_H
#define POLYRUB_H

#ifndef MARQI_H
	#include "marqi.h"
#endif

typedef struct mpl_data
	{
	int pcount;
	Poly **polys;
	Pixel *dit_colors;
	Pixel *dot_colors;
	} Mpl_data;

typedef struct mpl_2p
	{
	Mpl_data mpl;
	Poly *polys[2];
	Pixel dit_colors[2];
	Pixel dot_colors[2];
	} Mpl_2p;

Errcode make_poly(Poly *p, Boolean closed);
void rub_poly_points(Poly *p, Boolean closed,
	Pixel dit_color, Pixel dot_color, Pixel pt_color, Pixel pt1_color);
void unrub_poly_points(Poly *p, Boolean closed);
void move_poly_points(Poly *poly, Boolean closed);
void marqi_mpl_polys(Mpl_data *data, Marqihdr *mh);
void unmarqi_mpl_polys(Mpl_data *data, Marqihdr *mh);
Errcode move_polys_loop(Mpl_data *data, int *pdx, int *pdy);
Errcode rub_move_poly(Poly *poly, Pixel color, int *pdx, int *pdy);
Errcode rub_sz_polys(Mpl_data *mpl, 
	Poly **origs, int pcount, int *pp, int *pq);
Errcode rub_size_polys(Mpl_data *mpl, int *pp, int *pq);
Errcode rub_keep_star_type(Poly *p, int startype, int color, int *ptheta,
	int *prad, int *prad2);
Errcode rub_star_type(int startype, 
	int color, int *ptheta, int *prad, int *prad2);
#endif /* POLYRUB_H */
