#ifndef PALMENU_H
#define PALMENU_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

struct bundle;
struct button;
struct rgb3;
struct rscale;

extern struct button qmu_clus_sel;

/* cluster.c */
extern void ctable_to_cluster(struct rgb3 *ctab, int ccount);
extern struct rgb3 *cluster_to_ctable(void);

extern void shortcut_ccycle(struct button *b);
extern void feel_pp(struct button *m);
extern int get_a_end(void (*dfunc)(Pixel c));

extern void
some_cmod(void (*f)(int scale, struct rgb3 *p, int cix, int ix), int scale);

extern void force_ramp(void);
extern void find_ramp(void);
extern void right_click_pp(struct button *m);
extern void ccolor_box(struct button *b);

extern void ccycle(void);
extern void ctint(void);
extern void cneg(void);
extern void cclose(void);
extern void cunused(void);

extern void cl_cut(void);
extern void cl_paste(void);
extern void cl_blend(void);
extern void cl_swap(void);

extern void cluster_invert(void);
extern void cluster_reverse(void);
extern void cluster_line(void);
extern void change_cluster_mode(struct button *b);
extern void ping_cluster(void);
extern void see_cluster(struct button *m);
extern void feel_cluster(struct button *m);

extern int in_bundle(Pixel color, struct bundle *bun);
extern void qpick_bundle(void);
extern void qselect_bundle(void);
extern void mselect_bundle(struct button *m);

extern void scale_powell_palette(struct rscale *scale);
extern void see_powell_palette(struct button *b);

/* csort.c */
extern void cpack(void);
extern void csort(void);
extern void cthread(void);
extern void cspec(void);

/* cthread.c */
extern void
rthread_cmap(UBYTE *gotit,
		struct rgb3 *scmap, struct rgb3 *dcmap, int inertia, int ccount);

/* quickdat.c */
extern void see_clusid(struct button *b);
extern void toggle_clusid(struct button *b);
extern void see_crb(struct button *m);
extern void feel_crb(struct button *m);

/* vpsubs.c */
extern int cluster_count(void);
extern UBYTE *cluster_bundle(void);

#endif
