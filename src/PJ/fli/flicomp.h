#ifndef FLICOMP_H
#define FLICOMP_H

struct rgb3;

typedef struct complib {
	SHORT first_type;
	SHORT first_color_type;

	void *(*first_comp)(Raster *screen, void *cbuf,
					   SHORT x, SHORT y, USHORT width, USHORT height);
	void *(*first_colors)(struct rgb3 *ctab, void *cbuf, int num_colors);
	LONG (*make_pstamp)(Rcel *screen, void *cbuf,
					 	   SHORT x,SHORT y,USHORT width,USHORT height);
	SHORT delta_type;
	SHORT delta_color_type;

	void *(*delta_comp)(Raster *last_screen, void *cbuf, SHORT lx, SHORT ly,
					   Raster *this_screen,
					   SHORT tx, SHORT ty, USHORT width, USHORT height);
	void *(*delta_colors)(struct rgb3 *last_ctab, struct rgb3 *this_ctab,
						  void *cbuf, unsigned int num_colors);
} Complib;

#endif /* FLICOMP_H */
