#ifndef SDOT_H
#define SDOT_H

/* structure passed to solid line and dot routines */

typedef struct sdat {
	void *rast;
	Pixel color;
} Sdat;

/* Solid drawing routines. */
extern void gl_sdot(SHORT x, SHORT y, void *dat);
extern Errcode gl_shline(SHORT y,SHORT x0, SHORT x1, void *dat);
extern void gl_scline(SHORT x1, SHORT y1, SHORT x2, SHORT y2, void *dat);

#endif /* SDOT_H */
