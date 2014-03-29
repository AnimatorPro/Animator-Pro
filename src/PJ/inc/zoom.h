#ifndef ZOOM_H
#define ZOOM_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct button;
struct rast;
struct short_xy;

extern struct button zpan_cycle_group;

/* muparts.c */
extern void zpan_ccycle_redraw(struct button *hanger);

/* zoom.c */
extern Boolean check_zoom_drag(void);
extern Boolean curs_in_zoombox(void);
extern void get_zoomcurs_flixy(struct short_xy *xy);
extern void close_zwinmenu(void);
extern Errcode zoom_handtool(void);
extern Boolean y_needs_zoom(Coor y);
extern void upd_zoom_dot(Pixel c, Coor x, Coor y);

extern void zoom_put_dot(struct raster *r, Pixel c, Coor x, Coor y);
extern void both_put_dot(struct raster *r, Pixel c, Coor x, Coor y);

extern void
zoom_put_hseg(struct raster *r, Pixel *pixbuf, Coor x, Coor y, Ucoor width);

extern void
zoom_txlatblit(struct raster *src, Coor sx, Coor sy, Ucoor width, Ucoor height,
		Coor dx, Coor dy, struct tcolxldat *tcxl);

extern void
zoom_put_vseg(struct raster *r, Pixel *pixbuf, Coor x, Coor y, Ucoor height);

extern void
zoom_blitrect(struct raster *src, Coor sx, Coor sy,
		Coor x, Coor y, Coor width, Coor height);

extern void rect_zoom_it(Coor x, Coor y, Coor w, Coor h);
extern void go_zoom_settings(void);
extern void zoom_it(void);
extern Boolean zoom_disabled(void);
extern void toggle_zoom(struct button *m);
extern void ktoggle_zoom(void);
extern Boolean zoom_hidden(void);
extern void unzoom(void);
extern void rezoom(void);
extern void init_zoom(void);

#endif
