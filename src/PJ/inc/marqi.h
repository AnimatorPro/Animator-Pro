#ifndef MARQI_H
#define MARQI_H

#ifndef RECTANG_H
	#include "rectang.h"
#endif

#ifndef VERTICES_H
	#include "vertices.h"
#endif

struct wndo;
struct wscreen;

#define DMARQI_MILLIS 100 /* milli-seconds between marqi draws for creepy
					       * marqis */

typedef struct marqihdr {
	struct wndo *w; /* loaded by init current marqi window */
	Fullrect port;	/* clip port for drawing and saving within Raster w */
	Pixel oncolor, offcolor; /* loaded by init on and off colors */
		/* raster put dot function set by init to "put_dot" */ 	
	void (*putdot)(Raster *r, Pixel c, Coor x, Coor y);

	SHORT smod;     /* set by init to 0 the start "mod" */
	SHORT dmod;		/* set by init to 0 the current dot mod */
	dotout_func pdot; /* loaded by init: dotout function */
	void *adata;	/* animation subroutine specific data */
	UBYTE *dotbuf;	/* current dot in save buffer for save and restore
					 * dot calls set to NULL by init calls */
	SHORT waitcount; /* animation timeout set by init_marqihdr */
	SHORT unused[7]; /* for future */
} Marqihdr;

typedef struct marqi_circdat {
	Marqihdr mh;
	UBYTE *save;
	SHORT saved;
	Short_xy pos;
	SHORT d;
	Short_xy cent;
	SHORT movecent;
} Marqi_circdat;

/* cutcurs.c */
extern Errcode marqi_cut_xy(void);

/* vmarqi.c */
extern void
cinit_marqihdr(Marqihdr *mh, Pixel oncolor, Pixel offcolor, Boolean bothwins);

extern void vinit_marqihdr(Marqihdr *mh, int marqi_it, Boolean bothwins);

extern void undo_marqidot(SHORT x, SHORT y, void *marqihdr);
extern void savedraw_marqidot(SHORT x, SHORT y, void *marqihdr);
extern void restore_marqidot(SHORT x, SHORT y, void *marqihdr);

extern Errcode
rubba_vertex(Short_xy *p0, Short_xy *p1, Short_xy *p2,
		void (*dispfunc)(Short_xy *v), Pixel color);

extern Errcode
get_rub_vertex(Short_xy *p0, Short_xy *p1, Short_xy *p2, Pixel color);

extern Errcode get_rub_line(Short_xy *xys);
extern Errcode get_rub_axis(Short_xy *ends, Pixel oncol, Pixel offcol);
extern void marqi_rect(Marqihdr *mh, Rectangle *r);
extern Errcode rect_in_place(Rectangle *rect);
extern void box_coors(SHORT x, SHORT y, SHORT ox, SHORT oy);
extern Errcode rub_rect_in_place(Rectangle *rect);
extern Errcode quadpoly_in_place(Short_xy *qpoly);
extern Errcode clip_move_rect(Rectangle *rect);
extern Errcode cut_out_clip(Cliprect *clip);
extern Errcode get_rub_rect(Rectangle *rect);
extern Errcode get_srub_rect(Rectangle *rect);
extern Errcode cut_out_rect(Rectangle *rect);
extern Errcode gcut_out_rect(Rectangle *rect);

extern void savedraw_circle(Marqi_circdat *cd, Short_xy *cent, SHORT d);
extern void restore_circle(Marqi_circdat *cd, Short_xy *cent, SHORT d);
extern Errcode init_circdat(Marqi_circdat *cd, Pixel color);
extern Errcode get_rub_circle(Short_xy *cent, SHORT *diam, Pixel color);
extern Errcode rub_circle_diagonal(Short_xy *cent, SHORT *diam, Pixel color);

extern void
msome_vector(Short_xy *pts, int count,
		dotout_func dotout, void *dotdat, int open, int pt_size);

extern void marqi_vector(Marqihdr *mh, Short_xy *pts, int count, int pt_size);

/* wndo/marqi.c */
extern void
init_marqihdr(Marqihdr *mh, struct wndo *w, Rectangle *port,
		Pixel oncolor, Pixel offcolor);

extern Errcode marqmove_rect(Marqihdr *mh, Rectangle *rect, Rectangle *bclip);
extern void marqi_cut(Marqihdr *mh, Coor x, Coor y);
extern Errcode mh_cut_rect(Marqihdr *mh, Rectangle *rect, Rectangle *sclip);

extern Errcode
screen_cut_rect(struct wscreen *s, Rectangle *rect, Rectangle *sclip);

#endif
