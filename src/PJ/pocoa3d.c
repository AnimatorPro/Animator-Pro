/* pocoa3d.c interface library between poco and the optics subsystem */

#include "errcodes.h"
#include "jimk.h"
#include "pocoface.h"
#include "pocolib.h"
#include "a3d.h"
#include "auto.h"

extern Errcode builtin_err;
Popot poco_lmalloc(long size);
void po_free(Popot ppt);
Errcode po_poly_to_arrays(Poly *p, Popot *x, Popot *y);
Errcode po_arrays_to_poly(Poly *p, int ptcount, Popot *px, Popot *py);
int twirl1(Celcfit *cfit, int ix, int frames,int scale);
int a3d_get_auto_flags();

typedef struct xyzPoint
	{
	double x,y,z;			/* pixel coordinates */
	} XyzPoint;

typedef struct optPos
/* An optics move (well except for the path) */
	{
	XyzPoint move;			/* offset for straight move */
	XyzPoint spin_center;	/* center point of spin in pixels */
	XyzPoint spin_axis; 	/* a line from 0,0,0 to here defines spin axis*/
	XyzPoint spin_theta;	/* spins about 3 axis.	In degrees */
	XyzPoint size_center;	/* center point of scaling in pixels */
	long xp, xq;			/* xscaling factor.  newx = oldx*xp/xq */
	long yp, yq;			/* yscaling factor.  newy = oldy*yp/yq */
	long bp, bq;			/* both scale.	Applied after x and y scale */
	} OptPos;

typedef struct optState
/* Contains all the motion information for an optics transformation */
	{
	int pos_count;		/* Number of OptPos's */
	Popot pos;			/* Info on all the optics sliders. */
	int path_count; 	/* Points in the optics path (0 if no path) */
	Popot xpath;		/* X coordinates of path */
	Popot ypath;		/* Y coordinates of path */
	UBYTE path_type;			/* one of OPT_PATH defines below */
	UBYTE path_closed;	/* is movement path closed? */
	UBYTE outlined; 	/* is element outlined? */
	UBYTE el_type;			/* one of OPT_EL defines below */
	} OptState;


/****** Support routines for converting between Poco's optics state
		structures and Animator's *********/
static shortpt_to_doublept(Short_xyz *in, XyzPoint *out)
/*****************************************************************************
 * Convert a short xyz coordinate to a double xyz coordinate
 ****************************************************************************/
{
out->x = in->x;
out->y = in->y;
out->z = in->z;
}

static doublept_to_shortpt(XyzPoint *in, Short_xyz *out)
/*****************************************************************************
 * Convert a double xyz coordinate to a short xyz coordinate
 ****************************************************************************/
{
out->x = in->x;
out->y = in->y;
out->z = in->z;
}

static double tic_to_deg(SHORT tic)
/*****************************************************************************
 * Convert from 0-TWOPI integer angle representation to floating point
 * 0-360 degree representation
 ****************************************************************************/
{
double ret;

ret = tic;
ret *= 360;
ret /= TWOPI;
return(ret);
}

SHORT deg_to_tic(double deg)
/*****************************************************************************
 *	Convert from 0-360 floating point angle representation to  0-TWOPI
 *	integer representation
 ****************************************************************************/
{
return((deg*TWOPI+180.0)/360.0);
}

static void ado_to_opt(struct ado_setting *in, OptPos *out)
/*****************************************************************************
 * convert a single transformation stack element from Animator to Poco
 * form
 ****************************************************************************/
{
shortpt_to_doublept(&in->move, &out->move);
shortpt_to_doublept(&in->spin_center, &out->spin_center);
shortpt_to_doublept(&in->spin_axis, &out->spin_axis);
shortpt_to_doublept(&in->size_center, &out->size_center);
out->spin_theta.x = tic_to_deg(in->spin_theta.x);
out->spin_theta.y = tic_to_deg(in->spin_theta.y);
out->spin_theta.z = tic_to_deg(in->spin_theta.z);
out->xp = in->xp;
out->xq = in->xq;
out->yp = in->yp;
out->yq = in->yq;
out->bp = in->bp;
out->bq = in->bq;
}

static void opt_to_ado(OptPos *in, struct ado_setting *out)
/*****************************************************************************
 * convert a single transformation stack element from Poco to Animator
 * form
 ****************************************************************************/
{
doublept_to_shortpt(&in->move, &out->move);
doublept_to_shortpt(&in->spin_center, &out->spin_center);
doublept_to_shortpt(&in->spin_axis, &out->spin_axis);
doublept_to_shortpt(&in->size_center, &out->size_center);
out->spin_theta.x = deg_to_tic(in->spin_theta.x);
out->spin_theta.y = deg_to_tic(in->spin_theta.y);
out->spin_theta.z = deg_to_tic(in->spin_theta.z);
out->xp = in->xp;
out->xq = in->xq;
out->yp = in->yp;
out->yq = in->yq;
out->bp = in->bp;
out->bq = in->bq;
}

static Errcode opt_setp(int ptcount, Popot *px, Popot *py)
/*****************************************************************************
 * save temp path file from Pox x/y arrays
 ****************************************************************************/
{
Poly p;
Errcode err;

clear_struct(&p);
if ((err  = po_arrays_to_poly(&p, ptcount, px, py)) < Success)
	goto error;
save_poly(ppoly_name,  &p);
error:
	{
	pj_gentle_free(p.clipped_list);
	return(softerr(err, "opt_path"));
	}
}


static int opt_getp(Popot *px, Popot *py)
/*****************************************************************************
 * Stuff path polygon into poco int arrays px,py.  Return path point
 * count.
 ****************************************************************************/
{
Poly p;
Errcode err;

if (!pj_exists(ppoly_name))
	{
	px->pt = px->min = px->max = NULL;
	py->pt = py->min = py->max = NULL;
	return(0);			/* just no path... */
	}
if ((err = load_a_poly(ppoly_name, &p)) < 0)
	return(err);
if ((err = po_poly_to_arrays(&p, px, py)) >= Success)
	err = p.pt_count;
free_polypoints(&p);
return(err);
}


/*** start of actual library routines **********/
static void opt_clear_state(void)
/*****************************************************************************
 * void OptClearState(void);
 ****************************************************************************/
{
get_a3d_state();
ado_clear_all();
set_a3d_state();
}

static Errcode opt_ss(OptState	*state)
/*****************************************************************************
 * convert OptState to vs.move3, ppoly_name temp file, and various
 * vs flags
 ****************************************************************************/
{
OptPos *pos;
Errcode err;
UBYTE ptype;
int i;
int count;
struct ado_setting *as;

ado_clear_all();
if (state->path_count > 0)
	{
	if ((err = opt_setp(state->path_count, &state->xpath, &state->ypath))
		< Success)
		return(err);
	}
ptype = state->path_type;
if (ptype > PATH_CLOCKED)
	return(builtin_err	= Err_parameter_range);
vs.ado_path = ptype;
vs.pa_closed = state->path_closed;
vs.ado_outline = state->outlined;
if ((err =	opt_set_element(state->el_type)) < Success)
	return(err);
if ((count = state->pos_count) > 0)
	{
	if ((err = Popot_bufcheck(&state->pos,count*sizeof(OptPos)))<Success)
		return(err);
	pos = state->pos.pt;
	as = NULL;
	for (i = count;  --i >= 0; )
		{
		opt_to_ado(&pos[i], &vs.move3);
		make_rot_op();
		if (i != 0)
			{
			if ((err = do_move_along()) < Success)
				return(err);
			}
		}
	}
return(err);
}

static Errcode opt_set_state(Popot sta)
/*****************************************************************************
 * void OptSetState(struct optState *os);
 ****************************************************************************/
{
Errcode err;

if ((err = Popot_bufcheck(&sta,sizeof(OptState)))<Success)
	return(err);
get_a3d_state();
err = opt_ss(sta.pt);
set_a3d_state();
return(err);
}

static Errcode opt_get_state(Popot sta)
/*****************************************************************************
 * void OptGetState(struct optState *os);
 ****************************************************************************/
{
struct ado_setting *as;
int pos_count;
Errcode err;
OptState *state;
OptPos *pos;

if ((err = Popot_bufcheck(&sta,sizeof(OptState)))<Success)
	return(err);
get_a3d_state();
state = sta.pt;
clear_struct(state);
if ((err = state->path_count = opt_getp(&state->xpath, &state->ypath))
	< Success)
	return(err);
state->path_type = vs.ado_path;
state->path_closed = vs.pa_closed;
state->outlined = vs.ado_outline;
state->el_type = vs.ado_source;

as = &vs.move3;
state->pos_count = pos_count = slist_len(as);
state->pos	= poco_lmalloc(pos_count*sizeof(OptPos));
if ((pos = state->pos.pt) == NULL)
	{
	poco_freez(&state->xpath);
	poco_freez(&state->ypath);
	return(Err_no_memory);
	}
while (as != NULL)
	{
	ado_to_opt(as, pos);
	as = as->next;
	pos += 1;
	}
set_a3d_state();
}

static Errcode opt_free_state(Popot sta)
/*****************************************************************************
 * void OptFreeState(struct optState *os);
 ****************************************************************************/
{
OptState *state;
Errcode err;

if ((err = Popot_bufcheck(&sta,sizeof(OptState)))<Success)
	return(err);
state = sta.pt;
poco_freez(&state->xpath);
poco_freez(&state->ypath);
poco_freez(&state->pos);
clear_struct(state);
return(Success);
}

static void opt_clear_pos(void)
/*****************************************************************************
 * void OptClearPos(void);
 ****************************************************************************/
{
get_a3d_state();
ado_clear_pos();
set_a3d_state();
}

static void opt_set_pos(Popot pod)
/*****************************************************************************
 * void OptSetPos(struct optPos *op);
 ****************************************************************************/
{
if (Popot_bufcheck(&pod,sizeof(OptPos))<Success)
	return;
get_a3d_state();
opt_to_ado(pod.pt, &vs.move3);
make_rot_op();
set_a3d_state();
}

static void opt_get_pos(Popot pod)
/*****************************************************************************
 * void OptGetPos(struct optPos *op);
 ****************************************************************************/
{
if (Popot_bufcheck(&pod,sizeof(OptPos))<Success)
	return;
get_a3d_state();
ado_to_opt(&vs.move3, pod.pt);
set_a3d_state();
}

static void opt_clear_path(void)
/*****************************************************************************
 * void OptClearPath(void);
 ****************************************************************************/
{
pj_delete(ppoly_name);
}

static Errcode opt_set_path(int ptcount, Popot px, Popot py)
/*****************************************************************************
 * void OptSetPath(int ptcount, int *x, int *y);
 ****************************************************************************/
{
return(opt_setp(ptcount, &px, &py));
}

static int opt_get_path(Popot pxlist, Popot pylist)
/*****************************************************************************
 * int OptGetPath(int **x, int **y);
 ****************************************************************************/
{
Popot *px, *py;

if ((px = pxlist.pt) == NULL || (py = pylist.pt) == NULL)
	return(builtin_err = Err_null_ref);
return(opt_getp(px,py));
}

void opt_free_path(Popot pxlist, Popot pylist)
/*****************************************************************************
 * void OptFreePath(int **x, int **y);
 ****************************************************************************/
{
Popot *px, *py;

if ((px = pxlist.pt) == NULL || (py = pylist.pt) == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
poco_freez(px);
poco_freez(py);
}

static void opt_continue(void)
/*****************************************************************************
 * void OptContinue(void);
 ****************************************************************************/
{
get_a3d_state();
do_move_along();
set_a3d_state();
}

static void opt_default_centers(void)
/*****************************************************************************
 * void OptDefaultCenters(void);
 ****************************************************************************/
{
get_a3d_state();
a3d_default_centers();
set_a3d_state();
}

static int opt_get_element(void)
/*****************************************************************************
 * int OptGetElement(void);
 ****************************************************************************/
{
Boolean no_poly, no_tween;

a3d_check_el(&no_poly, &no_tween);
return(vs.ado_source);
}


static Errcode opt_set_element(unsigned el)
/*****************************************************************************
 * ErrCode OptSetElement(int el);
 ****************************************************************************/
{
if (el > OPS_TWEEN)
	return(builtin_err	= Err_parameter_range);
vs.ado_source = el;
if (opt_get_element() != el)
	return(Err_not_found);
return(Success);
}

static Errcode opt_to_frame(double time)
/*****************************************************************************
 * ErrCode OptToFrame(double time);
 ****************************************************************************/
{
Celcfit cfit;


get_a3d_state();
init_celcfit(&cfit);
make_rot_op();
twirl1(&cfit, (int)(time*100), 100, (int)(SCALE_ONE*time));
dirties();
set_a3d_state();
return(Success);
}

static Errcode opt_to_some(int fmode)
/*****************************************************************************
 * ?
 ****************************************************************************/
{
Autoarg aa;
Celcfit cfit;
Errcode err;

get_a3d_state();
init_celcfit(&cfit);
clear_struct(&aa);
aa.avec = twirl1;
aa.avecdat = &cfit;
aa.flags = a3d_get_auto_flags();
make_rot_op();
err  = noask_do_auto(&aa, fmode);
set_a3d_state();
return(err);
}

static Errcode opt_to_segment(int start, int stop)
/*****************************************************************************
 * ErrCode OptToSegment(int start, int stop);
 ****************************************************************************/
{
SHORT ostart,  ostop;
Errcode err;

ostart = vs.start_seg;
ostop = vs.stop_seg;
vs.start_seg = start;
vs.stop_seg = stop;
err = opt_to_some(DOAUTO_SEGMENT);
vs.stop_seg = ostop;
vs.start_seg = ostart;
return(err);
}

static Errcode opt_to_all(void)
/*****************************************************************************
 * ErrCode OptToAll(void);
 ****************************************************************************/
{
return(opt_to_some(DOAUTO_ALL));
}

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

PolibOptics po_liboptics = {
opt_clear_state,
	"void    OptClearState(void);",
opt_set_state,
	"void    OptSetState(struct optState *os);",
opt_get_state,
	"void    OptGetState(struct optState *os);",
opt_free_state,
	"void    OptFreeState(struct optState *os);",
opt_clear_pos,
	"void    OptClearPos(void);",
opt_set_pos,
	"void    OptSetPos(struct optPos *op);",
opt_get_pos,
	"void    OptGetPos(struct optPos *op);",
opt_clear_path,
	"void    OptClearPath(void);",
opt_set_path,
	"void    OptSetPath(int ptcount, int *x, int *y);",
opt_get_path,
	"int     OptGetPath(int **x, int **y);",
opt_free_path,
	"void    OptFreePath(int **x, int **y);",
opt_default_centers,
	"void    OptDefaultCenters(void);",
opt_continue,
	"void    OptContinue(void);",
opt_get_element,
	"int     OptGetElement(void);",
opt_set_element,
	"ErrCode OptSetElement(int el);",
opt_to_frame,
	"ErrCode OptToFrame(double time);",
opt_to_segment,
	"ErrCode OptToSegment(int start, int stop);",
opt_to_all,
	"ErrCode OptToAll(void);",
};

Poco_lib po_optics_lib =
	{
	NULL, "Optics",
	(Lib_proto *)&po_liboptics, POLIB_OPTICS_SIZE,
	};


