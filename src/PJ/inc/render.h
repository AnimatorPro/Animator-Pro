#ifndef RENDER_H
#define RENDER_H

#ifndef RASTER
#include "raster.h"
#endif

struct cmap;
struct poly;
struct rcel;
struct rgb3;
struct short_xy;
struct tcolxldat;
struct xformspec;

typedef struct renderdata {
	Cliprect cr;			/* the current render form clip rectangle */

	/* color gradient rectangle data */

    SHORT rdx0,rdy0,rdx1,rdy1; /* gradient range min and max rect */
	SHORT rwidth,rheight;  /* gradient range width and height */
} Rendata;

extern Rendata rdta;
extern char under_flag;

/* inkcashe.c */
extern void set_render_fast(void);
extern void free_render_cashes(void);
extern Errcode make_render_cashes(void);

/* inkdot.c */
extern void render_dot(SHORT x, SHORT y, void *data);
extern Errcode render_hline(SHORT y, SHORT x0, SHORT x1, void *r);
extern Errcode poll_render_hline(SHORT y, SHORT x0, SHORT x1, void *r);

/* render.c */
extern void set_render_clip(Rectangle *rect); // 39
extern void set_gradrect(Rectangle *rect); // 76
extern void set_twin_gradrect(void); // 86
extern void set_xy_gradrect(SHORT x0, SHORT y0, SHORT x1, SHORT y1); // 91
extern void set_full_gradrect(void); // 113

extern Errcode
transpblit(struct rcel *tcel, int clearcolor, int clear, int tinting); // 310

extern Errcode rblit_cel(struct rcel *c, struct tcolxldat *txd); // 343

extern void
render_mask_blit(UBYTE *mplane, SHORT mbpr, SHORT mx, SHORT my,
		void *drast, SHORT rx, SHORT ry, USHORT width, USHORT height); // 410

extern void
render_mask_alpha_blit(UBYTE *alpha, int abpr, int x, int y, int w, int h,
		struct rcel *r, Pixel oncolor); // 465

extern Errcode render_disk(Raster *r, SHORT cenx, SHORT ceny, SHORT diam); // 518
extern Errcode render_box(SHORT x, SHORT y, SHORT xx, SHORT yy); // 554
extern Errcode render_beveled_box(Rectangle *r, int bevel, Boolean filled); // 572
extern void render_outline(struct short_xy *pt, int count); // 645
extern void render_brush(SHORT x, SHORT y); // 658
extern Errcode render_opoly(struct poly *p, Boolean closed); // 711
extern Errcode render_circle(Raster *r, SHORT cenx, SHORT ceny, SHORT diam); // 728
extern Errcode render_separate(PLANEPTR ctable, int ccount, Rectangle *rect); // 749

extern Errcode
render_transform(struct rcel *cel, struct xformspec *xf, struct tcolxldat *txd); // 851

/* thikline.c */
extern Errcode render_line(SHORT x, SHORT y, SHORT xx, SHORT yy);
extern Errcode render_1_line(SHORT x, SHORT y, SHORT xx, SHORT yy);

#endif
