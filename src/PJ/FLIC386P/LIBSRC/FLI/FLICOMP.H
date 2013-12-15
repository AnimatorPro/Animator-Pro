#ifndef FLICOMP_H
#define FLICOMP_H

typedef struct complib {
	SHORT first_type;
	SHORT first_color_type;

	void *(*first_comp)(Raster *screen, void *cbuf,
					   SHORT x, SHORT y, SHORT width, SHORT height);
	void *(*first_colors)(Rgb3 *ctab, void *cbuf, int num_colors );
	LONG (*make_pstamp)(Rcel *screen, void *cbuf,
					 	   SHORT x,SHORT y,USHORT width,USHORT height);
	SHORT delta_type;
	SHORT delta_color_type;

	void *(*delta_comp)(Raster *last_screen, void *cbuf, SHORT lx, SHORT ly,
					   Raster *this_screen,
					   SHORT tx, SHORT ty, SHORT width, SHORT height);
	void *(*delta_colors)(Rgb3 *last_ctab, Rgb3 *this_ctab,
						  void *cbuf, int num_colors );
} Complib;

#endif /* FLICOMP_H */
