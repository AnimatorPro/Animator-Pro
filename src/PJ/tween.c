/* generated with makepull */
#include "jimk.h"
#include "broadcas.h"
#include "commonst.h"
#include "errcodes.h"
#include "flx.h"
#include "linklist.h"
#include "menus.h"
#include "polyrub.h"
#include "softmenu.h"
#include "tween.h"
#include "tweenmag.h"
#include "tweenpul.h"

/* draw vertices and links in PTCOL */
#define PTCOL vb.screen->SBRIGHT
/* draw 1st vertex in PT1COL */
#define PT1COL vb.screen->SRED

extern Menuhdr twe_menu;

typedef struct tween_cb
	{
	Tween_state cur,old;
	short last_made;
	Boolean renderable;
	Minitime_data oflxdata;
	Pentool *optool;
	Pixel s_color;
	Pixel e_color;
	} Tween_cb;
Tween_cb *twcb;

static void twe_disable_refresh(void);
static void twe_enable_refresh(void);

static Boolean tween_got_both()
/* Make sure both end polygons of the tween are present */
{
return tween_has_data(&twcb->cur);
}

static Pixel get_pcolor(Poly *p)
/* Get color from poly - blue if it's the start, red if the end */
{
return(p == &twcb->cur.p0 ? twcb->s_color : twcb->e_color );
}

static Boolean get_p1_p2(Poly **pp1, Poly **pp2)
/* get the polygons for either end of the tween.  If one is not defined
   then use the same poly for both ends.  If neither defined return
   FALSE */
{
if (!tween_got_both())
	return(FALSE);
else
	{
	*pp1 = &twcb->cur.p0;
	*pp2 = &twcb->cur.p1;
	return(TRUE);
	}
}

static Boolean closest_in_tween(Poly **pp, 
	LLpoint **pl, long *pdist, int x, int y, int end_mode)
/* Return closest point in selectable polygon */
{
Poly *p1, *p2;
LLpoint *lp1, *lp2;
long d1, d2;

if (!get_p1_p2(&p1,&p2))
	return(FALSE);
lp1 = (end_mode != TWEEN_END ? closest_point(p1, x, y, &d1) : NULL);
lp2 = (end_mode != TWEEN_START ? closest_point(p2, x, y, &d2) : NULL);
if (lp1 == NULL)
	goto GOT_D2;
else if (lp2 == NULL)
	goto GOT_D1;
else
	{
	if (d1 < d2)
		goto GOT_D1;
	else
		goto GOT_D2;
	}
GOT_D1:
	*pp = p1;
	*pl = lp1;
	*pdist = d1;
	goto OUT;
GOT_D2:
	*pp = p2;
	*pl = lp2;
	*pdist = d2;
	goto OUT;
OUT:
	return(TRUE);
}

static void see_links(Dlheader *llist, Poly *p0, Poly *p1)
/* Draw the link lines in grey */
{
Tween_link *link, *next;
LLpoint *cl0, *cl1;
LLpoint *pt0, *pt1;
Marqihdr mh;
int color;

color = PTCOL;
cinit_marqihdr(&mh,color,color,TRUE);
cl0 = p0->clipped_list;
cl1 = p1->clipped_list;
for (link = (Tween_link *)(llist->tails_prev);
    NULL != (next = (Tween_link *)(link->node.prev));
	link = next)
	{
	pt0 = slist_el(cl0, link->start);
	pt1 = slist_el(cl1, link->end);
	pj_cline(pt0->x,pt0->y,pt1->x,pt1->y,mh.pdot,&mh);
	}
}

static void twe_see_both_ends()
/* Draw start and end polygons, and the link lines.  Draw small grey
   circles over active vertices.  */
{
Poly *p0, *p1;
Pixel sc, ec;

if (!get_p1_p2(&p0,&p1))
	return;
sc = twcb->s_color;
ec = twcb->e_color;
/* draw start poly */
switch (vs.tween_end)
	{
	case TWEEN_BOTH:
	case TWEEN_START:
		rub_poly_points(p0, vs.closed_curve, 
			sc, sc, PTCOL, PT1COL);
		break;
	case TWEEN_END:
		dotty_disp_poly(p0, vs.closed_curve, sc, sc);
		break;
	}
/* draw end poly */
switch (vs.tween_end)
	{
	case TWEEN_BOTH:
	case TWEEN_END:
		rub_poly_points(p1, vs.closed_curve, 
			ec, ec, PTCOL, PT1COL);
		break;
	case TWEEN_START:
		dotty_disp_poly(p1, vs.closed_curve, ec, ec);
		break;
	}
see_links(&twcb->cur.links,p0,p1);
}

static redraw_both_ends()
/* Clear screen and redraw */
{
zoom_unundo();
twe_see_both_ends();
}

void tween_undraw()
{
zoom_unundo();
}

void tween_redraw()
{
save_undo();
twe_see_both_ends();
}


static Errcode tween_sv_undo()
/* Free old state and clone current state into it. */
{
Errcode err;

free_dl_list(&twcb->old.links);
if ((err =  clone_dl_list(
	&twcb->cur.links, &twcb->old.links, sizeof(Tween_link))) < Success)
	return(err);
if ((err = update_poly(&twcb->cur.p0, &twcb->old.p0)) < Success)
	return(err);
return(update_poly(&twcb->cur.p1, &twcb->old.p1));
}

static Errcode tween_save_undo()
/* Copy current state to undo and report error */
{
return(softerr(tween_sv_undo(),"tween_undo"));
}


static void tween_swap_undo()
/* Exchange old state and cur state */
{
swap_mem(&twcb->cur.p0, &twcb->old.p0, sizeof(twcb->old.p0));
swap_mem(&twcb->cur.p1, &twcb->old.p1, sizeof(twcb->old.p1));
swap_dl_list(&twcb->old.links, &twcb->cur.links);
redraw_both_ends();
}



static void twtw_wire(int mode)
/* Undraw tween polys, wireframe tween, and redraw */
{
find_range();
zoom_unundo();
a_wireframe_tween(&twcb->cur, tr_frames, flix.hdr.speed,
	vs.ccolor, vs.ccolor, vs.closed_curve, mode);
twe_see_both_ends();
}

static void wireframe_tween()
/* Play wireframe of cur tween once */
{
twtw_wire(TWEEN_ONCE);
}

static void wireloop_tween()
/* Play wireframe of cur tween forever */
{
twtw_wire(TWEEN_LOOP);
}


static Poly *sel_poly()
/* Select active poly.  If both poly select different one from last time */
{
Poly *res;

switch (vs.tween_end)
	{
	case TWEEN_BOTH:
		res = (twcb->last_made ? &twcb->cur.p0 : &twcb->cur.p1);
		break;
	case TWEEN_START:
		res = &twcb->cur.p0;
		break;
	case TWEEN_END:
		res = &twcb->cur.p1;
		break;
	}
return(res);
}

static void set_last_made()
/* record which end made last */
{
switch (vs.tween_end)
	{
	case TWEEN_BOTH:
		twcb->last_made = !twcb->last_made;
		break;
	case TWEEN_START:
		twcb->last_made = FALSE;
		break;
	case TWEEN_END:
		twcb->last_made = TRUE;
		break;
	}
}

static int contrary_mode(Poly *p)
{
if (p == &twcb->cur.p0)
	return(TWEEN_END);
else
	return(TWEEN_START);
}


static Poly *other_poly(Poly *p)
/* Given poly at one end find poly at other */
{
if (p == &twcb->cur.p0)
	return(&twcb->cur.p1);
else
	return(&twcb->cur.p0);
}


static Errcode force_other(Poly *p)
/* Make sure other poly exists, forcing it to be a clone of this one if
   necessary .  Free up any old links. */
{
Poly *other;
Errcode err = Success;

set_last_made();
other = other_poly(p);
if (other->polymagic != POLYMAGIC)
	{
	clear_struct(other);
	err = update_poly(p,other);
	}
free_dl_list(&twcb->cur.links);
return(err);
}


static void tween_end_to_start()
/* copy end poly to start poly */
{
if (tween_save_undo()<Success)
	return;
softerr(update_poly(&twcb->cur.p1,&twcb->cur.p0),NULL);
redraw_both_ends();
}


static void tween_swap_ends()
{
	if (!tween_got_both())
		return;
	if (tween_save_undo()<Success)
		return;
	tween_state_swap_ends(&twcb->cur);
	redraw_both_ends();
}

static void tween_clear()
/* Delete both ends of tween. */
{
if (tween_save_undo()<Success)
	return;
trash_tween_state(&twcb->cur);
twcb->last_made = TRUE;
redraw_both_ends();
}

static rev_ix(int ix, int pcount, Boolean closed)
/* return index if poly were reversed */
{
if (closed)
	{
	if (ix == 0)
		return(0);
	else
		return(pcount-ix);
	}
else
	{
	return(pcount-ix-1);
	}
}

static void tween_reverse_poly()
/* Reverse order of point in active poly, & reverse corresponding links */
{
Tween_link *link,*next;
Dlheader *llist;
Poly *p;
int pcount;

llist = &twcb->cur.links;
p = sel_poly();
reverse_poly(p);
if (!vs.closed_curve)	/* if open reverse a little more complex */
	p->clipped_list = p->clipped_list->next;
pcount = p->pt_count;
for (link = (Tween_link *)(llist->tails_prev);
    NULL != (next = (Tween_link *)(link->node.prev));
	link = next)
	{
	if (p == &twcb->cur.p0)
		link->start = rev_ix(link->start,pcount,vs.closed_curve);
	else
		link->end = rev_ix(link->end,pcount,vs.closed_curve);
	}
isort_list(llist, tween_cmp_link);
redraw_both_ends();
}


static void tween_do_star(Poly *p, int startype)
/* Rubberband out some star class poly and replace active end with
   result if successful */
{
int theta, rad, rad2;
Poly rp;
Errcode err;

if (p != NULL)
	{
	clear_struct(&rp);
	if ((err = rub_keep_star_type(&rp, startype, get_pcolor(p),
								  &theta,&rad,&rad2) )
		>= Success)
		{
		free_polypoints(p);
		*p = rp;
		p->polymagic = POLYMAGIC;
		softerr(force_other(p),NULL);
		redraw_both_ends();
		}
	else
		free_polypoints(&rp);
	}
}

static Poly *query_end(Boolean saveit)
/* Ask user which end of the tween he's referring to. */
{
static char *keys[] = { NULL, "st", "end", "esc", NULL };

	if(saveit)
		keys[0] = "save";
	else
		keys[0] = "load";

	switch(soft_multi_box(keys,"tween_end"))
	{
		case 1:
			return(&twcb->cur.p0);
			break;
		case 2:
			return(&twcb->cur.p1);
			break;
		default:
			return(NULL);
	}
}

static void save_cur_shape()
/* Save one end of the tween to a .ply file */
{
char *path;
Poly *p;
char sbuf[50];

	if ((p = query_end(TRUE)) == NULL)
		return;
	if ((path = vset_get_filename(stack_string("save_shape", sbuf),
						".PLY",save_str,POLY_PATH,NULL,TRUE)) != NULL)
	{
		if(overwrite_old(path))
			save_poly(path, p);
	}
}

static void load_cur_shape()
/* Load one end of the tween from a .ply file */
{
char *path;
Poly *p;
char sbuf[50];

if ((p = query_end(FALSE)) == NULL)
	return;
if ((path = vset_get_filename(stack_string("load_shape", sbuf),
					".PLY",load_str,POLY_PATH,NULL,FALSE)) != NULL)
	{
	if (tween_save_undo()>=Success)
		{
		if (load_a_poly(path, p) >= Success)
			{
			softerr(force_other(p),NULL);
			redraw_both_ends();
			}
		}
	}
}

static void render_tween()
/* Save tween-display state and the do-auto on that tween */
{
twe_disable_refresh();
tween_undraw();
render_a_tween(&twcb->cur);
tween_redraw();
twe_enable_refresh();
}

static void render_trails()
{
static char *keys[] = { "ask", "kp", "vu", "esc", NULL };
short steps = 16;

	if(!soft_qreq_number(&steps,2,100,"twe_trsteps"))
		goto done;

	if (steps < 2)
		steps = 2;
	tween_undraw();
	save_undo();
	softerr(tween_trail_frame(&twcb->cur, steps), NULL);

	for(;;)
	{
		switch(soft_multi_box(keys, "twe_trail"))
		{
			case 2:
				wait_click();
				break;
			default:
				zoom_unundo();
			case 1:
				goto done;
		}
	}

done:
	tween_redraw();
	return;
}


static void tti_polygon()
/* create poly a click at a time */
{
Poly *p;
LLpoint *this;

p = sel_poly();
free_polypoints(p);
if ((this = new_poly_point(p)) != NULL)
	{
	make_poly_loop(p, vs.tween_spline, 
		vs.closed_curve, this, get_pcolor(p));
	p->polymagic = POLYMAGIC;
	softerr(force_other(p),NULL);
	redraw_both_ends();
	}
}

static void tti_shape()
{
Poly *p;
Pixel color;

	p = sel_poly();
	color = get_pcolor(p);
	get_rub_shape(p,color,color);
	softerr(force_other(p),NULL);
	redraw_both_ends();
}

static void tti_star()
/* create a star end */
{
tween_do_star(sel_poly(),WP_STAR);
}

static void tti_petal()
/* create a petal end */
{
tween_do_star(sel_poly(),WP_PETAL);
}

static void tti_rpoly()
/* create a regular poly end */
{
tween_do_star(sel_poly(),WP_RPOLY);
}

static void tti_oval()
/* create and oval end */
{
tween_do_star(sel_poly(),WP_ELLIPSE);
}

static void tti_mshape()
/* move a shape end */
{
Poly *poly;
LLpoint *point;
long dist;
int dx,dy;

if (!closest_in_tween(&poly,&point,&dist,icb.mx,icb.my,vs.tween_end))
	return;
rub_move_poly(poly, get_pcolor(poly),&dx,&dy);
redraw_both_ends();
}

static Boolean init_2p_mpl(Mpl_2p *mp)
/* make up data structure to feed polygon rub-mover/sizer out of
   our tween polys... */
{
if (!get_p1_p2(&mp->polys[0], &mp->polys[1]))
	return(FALSE);
mp->mpl.polys = mp->polys;
mp->mpl.dit_colors = mp->dit_colors;
mp->mpl.dot_colors = mp->dot_colors;
mp->mpl.pcount = 2;
mp->dit_colors[0] = mp->dot_colors[0] = twcb->s_color;
mp->dit_colors[1] = mp->dot_colors[1] = twcb->e_color;
return(TRUE);
}

static void tti_mtween()
/* move both ends of tween */
{
Mpl_2p m2;
int dx,dy;
Rcel_save oundo;

if (!init_2p_mpl(&m2))
	return;
if ((report_temp_save_rcel(&oundo, undof)) < Success)
	return;
save_undo();

move_polys_loop(&m2.mpl, &dx, &dy);

report_temp_restore_rcel(&oundo, undof);
redraw_both_ends();
}


static Errcode tween_size_polys(Mpl_data *mpl, int *pp, int *pq)
/* Let user resize a polygon list. */
{
Errcode err;
Rcel_save oundo;

if ((err = report_temp_save_rcel(&oundo, undof)) < Success)
	return(err);
save_undo();

err = rub_size_polys(mpl, pp, pq);

report_temp_restore_rcel(&oundo, undof);
redraw_both_ends();
return(err);
}

static void tti_sshape()
/* size shape tween tool */
{
Mpl_data mpl;
Pixel color;
Poly *poly;
LLpoint *point;
long dist;
int p,q;

if (!closest_in_tween(&poly,&point,&dist,icb.mx,icb.my,vs.tween_end))
	return;
color = get_pcolor(poly);
mpl.pcount = 1;
mpl.polys = &poly;
mpl.dit_colors = mpl.dot_colors = &color;
tween_size_polys(&mpl,&p,&q);
}

static void tti_stween()
/* size tween tween tool */
{
Mpl_2p m2;
int p,q;

if (!init_2p_mpl(&m2))
	return;
tween_size_polys(&m2.mpl,&p,&q);
}


static void tti_mpoint()
/* move point tween tool */
{
Poly *poly;
LLpoint *point;
LLpoint *prev, *next;
long dist;

if (!closest_in_tween(&poly,&point,&dist,icb.mx,icb.my,vs.tween_end))
	return;
pp_find_next_prev(poly, point, &next, &prev);
get_rub_vertex((Short_xy *)&(prev->x),
	   (Short_xy *)&(point->x),
	   (Short_xy *)&(next->x), get_pcolor(poly));
redraw_both_ends();
}

static void tw_mag(int mag_mode)
/* Magnet tween tool.  Moves points with effect falling off as of
   square of distance between point and initial pen-down */
{
Rcel_save oundo;
Poly *poly;
LLpoint *pl;
long distance;

if ((report_temp_save_rcel(&oundo, undof)) < Success)
	return;
save_undo();
if (closest_in_tween(&poly, &pl,  &distance, icb.mx, icb.my,vs.tween_end))
	{
	mag_loop(poly, get_pcolor(poly), distance, mag_mode);
	}
report_temp_restore_rcel(&oundo, undof);
redraw_both_ends();
}


static void tti_magnet()
/* inverse squared proportional move points */
{
tw_mag(MAG_MAGNET);
}

static void tti_blow()
/* inverse linear proportional move points */
{
tw_mag(MAG_BLOW);
}

#undef MAG_MAGNET
#undef MAG_BLOW



static void tti_link()
/* link points tween tool */
{
Poly *spoly, *epoly;
LLpoint *ps, *pe;
long distance;
Short_xy pt;
Tween_link *newl;
Errcode err = Success;
int startix, endix;

if (closest_in_tween(&spoly, &ps, &distance, icb.mx, icb.my,vs.tween_end))
	{
	if ((err = rubba_vertex((Short_xy *)(&ps->x),&pt,(Short_xy *)(&ps->x),NULL,
		PTCOL)) >= Success)
		{
		if (closest_in_tween(&epoly, &pe, &distance, icb.mx, icb.my,
			contrary_mode(spoly)))
			{
			/* Figure out which point is the start and which the end. */
			if (spoly == &twcb->cur.p0)
				{
				startix = slist_ix(spoly->clipped_list,ps);
				endix = slist_ix(epoly->clipped_list,pe);
				}
			else
				{
				endix = slist_ix(spoly->clipped_list,ps);
				startix = slist_ix(epoly->clipped_list,pe);
				}
			if ((err = tween_add_a_link(&twcb->cur, startix, endix
			, vs.closed_curve, &newl)) < Success)
				goto OUT;
			}
		}
	}
OUT:
	softerr(err,"tween_link");
	redraw_both_ends();
}


static VFUNC tti_vectors[] = 
/* Jump table to dispatch tween-tool */
	{
	tti_polygon,
	tti_shape,
	tti_star,
	tti_petal,
	tti_rpoly,
	tti_oval,
	tti_mpoint,	/* move point */
	tti_magnet,	/* magnet */
	tti_blow,	/* blow */
	tti_mshape,
	tti_mtween,	/* mtween */
	tti_sshape,	/* sshape */
	tti_stween,	/* stween */
	tti_link,	/* link */
	};


Boolean got_tween()
/* return whether tween files are saved. */
{
return(pj_exists(tween_name));
}

Boolean tween_renderable()
/* returns whether should grey out 'render' option */
{
return(twcb->renderable);
}

Errcode save_tween_state()
/* Save current tween state to temp file */
{
if (!tween_got_both())
	return(Success);
return(softerr(save_tween(tween_name, &twcb->cur), 
		"!%s", "tween_sstate",  tween_name));
}

Errcode load_tween_state()
/* Restore tween state from temp file */
{
Errcode err;

switch (err = load_tween(tween_name,&twcb->cur))
	{
	case Success:
		pj_delete(tween_name);
		break;
	case Err_no_file:
		break;
	default:
		err = softerr(err, "tween_state");
		break;
	}
return(err);
}


static void tween_selit(Menuhdr *mh, SHORT hitid)
/* Pull-down interpreter for tween system */
{
Poly *p;

hide_mp();
switch(hitid)
	{
	case TWE_UND_PUL:
		tween_swap_undo();
		break;
	case TWE_ONC_PUL:
		wireframe_tween();
		break;
	case TWE_LOO_PUL:
		wireloop_tween();
		break;
	case TWE_REN_PUL:
		render_tween();
		break;
	case TWE_TRA_PUL:
		render_trails();
		break;
	case TWE_END_PUL:
		tween_end_to_start();
		break;
	case TWE_SWA_PUL:
		tween_swap_ends();
		break;
	case TWE_CLE_PUL:
		tween_clear();
		break;
	case TWE_FIL_PUL:
		if (tween_save_undo()>=Success)
			{
			if (save_tween_state() >= Success)
				{
				tween_undraw();
				twe_disable_refresh();
				trash_tween_state(&twcb->cur);
				go_files(12);
				load_tween_state();
				twe_enable_refresh();
				tween_redraw();
				}
			}
		break;
	case TWE_QUI_PUL:
		mh_gclose_code(mh, Err_abort);
		break;
	case SHA_POL_PUL:	/* polygon */
		vs.tween_tool = TTI_POLY;
		break;
	case SHA_SHA_PUL: /* shape */
		vs.tween_tool = TTI_SHAPE;
		break;
	case SHA_STA_PUL:		/* Star */
		vs.tween_tool = TTI_STAR;
		break;
	case SHA_PET_PUL:		/* Petal */
		vs.tween_tool = TTI_PETAL;
		break;
	case SHA_RPO_PUL:		/* Rpoly */
		vs.tween_tool = TTI_RPOLY;
		break;
	case SHA_OVA_PUL:		/* oval */
		vs.tween_tool = TTI_OVAL;
		break;
	case SHA_REV_PUL:		/* reverse */
		tween_reverse_poly();
		break;
	case SHA_USE_PUL:		/* Use Last */
		if (tween_save_undo()>=Success)
			{
			if ((load_a_poly(poly_name, p = sel_poly())) >= Success)
				{
				softerr(force_other(p),NULL);
				redraw_both_ends();
				}
			}
		break;
	case SHA_LOA_PUL:		/* Load */
		load_cur_shape();
		break;
	case SHA_SAV_PUL:	/* Save */
		save_cur_shape();
		break;
	case MOV_MVP_PUL:		/* Move Point */
		vs.tween_tool = TTI_MPOINT;
		break;
	case MOV_MAG_PUL:		/* Magnet */
		vs.tween_tool = TTI_MAGNET;
		break;
	case MOV_BLO_PUL:		/* Blow */
		vs.tween_tool = TTI_BLOW;
		break;
	case MOV_MVS_PUL:		/* Move Shape */
		vs.tween_tool = TTI_MSHAPE;
		break;
	case MOV_MVT_PUL:		/* Move tween */
		vs.tween_tool = TTI_MTWEEN;
		break;
	case MOV_SZS_PUL:		/* Size shape */
		vs.tween_tool = TTI_SSHAPE;
		break;
	case MOV_SZT_PUL:		/* Size tween */
		vs.tween_tool = TTI_STWEEN;
		break;
	case MOV_LIN_PUL:		/* Link Points */
		vs.tween_tool = TTI_LINK;
		break;
	case MOV_CLE_PUL:		/* clear links */
		if (tween_save_undo()>=Success)
			{
			free_dl_list(&twcb->cur.links);
			redraw_both_ends();
			}
		break;
	case OPT_CLO_PUL:
		vs.closed_curve = !vs.closed_curve;
		redraw_both_ends();
		break;
	case OPT_SPL_PUL:
		vs.tween_spline = !vs.tween_spline;
		break;
	case OPT_TWO_PUL:
		vs.color2 = !vs.color2;
		break;
	case OPT_IN__PUL:		/* in slow */
		vs.ado_ease = !vs.ado_ease;
		break;
	case OPT_OUT_PUL:		/* out slow */
		vs.ado_ease_out = !vs.ado_ease_out;
		break;
	case OPT_STI_PUL:		/* still */
		vs.ado_tween = !vs.ado_tween;
		break;
	case OPT_PIN_PUL:		/* ping-pong */
		vs.ado_pong = !vs.ado_pong;
		break;
	case OPT_REV_PUL:		/* reverse */
		vs.ado_reverse = !vs.ado_reverse;
		break;
	case OPT_COM_PUL:		/* complete */
		vs.ado_complete = !vs.ado_complete;
		break;
	case ACT_STA_PUL:
		vs.tween_end = TWEEN_START;
		redraw_both_ends();
		break;
	case ACT_END_PUL:
		vs.tween_end = TWEEN_END;
		redraw_both_ends();
		break;
	case ACT_BOT_PUL:
		vs.tween_end = TWEEN_BOTH;
		redraw_both_ends();
		break;
	}
show_mp();
}

twe_tool_func()
{
if (!tti_input())
	return;
if (tween_save_undo()>=Success)
	(*(tti_vectors[vs.tween_tool]))();
}


Pentool tween_pen_tool = PTOOLINIT1(
	NONEXT,
	NOTEXT, /* "Tween", loaded by load_tween_panel_strings() */
	PTOOL_OPT,
	TWEEN_PTOOL,
	"", /* no help text used!!!! "Transform one shape into another.", */
	NO_SUBOPTS,
	NULL,
	twe_tool_func,
	&plain_ptool_cursor,
	NULL, /* on install */
	NULL /* on remove */
);


void twe_go_tool(Button *b)
{
	(void)b;

	hide_mp();
	tween_undraw();
	twe_disable_refresh();
	qtools();
	twe_enable_refresh();
	tween_redraw();
	show_mp();
}

static void tween_pull_disables(Menuhdr *mh)
{
static SHORT notween_pulltab[] = 
	{
	TWE_ONC_PUL,
	TWE_LOO_PUL,
	TWE_END_PUL,
	TWE_SWA_PUL,
	TWE_TRA_PUL,
	TWE_CLE_PUL,
	TWE_END_PUL,
	SHA_SAV_PUL,
	SHA_REV_PUL,
	};
Boolean notween = !tween_got_both();

set_pul_disable(mh, TWE_REN_PUL, !tween_renderable() || notween);
set_pul_disable(mh, SHA_USE_PUL, !pj_exists(poly_name));
set_pultab_disable(mh, notween_pulltab, Array_els(notween_pulltab), notween);
set_leaf_disable(mh, MOV_PUL, notween);
}

static void tween_pull_asterisks(Menuhdr *mh)
{
static SHORT twtool_pulltab[] =
	{
	SHA_POL_PUL,
	SHA_SHA_PUL,
	SHA_STA_PUL,
	SHA_PET_PUL,
	SHA_RPO_PUL,
	SHA_OVA_PUL,
	MOV_MVP_PUL,
	MOV_MAG_PUL,
	MOV_BLO_PUL,
	MOV_MVS_PUL,
	MOV_MVT_PUL,
	MOV_SZS_PUL,
	MOV_SZT_PUL,
	MOV_LIN_PUL,
	};
pul_xflag(mh, ACT_BOT_PUL, vs.tween_end == TWEEN_BOTH);
pul_xflag(mh, ACT_STA_PUL, vs.tween_end == TWEEN_START);
pul_xflag(mh, ACT_END_PUL, vs.tween_end == TWEEN_END);
pul_xflag(mh, OPT_CLO_PUL, vs.closed_curve);
pul_xflag(mh, OPT_TWO_PUL, vs.color2);
pul_xflag(mh, OPT_SPL_PUL, vs.tween_spline);
pul_xflag(mh, OPT_STI_PUL, !vs.ado_tween);
pul_xflag(mh, OPT_PIN_PUL, vs.ado_pong);
pul_xflag(mh, OPT_IN__PUL, vs.ado_ease);
pul_xflag(mh, OPT_OUT_PUL, vs.ado_ease_out);
pul_xflag(mh, OPT_REV_PUL, vs.ado_reverse);
pul_xflag(mh, OPT_COM_PUL, vs.ado_complete);
pultab_xoff(mh, twtool_pulltab, Array_els(twtool_pulltab));
if (!tween_got_both() && vs.tween_tool >= TTI_MPOINT)
	vs.tween_tool = TTI_POLY;
pul_xflag(mh, twtool_pulltab[vs.tween_tool], TRUE);
}


static int tween_dopull(Menuhdr *mh)
{
tween_pull_asterisks(mh);
tween_pull_disables(mh);
return(menu_dopull(mh));
}


static twe_set_colors()
{
	twcb->s_color = vs.inks[1];
	twcb->e_color = vs.inks[2];
}

static void twe_color_refresh(void *dat, USHORT why)
{
Pixel os, oe;
(void)dat;
(void)why;

	os = twcb->s_color;
	oe = twcb->e_color;
	twe_set_colors();

	if(flx_olays_hidden()
		|| flxtime_data.draw_overlays != tween_redraw)
	{
		return;
	}

	if(os != twcb->s_color
		|| oe != twcb->e_color)
	{
		twe_see_both_ends();
	}
}

static Redraw_node twe_color_rn = {
	{ NULL, NULL }, /* node */
	twe_color_refresh,
	NULL,
	NEW_MINIPAL_INK,
};

static void twe_disable_refresh(void)
{
	flxtime_data.clear_overlays = NULL;
	flxtime_data.draw_overlays = NULL;
	rem_color_redraw(&twe_color_rn);
}
static void twe_enable_refresh(void)
{
	flxtime_data.clear_overlays = tween_undraw;
	flxtime_data.draw_overlays =  tween_redraw;
	add_color_redraw(&twe_color_rn);
}
static Boolean tween_menu_keys()
{
 	if(check_toggle_abort())
		return(TRUE);
	if (common_header_keys())
		return(TRUE);
	if(hit_undo_key())
	{
		tween_swap_undo();
		return(TRUE);
	}
	return(FALSE);
}

void tween_menu(
	Boolean renderable	/* grey out render button? */
	)	
/* entry point to tween menu */
{
Tween_cb rtcb;
Menuhdr tpull;
void *ss = NULL;
void *oundo;
void *oredo;

	if(MENU_ISOPEN(&twe_menu))
		return;
	/* save and clear undo and redo functions */
	oredo = vl.redoit;
	oundo = vl.undoit;
	vl.redoit = NULL;
	vl.undoit = NULL;

	hide_mp();
	clear_struct(&rtcb);
	twcb = &rtcb;
	twe_set_colors();
	twcb->oflxdata = flxtime_data; /* save old one */
	twcb->optool = vl.ptool;  /* save old one */
	fliborder_on();
	if (load_soft_pull(&tpull, 7, "tween", TWEENP_MUID,
		tween_selit, tween_dopull) < Success)
		goto OUT;
	if (load_tween_panel_strings(&ss) < Success)
		goto OUT;
	twe_enable_refresh();
	set_curptool(&tween_pen_tool);
	load_tween_state();
	init_tween_state(&twcb->old);
	twcb->last_made = TRUE;
	twcb->renderable = renderable;
	if (tween_save_undo()<Success)
		goto OUT;
	tween_redraw();
	menu_to_quickcent(&twe_menu);
	do_menuloop(vb.screen,&twe_menu,NULL,&tpull,tween_menu_keys);
	tween_undraw();
	save_tween_state();
	trash_tween_state(&twcb->old);
	trash_tween_state(&twcb->cur);
OUT:
	twe_disable_refresh();
	smu_free_pull(&tpull);
	smu_free_scatters(&ss);
	flxtime_data = twcb->oflxdata;
	restore_pentool(twcb->optool);
	twcb = NULL;
	show_mp();
	vl.redoit = oredo; /* restore old undo and redo functions */
	vl.undoit = oundo;
}

#ifdef SLUFFED
Boolean in_tween_menu()
{
return(twcb != NULL);
}
#endif /* SLUFFED */

#ifdef SLUFFED
void mtween_poly(void)
/* Entry point to tween system from pen-tools/poly */
{
tween_menu(TRUE);
}
#endif /* SLUFFED */

#ifdef SLUFFED
void mtween_curve(void)
/* Entry point to tween system from pen-tools/spline */
{
extern char curveflag;

curveflag = 1;
tween_menu(TRUE);
curveflag = 0;
}
#endif /* SLUFFED */
