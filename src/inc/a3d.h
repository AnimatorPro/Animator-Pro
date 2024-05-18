#ifndef A3D_H
#define A3D_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

struct button;
struct menuhdr;
struct short_xyz;

#define ADO_SPIN 0
#define ADO_SIZE 1
#define ADO_MOVE 2
#define ADO_PATH 3
#define SPIN_CENTER 0
#define SPIN_AXIS 1
#define SPIN_TURNS 2
#define PATH_SPLINE 0
#define PATH_POLY 1
#define PATH_SAMPLED 2
#define PATH_CLOCKED 3

#define OPS_SCREEN	0
#define OPS_THECEL	1
#define OPS_POLY 	2
#define OPS_SPLINE	3
#define OPS_TWEEN   4

extern struct menuhdr a3d_menu;

/* guy that gets directly dinked by xyz sliders */
extern struct short_xyz rot_theta;

/* a3ddat.c lets us know here if there's a path */
extern SHORT got_path;

/* a3ddat.c's flag if spin sub-panel is up */
extern char inspin;

/* a3d.c */
extern void default_center(struct short_xyz *v);
extern void iscale_theta(void);
extern void ado_xyz_slider(struct button *b);
extern void xyz_zero_sl(struct button *m);
extern void mado_loop(void);
extern void mado_view(void);
extern void mauto_ado(void);
extern void ado_clear_pos(void);
extern void move_along(struct button *m);
extern void qload_a3d(void);
extern void qsave_a3d(void);
extern void mview_path(void);
extern void edit_path(void);
extern void go_ado(void);

/* a3ddat.c */
extern Errcode load_a3d_panel(void **ss);
extern void a3d_disables(void);
extern void arrange_a3d_menu(void);

#endif
