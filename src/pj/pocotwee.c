#include "jimk.h"
#include "errcodes.h"
#include "pocoface.h"
#include "pocolib.h"
#include "tween.h"
#include "auto.h"

/* pocotwee.c - Poco library functions for the tweening system.
 */

extern Errcode po_poly_to_arrays(Poly *p, Popot *x, Popot *y);
extern Errcode po_arrays_to_poly(Poly *p, int ptcount, Popot *px, Popot *py);
extern ErrCode po_arrays_to_ll_poly(Poly *poly
,  int ptcount, Popot *px, Popot *py);
extern Errcode po_2_arrays_check(int ptcount, Popot *px, Popot *py);

void po_tween_clear_links(void);
Boolean po_tween_exists(void);

static Tween_state poco_tween_state;

void init_poco_tween()
/*----------------------------------------------------------------------------
 * Load in tween state from temp file.
 *--------------------------------------------------------------------------*/
{
	load_tween(tween_name, &poco_tween_state);
}

void cleanup_poco_tween()
/*----------------------------------------------------------------------------
 * Save out tween that Poco maybe altered,  and free it up.
 *--------------------------------------------------------------------------*/
{
	if (tween_has_data(&poco_tween_state))
		save_tween(tween_name, &poco_tween_state);
	trash_tween_state(&poco_tween_state);
}


static ErrCode po_tween_load(Popot pop_file_name)
/*****************************************************************************
 * ErrCode TweenLoad(char *file_name);
 ****************************************************************************/
{
	Errcode err;
	char *file_name;

	if ((file_name = pop_file_name.pt) == NULL)
		return builtin_err = Err_null_ref;
	trash_tween_state(&poco_tween_state);
	if ((err = load_tween(file_name, &poco_tween_state)) < Success)
		return err;
	return Success;
}

static ErrCode po_tween_save(Popot pop_file_name)
/*****************************************************************************
 * ErrCode TweenSave(char *file_name);
 ****************************************************************************/
{
	char *file_name;

	if (!po_tween_exists())
		return Err_not_found;
	if ((file_name = pop_file_name.pt) == NULL)
		return builtin_err = Err_null_ref;
	return save_tween(file_name, &poco_tween_state);
}

static Boolean po_tween_exists(void)
/*****************************************************************************
 * Boolean TweenExists(void);
 ****************************************************************************/
{
	return tween_has_data(&poco_tween_state);
}

static void po_tween_clear(void)
/*****************************************************************************
 * void	TweenClear(void);
 ****************************************************************************/
{
	trash_tween_state(&poco_tween_state);
}

static ErrCode po_tween_set_poly(int ptcount, Popot *px, Popot *py, Poly *poly,
	Poly *other_poly)
/*----------------------------------------------------------------------------
 * Set one of the tween polygons to a pocy 2 array style poly.
 * If the other poly is empty, set it too.
 *--------------------------------------------------------------------------*/
{
	Errcode err;

	if ((err = po_2_arrays_check(ptcount, px, py)) < Success)
		return err;
	free_polypoints(poly);
	po_tween_clear_links();
	if ((err = po_arrays_to_ll_poly(poly, ptcount, px, py)) >= Success)
		{
		if (other_poly->polymagic != POLYMAGIC)
			err = po_arrays_to_ll_poly(other_poly, ptcount, px, py);
		}
	return err;
}

static ErrCode po_tween_get_poly(Poly *poly
, Popot *pptcount, Popot *ppx, Popot *ppy)
/*----------------------------------------------------------------------------
 Get Set one of the tween polygons into a pocy 2 array style poly.
 *--------------------------------------------------------------------------*/
{
	int *ptcount;
	Popot *px, *py;
	Errcode err;

	if (!po_tween_exists())
		return Err_not_found;
	/* Make sure that input Popot's are non-null. */
	if ((ptcount = pptcount->pt) == NULL
	||	(px = ppx->pt) == NULL
	||	(py = ppy->pt) == NULL)
		return builtin_err = Err_null_ref;
	if ((err = po_poly_to_arrays(poly, px, py)) < Success)
		return err;
	*ptcount = poly->pt_count;
	return Success;
}

static ErrCode po_tween_set_start(int ptcount, Popot pop_x, Popot pop_y)
/*****************************************************************************
 * ErrCode TweenSetStart(int ptcount, int *x, int *y);
 ****************************************************************************/
{
	return po_tween_set_poly(ptcount, &pop_x, &pop_y
	, &poco_tween_state.p0, &poco_tween_state.p1);
}

static ErrCode po_tween_get_start(Popot pop_ptcount, Popot pop_x, Popot pop_y)
/*****************************************************************************
 * ErrCode TweenGetStart(int *ptcount, int **x, int **y);
 ****************************************************************************/
{
	return po_tween_get_poly(&poco_tween_state.p0, &pop_ptcount
	,	&pop_x, &pop_y);
}

static ErrCode po_tween_set_end(int ptcount, Popot pop_x, Popot pop_y)
/*****************************************************************************
 * ErrCode TweenSetEnd(int ptcount, int *x, int *y);
 ****************************************************************************/
{
	return po_tween_set_poly(ptcount, &pop_x, &pop_y
	, &poco_tween_state.p1, &poco_tween_state.p0);
}

static ErrCode po_tween_get_end(Popot pop_ptcount, Popot pop_x, Popot pop_y)
/*****************************************************************************
 * ErrCode TweenGetEnd(int *ptcount, int **x, int **y);
 ****************************************************************************/
{
	return po_tween_get_poly(&poco_tween_state.p1, &pop_ptcount
	,	&pop_x, &pop_y);
}

static void po_tween_swap_ends(void)
/*****************************************************************************
 * void	TweenSwapEnds(void);
 ****************************************************************************/
{
	if (po_tween_exists())
		tween_state_swap_ends(&poco_tween_state);
}

static ErrCode po_tween_end_to_start(void)
/*****************************************************************************
 * ErrCode TweenEndToStart(void);
 ****************************************************************************/
{
	return update_poly(&poco_tween_state.p1,&poco_tween_state.p0);
}

static ErrCode po_tween_one_link(int start_point, int end_point)
/*****************************************************************************
 * ErrCode TweenOneLink(int start_point, int end_point);
 ****************************************************************************/
{
	Tween_link *nlink;

	free_dl_list(&poco_tween_state.links);
	return tween_add_a_link(&poco_tween_state, start_point, end_point
	, vs.closed_curve, &nlink);
}

static ErrCode po_tween_set_links(int link_count
, Popot pop_starts, Popot pop_ends)
/*****************************************************************************
 * Errcode TweenSetLinks(int link_count, int *starts, int *ends);
 ****************************************************************************/
{
	Errcode err;
	Tween_link *nlink;
	int *starts, *ends;
	int start, end;
	int start_diff, end_diff;
	int p0count, p1count;
	int i;

	if (!po_tween_exists())		/* Need both polys for this to work. */
		return Err_not_found;
	po_tween_clear_links();		/* Get rid of old links. */
	if (link_count <= 0)		/* If no new ones return now. */
		return Success;
	if ((err = po_2_arrays_check(link_count, &pop_starts, &pop_ends)) 
	< Success)					/* Make sure data is really there. */
		return err;
	starts = pop_starts.pt;
	ends = pop_ends.pt;
	/* Do the first link separately because... */
	start = starts[0];
	end = ends[0];
	if ((err = tween_add_a_link(&poco_tween_state, start, end
	, vs.closed_curve, &nlink))
		< Success)
		return err;
	/* ...the first link setting is quite possibly going to reorder the
	 * polygon.  If this happens we need to reorder the succeeding links.
	 * So we just set the first link,  see if it comes back the same
	 * as what we fed in,  and then adjust things if not. */
	start_diff = nlink->start - start;
	end_diff = nlink->end - end;
	if (start_diff != 0 || end_diff != 0)
		{
		p0count = poco_tween_state.p0.pt_count;
		p1count = poco_tween_state.p1.pt_count;
		start_diff += p0count;		/* In case it's negative */
		end_diff += p1count;
		for (i=1; i<link_count; ++i)
			{
			starts[i] = (starts[i] + start_diff)%p0count;
			ends[i] = (ends[i] + end_diff)%p1count;
			}
		}
	/* Now we just add the links to tween one at a time. */
	for (i=1; i<link_count; ++i)
		{
		if ((err = tween_add_a_link(&poco_tween_state, starts[i], ends[i]
		, vs.closed_curve, &nlink))
			< Success)
			return err;
		}
	return Success;
}

static Errcode po_lmalloc_2(Popot *x, Popot *y, int size)
/*----------------------------------------------------------------------------
 * Allocate Poco memory for 2 arrays that are the same size.
 *--------------------------------------------------------------------------*/
{
	*x = poco_lmalloc(size);
	if ((x->pt) == NULL)
		return Err_no_memory;
	*y = poco_lmalloc(size);
	if ((y->pt) == NULL)
		{
		po_free(*x);
		Popot_make_null(x);
		return Err_no_memory;
		}
	return Success;
}

static ErrCode po_tween_get_links(Popot pop_link_count
, Popot pop_starts, Popot pop_ends)
/*****************************************************************************
 * Errcode TweenGetLinks(int *link_count, int **starts, int **ends);
 ****************************************************************************/
{
	int *plink_count;
	Popot *pstarts, *pends;
	Popot starts, ends;
	int *s, *e;
	int link_count;
	Errcode err = Success;
	Tween_link *link, *next;

	/* Make sure the recieving pointers are all there. */
	if ((plink_count = pop_link_count.pt) == NULL
	||	(pstarts = pop_starts.pt) == NULL
	||	(pends = pop_ends.pt) == NULL)
		return builtin_err = Err_null_ref;
	link_count = listlen(&poco_tween_state.links);	
	Popot_make_null(&starts);
	Popot_make_null(&ends);
	if (link_count != 0)
		{
		if ((err = po_lmalloc_2(&starts, &ends, link_count * sizeof(int))) 
		< Success)
			{
			link_count = 0;		/* Just to make error recovery easier. */
			goto CLEANUP;
			}
		s = starts.pt;			/* Grab integer pointers to data areas. */
		e = ends.pt;
								/* Step through list of links copying them
								 * to arrays. */
		for (link = (Tween_link *)(poco_tween_state.links.head);
			NULL  != (next = (Tween_link *)(link->node.next));
			link = next)
			{
			*s++  = link->start;
			*e++ = link->end;
			}
		}
CLEANUP:
	/* Save all the return values. */
	*plink_count = link_count;
	*pstarts = starts;
	*pends = ends;
	return err;
}

static void po_tween_clear_links(void)
/*****************************************************************************
 * void TweenClearLinks(void);
 ****************************************************************************/
{
	free_dl_list(&poco_tween_state.links);
}

static void po_tween_set_splined(Boolean is_splined)
/*****************************************************************************
 * void	TweenSetSplined(Boolean is_splined);
 ****************************************************************************/
{
	vs.tween_spline = is_splined;
}

static Boolean po_tween_get_splined(void)
/*****************************************************************************
 * Boolean	TweenGetSplined(void);
 ****************************************************************************/
{
	return vs.tween_spline;
}

static ErrCode po_tween_trails(int steps)
/*****************************************************************************
 * ErrCode	TweenTrails(int steps);
 ****************************************************************************/
{
	ErrCode err;

	if (!tween_has_data(&poco_tween_state))
		return Err_not_found;
	free_render_cashes();
	err = tween_trail_frame(&poco_tween_state, steps);
	make_render_cashes();
}

static void short_xyz_to_arrays(int point_count, Short_xyz *point_list
, int *x, int *y)
/*----------------------------------------------------------------------------
 * Convert from Short_xzy to 2 arrays format (tossing out the z coordinate).
 *--------------------------------------------------------------------------*/
{
	while (--point_count >= 0)
		{
		*x++ = point_list->x;
		*y++ = point_list->y;
		++point_list;
		}
}

static ErrCode po_short_xyz_to_arrays(int point_count, Short_xyz *point_list
, Popot *ret_x, Popot *ret_y)
/*----------------------------------------------------------------------------
 * Allocate Poco memory for arrays and then convert point_list into
 * 2 arrays format.
 *--------------------------------------------------------------------------*/
{
	Popot x, y;
	Errcode err;

/* First allocate poco memory space for the return arrays. */
	if ((err = po_lmalloc_2(&x, &y, point_count * sizeof(int))) < Success)
		return err;
/* Then do format conversion. */
	short_xyz_to_arrays(point_count, point_list, x.pt, y.pt);
/* And return result. */
	*ret_x = x;
	*ret_y = y;
	return Success;
}

static ErrCode po_tween_make_poly(double time
, Popot pop_ptcount, Popot pop_x, Popot pop_y)
/*****************************************************************************
 * ErrCode TweenMakePoly(double time, int *ptcount, int **x, int **y);
 * Convert a tween-state and a time to a some coordinate lists for Poco.
 ****************************************************************************/
{
	Popot *px, *py;
	int *ppoint_count;
	Errcode err;
	Tw_tlist tlist;
	int scale;
	Short_xyz *points;
	int point_count;

	init_tw_list(&tlist);
	if (time < 0.0)
		time = 0.0;
	if (time > 2.0)		/* We'll allow a little overshoot - it can be
						 * interesting. */
		time = 2.0;
	if ((ppoint_count = pop_ptcount.pt) == NULL
	|| (px = pop_x.pt) == NULL
	|| (py = pop_y.pt) == NULL)
		{
		err = builtin_err = Err_null_ref;
		goto CLEANUP;
		}
	if (!tween_has_data(&poco_tween_state))
		{
		err = Err_not_found;
		goto CLEANUP;
		}
	if ((err = ts_to_tw_list(&poco_tween_state, vs.closed_curve, &tlist)) 
	< Success)
		{
		goto CLEANUP;
		}
	scale = SCALE_ONE * time;
	calc_tween_points(&tlist
	, vs.closed_curve, scale, &points, &point_count);
	if ((err = po_short_xyz_to_arrays(point_count, points, px, py)) 
	< Success)
		{
		goto CLEANUP;
		}
	*ppoint_count = point_count;
CLEANUP:
	trash_tw_list(&tlist);
	return err;
}

static ErrCode po_tween_render(void)
/*****************************************************************************
 * ErrCode	TweenRender(void);
 ****************************************************************************/
{
	Tween1_data twda;
	Autoarg aa;
	Errcode err;

	if (!tween_has_data(&poco_tween_state))
		return Err_not_found;
	twda.is_spline = vs.tween_spline;
	twda.ts = &poco_tween_state;
	clear_struct(&aa);
	aa.avec = tween1;
	aa.avecdat = &twda;
	free_render_cashes();		/* AAARRRR */
	err = noask_do_auto_time_mode(&aa);
	make_render_cashes();
	return err;
}

#ifdef NEVER
ErrCode TweenLoad(char *file_name);
ErrCode TweenSave(char *file_name);
Boolean TweenExists(void);
void	TweenClear(void);
ErrCode TweenSetStart(int ptcount, int *x, int *y);
ErrCode TweenGetStart(int *ptcount, int **x, int **y);
ErrCode TweenSetEnd(int ptcount, int *x, int *y);
ErrCode TweenGetEnd(int *ptcount, int **x, int **y);
void	TweenSwapEnds(void);
ErrCode TweenEndToStart(void);
ErrCode TweenAddLink(int start_point, int end_point);
Errcode TweenSetLinks(int link_count, int *starts, int *ends);
Errcode TweenGetLinks(int *link_count, int **starts, int **ends);
void	TweenClearLinks(void);
void	TweenSetSplined(Boolean is_splined);
Boolean	TweenGetSplined(void);
ErrCode	TweenTrails(int steps);
ErrCode TweenMakePoly(double time, int *ptcount, int **x, int **y);
ErrCode	TweenRender(void);


ErrCode po_tween_load(Popot pop_file_name)
ErrCode po_tween_save(Popot pop_file_name)
Boolean po_tween_exists(void)
void po_tween_clear(void)
ErrCode po_tween_set_start(int ptcount, Popot pop_x, Popot pop_y)
ErrCode po_tween_get_start(Popot pop_ptcount, Popot pop_x, Popot pop_y)
ErrCode po_tween_set_end(int ptcount, Popot pop_x, Popot pop_y)
ErrCode po_tween_get_end(Popot pop_ptcount, Popot pop_x, Popot pop_y)
void po_tween_swap_ends(void)
ErrCode po_tween_end_to_start(void)
ErrCode po_tween_one_link(int start_point, int end_point)
ErrCode po_tween_set_links(int link_count, Popot pop_starts, Popot pop_ends)
ErrCode po_tween_get_links(Popot pop_link_count, Popot pop_starts, Popot pop_ends)
void po_tween_clear_links(void)
void po_tween_set_splined(Boolean is_splined)
Boolean po_tween_get_splined(void)
ErrCode po_tween_trails(void)
ErrCode po_tween_make_poly(double time, Popot pop_ptcount, Popot pop_x, Popot pop_y)
ErrCode po_tween_render(void)
#endif /* NEVER */

/*----------------------------------------------------------------------------
 * library protos...
 *
 * Maintenance notes:
 *	To add a function to this library, add the pointer and prototype string
 *	TO THE END of the following list.  Then go to POCOLIB.H and add a
 *	corresponding prototype to the library structure typedef therein which
 *	matches the name of the structure below.  When creating the prototype
 *	in POCOLIB.H, remember that all arguments prototyped below as pointers
 *	must be defined as Popot types in the prototype in pocolib.h; any number
 *	of stars in the Poco proto still equate to a Popot with no stars in the
 *	pocolib.h prototype.
 *
 *	DO NOT ADD NEW PROTOTYPES OTHER THAN AT THE END OF AN EXISTING STRUCTURE!
 *	DO NOT DELETE A PROTOTYPE EVER! (The best you could do would be to point
 *	the function to a no-op service routine).  Breaking these rules will
 *	require the recompilation of every POE module in the world.  These
 *	rules apply only to library functions which are visible to POE modules
 *	(ie, most of them).  If a specific typedef name exists in pocolib.h, the
 *	rules apply.  If the protos are coded as a generic array of Lib_proto
 *	structs without an explicit typedef in pocolib.h, the rules do not apply.
 *--------------------------------------------------------------------------*/

PolibTween po_libtween = {
po_tween_load,
	"ErrCode TweenLoad(char *file_name);",
po_tween_save,
	"ErrCode TweenSave(char *file_name);",
po_tween_exists,
	"Boolean TweenExists(void);",
po_tween_clear,
	"void	TweenClear(void);",
po_tween_set_start,
	"ErrCode TweenSetStart(int ptcount, int *x, int *y);",
po_tween_get_start,
	"ErrCode TweenGetStart(int *ptcount, int **x, int **y);",
po_tween_set_end,
	"ErrCode TweenSetEnd(int ptcount, int *x, int *y);",
po_tween_get_end,
	"ErrCode TweenGetEnd(int *ptcount, int **x, int **y);",
po_tween_swap_ends,
	"void	TweenSwapEnds(void);",
po_tween_end_to_start,
	"ErrCode TweenEndToStart(void);",
po_tween_one_link,
	"ErrCode TweenOneLink(int start_point, int end_point);",
po_tween_set_links,
	"Errcode TweenSetLinks(int link_count, int *starts, int *ends);",
po_tween_get_links,
	"Errcode TweenGetLinks(int *link_count, int **starts, int **ends);",
po_tween_clear_links,
	"void TweenClearLinks(void);",
po_tween_set_splined,
	"void	TweenSetSplined(Boolean is_splined);",
po_tween_get_splined,
	"Boolean	TweenGetSplined(void);",
po_tween_trails,
	"ErrCode	TweenTrails(int steps);",
po_tween_make_poly,
	"ErrCode TweenMakePoly(double time, int *ptcount, int **x, int **y);",
po_tween_render,
	"ErrCode	TweenRender(void);",
};

Poco_lib po_tween_lib = {
	NULL, "Tween",
	(Lib_proto *)&po_libtween,POLIB_TWEEN_SIZE,
	};
