/* pocomode.c - poco library functions that get/set drawing state
   and other variables. */

#include "errcodes.h"
#include "jimk.h"
#include "pocoface.h"
#include "pocolib.h"

extern Errcode builtin_err;

static Errcode po_ink_set(Popot name)
/*****************************************************************************
 * ErrCode SetInk(char *name)
 ****************************************************************************/
{
extern Option_tool *ink_list;
Option_tool *l;

if (name.pt == NULL)
	return(builtin_err = Err_null_ref);

l = ink_list;
while (l != NULL)
	{
	if (txtcmp(name.pt, l->name) == 0)
		{
		free_render_cashes();
		set_curink(l);
		set_render_fast();
		make_render_cashes();
		return(Success);
		}
	l = l->next;
	}
return(Err_not_found);
}

static void po_get_ink(Popot name)
/*****************************************************************************
 * void GetInk(char *buf)
 ****************************************************************************/
{
if (Popot_bufcheck(&name, 16) >= Success)
	strcpy(name.pt,vl.ink->ot.name);
}

static void po_ink_strength(int percent)
/*****************************************************************************
 * void SetInkStrength(int percent)
 ****************************************************************************/
{
if (percent > 100)
	percent = 100;
if (percent < 0)
	percent = 0;
free_render_cashes();
vl.ink->strength = percent;
make_render_cashes();
}

static int po_get_ink_strength(void)
/*****************************************************************************
 * int GetInkStrength(void)
 ****************************************************************************/
{
return(vl.ink->strength);
}

static void po_ink_dither(Boolean dither)
/*****************************************************************************
 * void SetInkDither(Boolean dither)
 ****************************************************************************/
{
free_render_cashes();
vl.ink->dither = dither;
make_render_cashes();
}

static Boolean po_get_ink_dither(void)
/*****************************************************************************
 * Boolean GetInkDither(void)
 ****************************************************************************/
{
return(vl.ink->dither);
}

static void po_tool_fill(Boolean fill)
/*****************************************************************************
 * void SetFilled(Boolean fill)
 ****************************************************************************/
{
vs.fillp = fill;
}

static Boolean po_get_fill(void)
/*****************************************************************************
 * Boolean GetFilled(void)
 ****************************************************************************/
{
return(vs.fillp);
}


static void po_tool_brush_size(int size)
/*****************************************************************************
 * void SetBrushSize(int size)
 ****************************************************************************/
{
vs.use_brush = (size != 0);
set_circle_brush(size);
}

static int po_get_brush_size(void)
/*****************************************************************************
 * int GetBrushSize(void)
 ****************************************************************************/
{
	return(vs.use_brush?get_brush_size():0);
}

static void po_set_key_clear(Boolean clear)
/*****************************************************************************
 * void SetKeyMode(Boolean clear)
 ****************************************************************************/
{
vs.zero_clear = (clear != 0);
}

static Boolean po_get_key_clear(void)
/*****************************************************************************
 * Boolean GetKeyMode(void)
 ****************************************************************************/
{
return(vs.zero_clear);
}

static void po_set_key_color(int color)
/*****************************************************************************
 * void SetKeyColor(int color)
 ****************************************************************************/
{
vs.inks[0] = color&0xff;
}

static int po_get_key_color(void)
/*****************************************************************************
 * int GetKeyColor(void)
 ****************************************************************************/
{
return(vs.inks[0]);
}

static void po_set_mask_use(Boolean use_it)
/*****************************************************************************
 * void SetMaskUse(Boolean use_it)
 ****************************************************************************/
{
vs.use_mask = use_it;
if (vs.use_mask)
	vs.make_mask = 0;
set_render_fast();
free_render_cashes();
make_render_cashes();
}

static Boolean po_get_mask_use(void)
/*****************************************************************************
 * Boolean GetMaskUse(void)
 ****************************************************************************/
{
return(vs.use_mask);
}

static void po_set_mask_make(Boolean make_it)
/*****************************************************************************
 * void SetMaskCreate(Boolean make_it)
 ****************************************************************************/
{
vs.make_mask = make_it;
if (vs.make_mask)
	vs.use_mask = 0;
set_render_fast();
free_render_cashes();
make_render_cashes();
}

static Boolean po_get_mask_make(void)
/*****************************************************************************
 * Boolean GetMaskCreate(void)
 ****************************************************************************/
{
return(vs.make_mask);
}

static int po_get_star_points(void)
/*****************************************************************************
 * int GetStarPoints(void)
 ****************************************************************************/
{
return(vs.star_points);
}

static void po_set_star_points(int points)
/*****************************************************************************
 * void SetStarPoints(int points)
 ****************************************************************************/
{
extern Qslider star_points_sl;
vs.star_points = clip_to_slider(points,&star_points_sl);
}

static int po_get_star_ratio(void)
/*****************************************************************************
 * int GetStarRatio(void)
 ****************************************************************************/
{
return(vs.star_ratio);
}

static void po_set_star_ratio(int ratio)
/*****************************************************************************
 * void SetStarRatio(int ratio)
 ****************************************************************************/
{
extern Qslider star_ratio_sl;
vs.star_ratio = clip_to_slider(ratio, &star_ratio_sl);
}

static void po_get_spline_tcb(Popot t, Popot c, Popot b)
/*****************************************************************************
 * void GetSplineTCB(int *t, int *c, int *b)
 ****************************************************************************/
{
if (t.pt == NULL || c.pt == NULL || b.pt == NULL)
	{
	builtin_err = Err_null_ref;
	}
else
	{
	*(int *)t.pt = vs.sp_tens;
	*(int *)c.pt = vs.sp_cont;
	*(int *)b.pt = vs.sp_bias;
	}
}

static void po_set_spline_tcb(int t, int c, int b)
/*****************************************************************************
 * void SetSplineTCB(int t, int c, int b)
 ****************************************************************************/
{
extern Qslider otens_sl, ocont_sl, obias_sl;

vs.sp_tens = clip_to_slider(t, &otens_sl);
vs.sp_cont = clip_to_slider(c, &ocont_sl);
vs.sp_bias = clip_to_slider(b, &obias_sl);
}

static Boolean po_get_two_color(void)
/*****************************************************************************
 * Boolean GetTwoColorOn(void)
 ****************************************************************************/
{
return(vs.color2);
}

static void po_set_two_color(Boolean setit)
/*****************************************************************************
 * void SetTwoColorOn(Boolean setit)
 ****************************************************************************/
{
vs.color2 = setit;
}

static int po_get_outline_color(void)
/*****************************************************************************
 * int GetTwoColor(void)
 ****************************************************************************/
{
return(vs.inks[7]);
}

static void po_set_outline_color(int color)
/*****************************************************************************
 * void SetTwoColor(int color)
 ****************************************************************************/
{
vs.inks[7] = (color&255);
}

static Boolean po_get_poly_closed(void)
/*****************************************************************************
 * Boolean GetClosed(void)
 ****************************************************************************/
{
return(vs.closed_curve);
}

static void po_set_poly_closed(Boolean closed)
/*****************************************************************************
 * void SetClosed(Boolean closed)
 ****************************************************************************/
{
vs.closed_curve = closed;
}

static Boolean po_get_cycle_draw(void)
/*****************************************************************************
 * Boolean GetCycleDraw(void)
 ****************************************************************************/
{
return(vs.cycle_draw);
}

static void po_set_cycle_draw(Boolean cycle)
/*****************************************************************************
 * void SetCycleDraw(Boolean cycle)
 ****************************************************************************/
{
set_ccycle(cycle);
}

static Boolean po_get_multi(void)
/*****************************************************************************
 * Boolean GetMultiFrame(void)
 ****************************************************************************/
{
return vs.multi;
}

static po_set_multi(Boolean multi)
/*****************************************************************************
 * void SetMultiFrame(Boolean multi)
 ****************************************************************************/
{
vs.multi = multi;
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

PolibMode po_libmode = {
po_ink_set,
	"ErrCode SetInk(char *name);",
po_get_ink,
	"void    GetInk(char *buf);",
po_ink_strength,
	"void    SetInkStrength(int percent);",
po_get_ink_strength,
	"int     GetInkStrength(void);",
po_ink_dither,
	"void    SetInkDither(Boolean dither);",
po_get_ink_dither,
	"Boolean GetInkDither(void);",
po_tool_fill,
	"void    SetFilled(Boolean fill);",
po_get_fill,
	"Boolean GetFilled(void);",
po_tool_brush_size,
	"void    SetBrushSize(int size);",
po_get_brush_size,
	"int     GetBrushSize(void);",
po_set_key_clear,
	"void    SetKeyMode(Boolean clear);",
po_get_key_clear,
	"Boolean GetKeyMode(void);",
po_set_key_color,
	"void    SetKeyColor(int color);",
po_get_key_color,
	"int     GetKeyColor(void);",
po_set_mask_use,
	"void    SetMaskUse(Boolean use_it);",
po_get_mask_use,
	"Boolean GetMaskUse(void);",
po_set_mask_make,
	"void    SetMaskCreate(Boolean make_it);",
po_get_mask_make,
	"Boolean GetMaskCreate(void);",
po_set_star_points,
	"void    SetStarPoints(int points);",
po_get_star_points,
	"int     GetStarPoints(void);",
po_set_star_ratio,
	"void    SetStarRatio(int ratio);",
po_get_star_ratio,
	"int     GetStarRatio(void);",
po_set_spline_tcb,
	"void    SetSplineTCB(int t, int c, int b);",
po_get_spline_tcb,
	"void    GetSplineTCB(int *t, int *c, int *b);",
po_set_two_color,
	"void    SetTwoColorOn(Boolean setit);",
po_get_two_color,
	"Boolean GetTwoColorOn(void);",
po_set_outline_color,
	"void    SetTwoColor(int color);",
po_get_outline_color,
	"int     GetTwoColor(void);",
po_set_poly_closed,
	"void    SetClosed(Boolean closed);",
po_get_poly_closed,
	"Boolean GetClosed(void);",
po_set_cycle_draw,
	"void    SetCycleDraw(Boolean cycle);",
po_get_cycle_draw,
	"Boolean GetCycleDraw(void);",
po_get_multi,
	"Boolean GetMultiFrame(void);",
po_set_multi,
	"void    SetMultiFrame(Boolean multi);",
};

Poco_lib po_mode_lib = {
	NULL, "Graphics Modes",
	(Lib_proto *)&po_libmode,POLIB_MODE_SIZE,
	};
